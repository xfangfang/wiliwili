//
// Created by copilot on 2026/4/6.
//

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <nlohmann/json.hpp>
#include <borealis/core/singleton.hpp>
#include <borealis/core/event.hpp>

// A single FLV segment (for multi-part FLV videos)
struct FlvSegment {
    std::string url;
    uint64_t size = 0;   // content-length hint (0 if unknown)
    int order    = 0;
};
inline void to_json(nlohmann::json& j, const FlvSegment& s) {
    j = nlohmann::json{{"url", s.url}, {"size", s.size}, {"order", s.order}};
}
inline void from_json(const nlohmann::json& j, FlvSegment& s) {
    if (j.contains("url")) j.at("url").get_to(s.url);
    if (j.contains("size")) j.at("size").get_to(s.size);
    if (j.contains("order")) j.at("order").get_to(s.order);
}

enum class DownloadTaskStatus {
    PENDING,
    DOWNLOADING,
    PAUSED,
    COMPLETED,
    FAILED,
    CANCELLED,
};

struct DownloadTask {
    std::string id;           // UUID
    std::string bvid;
    uint64_t cid = 0;
    std::string title;
    std::string cover_url;
    int quality        = 0;
    std::string quality_desc;
    bool is_dash       = false;  // true = DASH, false = FLV

    // DASH: all URLs are tried in order (first = preferred, rest = backup)
    std::vector<std::string> video_urls;
    std::vector<std::string> audio_urls;  // DASH audio URLs

    // FLV: segments (may be only 1)
    std::vector<FlvSegment> flv_segments;

    DownloadTaskStatus status = DownloadTaskStatus::PENDING;

    // Progress (video part)
    int64_t downloaded_bytes = 0;
    int64_t total_bytes      = 0;

    // Progress (audio part, only for DASH)
    int64_t audio_downloaded_bytes = 0;
    int64_t audio_total_bytes      = 0;

    // Directory and relative file paths
    std::string dir;          // absolute path to task directory
    std::string video_file;   // relative name inside dir
    std::string audio_file;   // relative name inside dir (DASH only)

    // Internal: cancellation / pause flag (not persisted)
    std::shared_ptr<std::atomic<bool>> cancelFlag   = std::make_shared<std::atomic<bool>>(false);
    std::shared_ptr<std::atomic<bool>> pauseFlag    = std::make_shared<std::atomic<bool>>(false);

    DownloadTask() = default;
};

inline void to_json(nlohmann::json& j, const DownloadTask& t) {
    j = nlohmann::json{
        {"id", t.id},
        {"bvid", t.bvid},
        {"cid", t.cid},
        {"title", t.title},
        {"cover_url", t.cover_url},
        {"quality", t.quality},
        {"quality_desc", t.quality_desc},
        {"is_dash", t.is_dash},
        {"video_urls", t.video_urls},
        {"audio_urls", t.audio_urls},
        {"flv_segments", t.flv_segments},
        {"status", static_cast<int>(t.status)},
        {"downloaded_bytes", t.downloaded_bytes},
        {"total_bytes", t.total_bytes},
        {"audio_downloaded_bytes", t.audio_downloaded_bytes},
        {"audio_total_bytes", t.audio_total_bytes},
        {"dir", t.dir},
        {"video_file", t.video_file},
        {"audio_file", t.audio_file},
    };
}
inline void from_json(const nlohmann::json& j, DownloadTask& t) {
    if (j.contains("id")) j.at("id").get_to(t.id);
    if (j.contains("bvid")) j.at("bvid").get_to(t.bvid);
    if (j.contains("cid")) j.at("cid").get_to(t.cid);
    if (j.contains("title")) j.at("title").get_to(t.title);
    if (j.contains("cover_url")) j.at("cover_url").get_to(t.cover_url);
    if (j.contains("quality")) j.at("quality").get_to(t.quality);
    if (j.contains("quality_desc")) j.at("quality_desc").get_to(t.quality_desc);
    if (j.contains("is_dash")) j.at("is_dash").get_to(t.is_dash);
    if (j.contains("video_urls")) j.at("video_urls").get_to(t.video_urls);
    if (j.contains("audio_urls")) j.at("audio_urls").get_to(t.audio_urls);
    if (j.contains("flv_segments")) j.at("flv_segments").get_to(t.flv_segments);
    if (j.contains("status")) t.status = static_cast<DownloadTaskStatus>(j.at("status").get<int>());
    if (j.contains("downloaded_bytes")) j.at("downloaded_bytes").get_to(t.downloaded_bytes);
    if (j.contains("total_bytes")) j.at("total_bytes").get_to(t.total_bytes);
    if (j.contains("audio_downloaded_bytes")) j.at("audio_downloaded_bytes").get_to(t.audio_downloaded_bytes);
    if (j.contains("audio_total_bytes")) j.at("audio_total_bytes").get_to(t.audio_total_bytes);
    if (j.contains("dir")) j.at("dir").get_to(t.dir);
    if (j.contains("video_file")) j.at("video_file").get_to(t.video_file);
    if (j.contains("audio_file")) j.at("audio_file").get_to(t.audio_file);
    // allocate fresh atomic flags
    t.cancelFlag = std::make_shared<std::atomic<bool>>(false);
    t.pauseFlag  = std::make_shared<std::atomic<bool>>(false);
}

class DownloadManager : public brls::Singleton<DownloadManager> {
public:
    DownloadManager();
    ~DownloadManager() override;

    // Persist / restore state
    void loadState();
    void saveState();

    // Enqueue a new task (starts download immediately if slots available)
    void addTask(DownloadTask task);

    // Control
    void pauseTask(const std::string& id);
    void resumeTask(const std::string& id);
    void cancelTask(const std::string& id);
    void deleteTask(const std::string& id);  // cancel + remove files + remove task

    // Query
    DownloadTask* getTask(const std::string& id);
    const std::vector<DownloadTask>& getTasks() const;
    std::vector<DownloadTask>& getTasks();

    // Thread-safe snapshot for UI iteration
    std::vector<DownloadTask> getTasksSnapshot() const;

    // Returns true if there are PENDING / DOWNLOADING / PAUSED tasks
    bool hasIncompleteDownloads() const;

    // Events fired on the main thread
    // payload: task id
    brls::Event<std::string> taskProgressEvent;
    brls::Event<std::string> taskStatusChangedEvent;

private:
    void startNextTask();
    void runTask(const std::string& id);

    // Download helpers
    bool downloadFile(const std::string& url,
                      const std::string& filepath,
                      std::atomic<bool>& cancelFlag,
                      std::atomic<bool>& pauseFlag,
                      int64_t& downloadedBytes,
                      int64_t& totalBytes,
                      const std::string& taskId);

    bool downloadDash(DownloadTask& task);
    bool downloadFlv(DownloadTask& task);
    void saveInfoJson(const DownloadTask& task);
    void saveDanmaku(const DownloadTask& task);

    std::vector<DownloadTask> tasks;
    mutable std::mutex tasksMutex;

    static constexpr int MAX_CONCURRENT = 1;
};
