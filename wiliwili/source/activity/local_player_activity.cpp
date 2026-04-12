//
// Created by copilot on 2026/4/6.
//

#include <fstream>
#include <fmt/format.h>
#include <tinyxml2.h>
#include <cpr/filesystem.h>
#include <borealis/core/application.hpp>
#include <borealis/core/logger.hpp>
#include <nlohmann/json.hpp>

#include "activity/local_player_activity.hpp"
#include "utils/download_manager.hpp"
#include "view/danmaku_core.hpp"
#include "view/video_view.hpp"
#include "view/mpv_core.hpp"

#ifdef _WIN32
static const char PATH_SEP_LOCAL = '\\';
#else
static const char PATH_SEP_LOCAL = '/';
#endif

static std::string joinLocalPath(const std::string& dir, const std::string& file) {
    return dir + PATH_SEP_LOCAL + file;
}

LocalPlayerActivity::LocalPlayerActivity(const std::string& id) : taskId(id) {}

LocalPlayerActivity::~LocalPlayerActivity() {
    this->video->stop();
}

void LocalPlayerActivity::onContentAvailable() {
    // Find the task
    auto* task = DownloadManager::instance().getTask(taskId);
    if (!task || task->status != DownloadTaskStatus::COMPLETED) {
        brls::Logger::error("LocalPlayerActivity: task {} not found or not completed", taskId);
        brls::Application::popActivity();
        return;
    }

    video->setTitle(task->title.empty() ? task->bvid : task->title);
    video->setOnlineCount(task->quality_desc);
    video->hideVideoQualityButton();
    video->hideDLNAButton();
    video->hideVideoRelatedSetting();
    video->hideHistorySetting();
    video->hideHighlightLineSetting();
    video->hideSkipOpeningCreditsSetting();
    video->disableCloseOnEndOfFile();
    video->setFullscreenIcon(true);
    video->registerCommonActions(this);

    std::string videoPath = joinLocalPath(task->dir, task->video_file);
    if (!cpr::fs::exists(videoPath)) {
        brls::Logger::error("LocalPlayerActivity: video file not found: {}", videoPath);
        brls::Application::popActivity();
        return;
    }

    if (task->is_dash && !task->audio_file.empty()) {
        // DASH: separate video + audio
        std::string audioPath = joinLocalPath(task->dir, task->audio_file);
        if (cpr::fs::exists(audioPath)) {
            video->setUrl(videoPath, 0, -1, audioPath);
        } else {
            video->setUrl(videoPath, 0, -1);
        }
    } else {
        // FLV (already concatenated)
        video->setUrl(videoPath, 0, -1);
    }

    // Load danmaku if available
    std::string danmakuPath = joinLocalPath(task->dir, "danmaku.xml");
    if (cpr::fs::exists(danmakuPath)) {
        std::ifstream f(danmakuPath);
        if (f.is_open()) {
            std::string xml((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            tinyxml2::XMLDocument document;
            tinyxml2::XMLError error = document.Parse(xml.c_str());
            if (error == tinyxml2::XMLError::XML_SUCCESS) {
                tinyxml2::XMLElement* element = document.RootElement();
                if (element) {
                    std::vector<DanmakuItem> items;
                    for (auto child = element->FirstChildElement(); child != nullptr;
                         child = child->NextSiblingElement()) {
                        if (child->Name()[0] != 'd') continue;
                        const char* content = child->GetText();
                        if (!content) continue;
                        try {
                            items.emplace_back(content, child->Attribute("p"));
                        } catch (...) {}
                    }
                    DanmakuCore::instance().loadDanmakuData(items);
                    brls::Logger::debug("LocalPlayerActivity: loaded {} danmaku items", items.size());
                }
            }
        }
    }
}
