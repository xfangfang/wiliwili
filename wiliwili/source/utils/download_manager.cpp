//
// Created by copilot on 2026/4/6.
//

#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <fmt/format.h>
#include <cpr/cpr.h>
#include <cpr/filesystem.h>
#include <borealis/core/thread.hpp>
#include <borealis/core/application.hpp>
#include <borealis/core/logger.hpp>

#include "utils/download_manager.hpp"
#include "utils/config_helper.hpp"
#include "api/bilibili/util/http.hpp"
#include "api/bilibili/util/uuid.hpp"
#include "bilibili.h"

using namespace brls::literals;

// ---------- helpers ----------

static std::string generateId(const std::string& seed) {
    // deterministic from bvid+cid+timestamp
    return bilibili::genUUID(seed + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
}

static std::string sanitizeFilename(std::string name) {
    for (auto& c : name) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' ||
            c == '|') {
            c = '_';
        }
    }
    return name;
}

#ifdef _WIN32
static const char PATH_SEP = '\\';
#else
static const char PATH_SEP = '/';
#endif

static std::string joinPath(const std::string& dir, const std::string& file) {
    return dir + PATH_SEP + file;
}

// ---------- DownloadManager ----------

DownloadManager::DownloadManager() = default;
DownloadManager::~DownloadManager() {
    // Cancel all active downloads on destruction
    std::lock_guard<std::mutex> lock(tasksMutex);
    for (auto& t : tasks) {
        if (t.cancelFlag) t.cancelFlag->store(true);
    }
}

void DownloadManager::loadState() {
    const std::string path = joinPath(ProgramConfig::instance().getConfigDir(), "download_state.json");
    if (!cpr::fs::exists(path)) return;

    std::ifstream f(path);
    if (!f.is_open()) return;

    try {
        nlohmann::json j;
        f >> j;
        std::lock_guard<std::mutex> lock(tasksMutex);
        tasks = j.get<std::vector<DownloadTask>>();
        // Mark any "DOWNLOADING" tasks as PAUSED since the app was interrupted
        for (auto& t : tasks) {
            if (t.status == DownloadTaskStatus::DOWNLOADING) {
                t.status = DownloadTaskStatus::PAUSED;
            }
        }
        brls::Logger::info("DownloadManager: loaded {} tasks from state", tasks.size());
    } catch (const std::exception& e) {
        brls::Logger::error("DownloadManager: failed to load state: {}", e.what());
    }
}

void DownloadManager::saveState() {
    const std::string configDir = ProgramConfig::instance().getConfigDir();
    cpr::fs::create_directories(configDir);
    const std::string path = joinPath(configDir, "download_state.json");

    try {
        std::lock_guard<std::mutex> lock(tasksMutex);
        nlohmann::json j = tasks;
        std::ofstream f(path);
        if (!f.is_open()) {
            brls::Logger::error("DownloadManager: cannot write state to: {}", path);
            return;
        }
        f << j.dump(2);
        brls::Logger::debug("DownloadManager: state saved ({} tasks)", tasks.size());
    } catch (const std::exception& e) {
        brls::Logger::error("DownloadManager: failed to save state: {}", e.what());
    }
}

void DownloadManager::addTask(DownloadTask task) {
    if (task.id.empty()) {
        task.id = generateId(task.bvid + std::to_string(task.cid));
    }
    if (task.dir.empty()) {
        std::string base = ProgramConfig::instance().getDownloadDir();
        std::string dirName = task.bvid + "_" + std::to_string(task.cid);
        std::string candidate = joinPath(base, dirName);
        // If directory already exists, append a timestamp to avoid collision
        if (cpr::fs::exists(candidate)) {
            auto ts = std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
            candidate = joinPath(base, dirName + "_" + std::to_string(ts));
        }
        task.dir = candidate;
    }
    if (task.video_file.empty()) {
        task.video_file = task.is_dash ? "video.m4s" : "video.flv";
    }
    if (task.audio_file.empty() && task.is_dash) {
        task.audio_file = "audio.m4s";
    }
    task.status = DownloadTaskStatus::PENDING;
    task.cancelFlag = std::make_shared<std::atomic<bool>>(false);
    task.pauseFlag  = std::make_shared<std::atomic<bool>>(false);

    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        tasks.push_back(std::move(task));
    }
    saveState();
    startNextTask();
}

void DownloadManager::pauseTask(const std::string& id) {
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        for (auto& t : tasks) {
            if (t.id == id && t.status == DownloadTaskStatus::DOWNLOADING) {
                t.pauseFlag->store(true);
                t.status = DownloadTaskStatus::PAUSED;
                brls::sync([this, id]() {
                    taskStatusChangedEvent.fire(id);
                });
                break;
            }
        }
    }
    saveState();
}

void DownloadManager::resumeTask(const std::string& id) {
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        for (auto& t : tasks) {
            if (t.id == id && t.status == DownloadTaskStatus::PAUSED) {
                t.status     = DownloadTaskStatus::PENDING;
                t.cancelFlag = std::make_shared<std::atomic<bool>>(false);
                t.pauseFlag  = std::make_shared<std::atomic<bool>>(false);
                break;
            }
        }
    }
    saveState();
    startNextTask();
}

void DownloadManager::cancelTask(const std::string& id) {
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        for (auto& t : tasks) {
            if (t.id == id) {
                if (t.cancelFlag) t.cancelFlag->store(true);
                t.status = DownloadTaskStatus::CANCELLED;
                brls::sync([this, id]() {
                    taskStatusChangedEvent.fire(id);
                });
                break;
            }
        }
    }
    saveState();
}

void DownloadManager::deleteTask(const std::string& id) {
    std::string dir;
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        for (auto& t : tasks) {
            if (t.id == id) {
                if (t.cancelFlag) t.cancelFlag->store(true);
                dir = t.dir;
                break;
            }
        }
        tasks.erase(std::remove_if(tasks.begin(), tasks.end(),
                                   [&id](const DownloadTask& t) { return t.id == id; }),
                    tasks.end());
    }
    // Remove directory and its contents
    if (!dir.empty() && cpr::fs::exists(dir)) {
        try {
            cpr::fs::remove_all(dir);
        } catch (const std::exception& e) {
            brls::Logger::error("DownloadManager: failed to remove dir {}: {}", dir, e.what());
        }
    }
    saveState();
    brls::sync([this, id]() {
        taskStatusChangedEvent.fire(id);
    });
}

DownloadTask* DownloadManager::getTask(const std::string& id) {
    std::lock_guard<std::mutex> lock(tasksMutex);
    for (auto& t : tasks) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

const std::vector<DownloadTask>& DownloadManager::getTasks() const { return tasks; }
std::vector<DownloadTask>& DownloadManager::getTasks() { return tasks; }

std::vector<DownloadTask> DownloadManager::getTasksSnapshot() const {
    std::lock_guard<std::mutex> lock(tasksMutex);
    return tasks;
}

bool DownloadManager::hasIncompleteDownloads() const {
    std::lock_guard<std::mutex> lock(tasksMutex);
    for (const auto& t : tasks) {
        if (t.status == DownloadTaskStatus::PENDING || t.status == DownloadTaskStatus::DOWNLOADING ||
            t.status == DownloadTaskStatus::PAUSED) {
            return true;
        }
    }
    return false;
}

bool DownloadManager::hasCompletedTask(const std::string& bvid, uint64_t cid) const {
    std::lock_guard<std::mutex> lock(tasksMutex);
    for (const auto& t : tasks) {
        if (t.bvid == bvid && t.cid == cid && t.status == DownloadTaskStatus::COMPLETED) {
            return true;
        }
    }
    return false;
}

void DownloadManager::startNextTask() {
    std::string nextId;
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        int active = 0;
        for (const auto& t : tasks) {
            if (t.status == DownloadTaskStatus::DOWNLOADING) active++;
        }
        if (active >= MAX_CONCURRENT) return;
        for (auto& t : tasks) {
            if (t.status == DownloadTaskStatus::PENDING) {
                t.status = DownloadTaskStatus::DOWNLOADING;
                nextId   = t.id;
                break;
            }
        }
    }
    if (!nextId.empty()) {
        saveState();
        cpr::async([this, nextId]() { this->runTask(nextId); });
    }
}

// ---------- Core download logic ----------

bool DownloadManager::downloadFile(const std::string& url,
                                   const std::string& filepath,
                                   std::atomic<bool>& cancelFlag,
                                   std::atomic<bool>& pauseFlag,
                                   int64_t& downloadedBytes,
                                   int64_t& totalBytes,
                                   const std::string& taskId) {
    // Support resume: if file already partially exists, start from where we left off
    int64_t existingBytes = 0;
    if (cpr::fs::exists(filepath)) {
        try {
            existingBytes = static_cast<int64_t>(cpr::fs::file_size(filepath));
        } catch (...) {}
    }

    // Open file in append mode if resuming, otherwise truncate
    std::ofstream ofs;
    if (existingBytes > 0) {
        ofs.open(filepath, std::ios::binary | std::ios::app);
        downloadedBytes = existingBytes;
    } else {
        ofs.open(filepath, std::ios::binary | std::ios::trunc);
        downloadedBytes = 0;
    }
    if (!ofs.is_open()) {
        brls::Logger::error("DownloadManager: cannot open file for writing: {}", filepath);
        return false;
    }

    // Read speed limit from config (MB/s; 0 = unlimited)
    int64_t limitBytesPerSec = 0;
    {
        std::string limitStr = ProgramConfig::instance().getSettingItem(SettingItem::DOWNLOAD_SPEED_LIMIT, std::string{"1"});
        try {
            double mb = std::stod(limitStr);
            if (mb > 0.0) limitBytesPerSec = static_cast<int64_t>(mb * 1024.0 * 1024.0);
        } catch (...) {}
    }

    // Speed-limit state (token bucket, 1-second window)
    auto windowStart  = std::chrono::steady_clock::now();
    int64_t windowBytes = 0;

    // Throttle progress-event firing (at most once per 250 ms)
    auto lastProgressFire = std::chrono::steady_clock::now();

    auto session = bilibili::HTTP::createSession();
    session->SetUrl(cpr::Url{url});
    session->SetHeader(bilibili::HTTP::HEADERS);
    if (existingBytes > 0) {
        session->SetHeader(cpr::Header{{"Range", fmt::format("bytes={}-", existingBytes)}});
    }
    session->SetTimeout(cpr::Timeout{0});  // no timeout for downloads

    bool succeeded = true;

    session->SetWriteCallback(cpr::WriteCallback([&](const std::string_view& data, intptr_t) -> bool {
        if (cancelFlag.load() || pauseFlag.load()) return false;
        ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
        downloadedBytes += static_cast<int64_t>(data.size());

        // Speed limiting
        if (limitBytesPerSec > 0) {
            windowBytes += static_cast<int64_t>(data.size());
            auto now     = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now - windowStart).count();
            if (elapsed < 1.0 && windowBytes >= limitBytesPerSec) {
                // Sleep for the remainder of the current 1-second window
                int64_t waitMs = static_cast<int64_t>((1.0 - elapsed) * 1000.0);
                if (waitMs > 0) std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
                windowStart  = std::chrono::steady_clock::now();
                windowBytes  = 0;
            } else if (elapsed >= 1.0) {
                windowStart  = std::chrono::steady_clock::now();
                windowBytes  = 0;
            }
        }

        // Throttle progress event to at most 4 per second
        auto now2 = std::chrono::steady_clock::now();
        if (std::chrono::duration<double>(now2 - lastProgressFire).count() >= 0.25) {
            lastProgressFire = now2;
            brls::sync([this, taskId]() {
                taskProgressEvent.fire(taskId);
            });
        }
        return true;
    }));

    session->SetProgressCallback(cpr::ProgressCallback([&cancelFlag, &pauseFlag, &downloadedBytes, &totalBytes](cpr::cpr_off_t dltotal, cpr::cpr_off_t dlnow, ...) -> bool {
        if (dltotal > 0) totalBytes = dltotal;
        return !cancelFlag.load() && !pauseFlag.load();
    }));

    auto resp = session->Get();
    ofs.close();

    if (cancelFlag.load() || pauseFlag.load()) {
        return false;
    }

    if (resp.error || (resp.status_code != 200 && resp.status_code != 206)) {
        brls::Logger::error("DownloadManager: download failed for {} status={} err={}", url, resp.status_code, resp.error.message);
        succeeded = false;
    }
    return succeeded;
}

bool DownloadManager::downloadDash(DownloadTask& task) {
    cpr::fs::create_directories(task.dir);

    // Download video
    std::string videoPath = joinPath(task.dir, task.video_file);
    bool videoOk = false;
    for (const auto& vurl : task.video_urls) {
        if (task.cancelFlag->load()) return false;
        if (task.pauseFlag->load()) return false;
        if (downloadFile(vurl, videoPath, *task.cancelFlag, *task.pauseFlag,
                         task.downloaded_bytes, task.total_bytes, task.id)) {
            videoOk = true;
            break;
        }
    }
    if (!videoOk) return false;

    // Download audio
    if (!task.audio_urls.empty()) {
        std::string audioPath = joinPath(task.dir, task.audio_file);
        bool audioOk = false;
        for (const auto& aurl : task.audio_urls) {
            if (task.cancelFlag->load()) return false;
            if (task.pauseFlag->load()) return false;
            if (downloadFile(aurl, audioPath, *task.cancelFlag, *task.pauseFlag,
                             task.audio_downloaded_bytes, task.audio_total_bytes, task.id)) {
                audioOk = true;
                break;
            }
        }
        if (!audioOk) return false;
    }
    return true;
}

bool DownloadManager::downloadFlv(DownloadTask& task) {
    cpr::fs::create_directories(task.dir);

    if (task.flv_segments.size() <= 1) {
        // Single FLV file
        std::string videoPath = joinPath(task.dir, task.video_file);
        std::string url = task.flv_segments.empty() ? (task.video_urls.empty() ? "" : task.video_urls[0])
                                                     : task.flv_segments[0].url;
        if (url.empty()) return false;
        return downloadFile(url, videoPath, *task.cancelFlag, *task.pauseFlag,
                            task.downloaded_bytes, task.total_bytes, task.id);
    }

    // Multi-segment FLV: download each segment, then concatenate
    std::vector<std::string> segFiles;
    for (size_t i = 0; i < task.flv_segments.size(); ++i) {
        if (task.cancelFlag->load()) return false;
        if (task.pauseFlag->load()) return false;

        const auto& seg = task.flv_segments[i];
        std::string segFile = joinPath(task.dir, fmt::format("seg_{}.flv", i));
        segFiles.push_back(segFile);

        int64_t segDl = 0, segTotal = 0;
        if (!downloadFile(seg.url, segFile, *task.cancelFlag, *task.pauseFlag, segDl, segTotal, task.id)) {
            return false;
        }
        task.downloaded_bytes += segDl;
        brls::sync([this, id = task.id]() { taskProgressEvent.fire(id); });
    }

    // Concatenate segments into final file
    std::string finalPath = joinPath(task.dir, task.video_file);
    {
        std::ofstream out(finalPath, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) return false;
        for (const auto& sf : segFiles) {
            std::ifstream in(sf, std::ios::binary);
            if (!in.is_open()) return false;
            out << in.rdbuf();
        }
    }
    // Remove segment temp files
    for (const auto& sf : segFiles) {
        try { cpr::fs::remove(sf); } catch (...) {}
    }
    return true;
}

void DownloadManager::saveInfoJson(const DownloadTask& task) {
    nlohmann::json info;
    info["id"]          = task.id;
    info["bvid"]        = task.bvid;
    info["cid"]         = task.cid;
    info["title"]       = task.title;
    info["cover_url"]   = task.cover_url;
    info["quality"]     = task.quality;
    info["quality_desc"] = task.quality_desc;
    info["is_dash"]     = task.is_dash;
    info["video_file"]  = task.video_file;
    info["audio_file"]  = task.audio_file;

    std::string path = joinPath(task.dir, "info.json");
    std::ofstream f(path);
    if (f.is_open()) {
        f << info.dump(2);
    }
}

void DownloadManager::saveDanmaku(const DownloadTask& task) {
    if (task.cid == 0) return;
    std::string taskId = task.id;
    std::string dir    = task.dir;
    BILI::get_danmaku(
        task.cid,
        [taskId, dir](const std::string& xml) {
            std::string path = dir + PATH_SEP + "danmaku.xml";
            std::ofstream f(path);
            if (f.is_open()) {
                f << xml;
                brls::Logger::debug("DownloadManager: saved danmaku.xml for {}", taskId);
            }
        },
        [taskId](BILI_ERR) {
            brls::Logger::error("DownloadManager: failed to download danmaku for {}: {}", taskId, error);
        });
}

void DownloadManager::runTask(const std::string& id) {
    // Take a working copy of the task so we don't hold the mutex during the
    // long-running network download.
    DownloadTask workCopy;
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        bool found = false;
        for (const auto& t : tasks) {
            if (t.id == id) { workCopy = t; found = true; break; }
        }
        if (!found) return;
        // Ensure the live task shares the same cancel/pause flags as our copy
        for (auto& t : tasks) {
            if (t.id == id) {
                t.cancelFlag = workCopy.cancelFlag;
                t.pauseFlag  = workCopy.pauseFlag;
                break;
            }
        }
    }

    // Create the task directory first
    cpr::fs::create_directories(workCopy.dir);

    // Download cover image first (best-effort; never block the video download)
    if (!workCopy.cover_url.empty()) {
        std::string coverPath = joinPath(workCopy.dir, "cover.jpg");
        if (!cpr::fs::exists(coverPath)) {
            try {
                auto coverSession = bilibili::HTTP::createSession();
                coverSession->SetUrl(cpr::Url{workCopy.cover_url});
                coverSession->SetHeader(bilibili::HTTP::HEADERS);
                coverSession->SetTimeout(cpr::Timeout{10000});
                auto coverResp = coverSession->Get();
                if (!coverResp.error && (coverResp.status_code == 200 || coverResp.status_code == 206)) {
                    std::ofstream cf(coverPath, std::ios::binary | std::ios::trunc);
                    if (cf.is_open()) cf.write(coverResp.text.data(), static_cast<std::streamsize>(coverResp.text.size()));
                }
            } catch (...) {}
        }
    }

    bool success = false;
    if (workCopy.is_dash) {
        success = downloadDash(workCopy);
    } else {
        success = downloadFlv(workCopy);
    }

    // Write progress back
    bool taskCompleted = false;
    std::string taskTitle;
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        for (auto& t : tasks) {
            if (t.id == id) {
                t.downloaded_bytes       = workCopy.downloaded_bytes;
                t.total_bytes            = workCopy.total_bytes;
                t.audio_downloaded_bytes = workCopy.audio_downloaded_bytes;
                t.audio_total_bytes      = workCopy.audio_total_bytes;

                if (workCopy.cancelFlag->load()) {
                    t.status = DownloadTaskStatus::CANCELLED;
                } else if (workCopy.pauseFlag->load()) {
                    t.status = DownloadTaskStatus::PAUSED;
                } else if (success) {
                    t.status = DownloadTaskStatus::COMPLETED;
                    taskCompleted = true;
                    taskTitle = t.title;
                } else {
                    t.status = DownloadTaskStatus::FAILED;
                }
                break;
            }
        }
    }

    // Save info and danmaku outside the lock
    if (taskCompleted) {
        saveInfoJson(workCopy);
        saveDanmaku(workCopy);
    }

    saveState();

    brls::sync([this, id, success, taskTitle]() {
        taskStatusChangedEvent.fire(id);
        if (success && !taskTitle.empty()) {
            brls::Application::notify("wiliwili/player/download/completed"_i18n + ": " + taskTitle);
        } else if (success) {
            brls::Application::notify("wiliwili/player/download/completed"_i18n);
        }
    });

    // Kick off next task (if any pending)
    startNextTask();
}
