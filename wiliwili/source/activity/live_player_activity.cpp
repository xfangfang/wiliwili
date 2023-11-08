//
// Created by fang on 2022/8/4.
//

#include "activity/live_player_activity.hpp"
#include "utils/number_helper.hpp"

#include <vector>
#include <chrono>

#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "view/live_core.hpp"
#include "view/grid_dropdown.hpp"

#include "utils/shader_helper.hpp"
#include "utils/config_helper.hpp"

#include "live/danmaku_live.hpp"
#include "live/extract_messages.hpp"
#include "live/ws_utils.hpp"

#include "bilibili.h"

#include "borealis/core/thread.hpp"

using namespace brls::literals;

static void process_danmaku(const std::vector<LiveDanmakuItem>& danmaku_list) {
    //TODO:做其他处理
    //...

    //弹幕加载到视频中去
    LiveDanmakuCore::instance().add(danmaku_list);

    // danmaku_t_free(dan);
}

static void onDanmakuReceived(std::string&& message) {
    const std::string& msg = message;
    std::vector<uint8_t> payload(msg.begin(), msg.end());
    std::vector<std::string> messages = parse_packet(payload);

    if (messages.size() == 0) {
        return;
    }

    std::vector<LiveDanmakuItem> danmaku_list;

    for (const auto& live_msg : extract_messages(messages)) {
        if (live_msg.type == danmaku) {
            if (!live_msg.ptr) continue;
            danmaku_list.emplace_back(
                LiveDanmakuItem((danmaku_t*)live_msg.ptr));
            // free(live_msg.ptr);
        } else if (live_msg.type == watched_change) {
            //TODO: 更新在线人数
            free(live_msg.ptr);
        }
    }
    process_danmaku(danmaku_list);
}

LiveActivity::LiveActivity(const bilibili::LiveVideoResult& live)
    : liveData(live) {
    brls::Logger::debug("LiveActivity: create: {}", live.roomid);
    MPVCore::instance().command_async("set", "loop-playlist", "force");
    this->setCommonData();
    GA("open_live", {{"id", std::to_string(live.roomid)}})
    GA("open_live", {{"live_id", std::to_string(live.roomid)}})
    LiveDanmaku::instance().setonMessage(onDanmakuReceived);
}

LiveActivity::LiveActivity(int roomid, const std::string& name,
                           const std::string& views) {
    brls::Logger::debug("LiveActivity: create: {}", roomid);
    MPVCore::instance().command_async("set", "loop-playlist", "force");
    this->liveData.roomid                  = roomid;
    this->liveData.title                   = name;
    this->liveData.watched_show.text_large = views;
    this->setCommonData();
    GA("open_live", {{"id", std::to_string(roomid)}})
    GA("open_live", {{"live_id", std::to_string(roomid)}})
    LiveDanmaku::instance().setonMessage(onDanmakuReceived);
}

void LiveActivity::setCommonData() {
    LiveDanmaku::instance().connect(
        liveData.roomid, std::stoi(ProgramConfig::instance().getUserID()));

    // 重置播放器
    MPVCore::instance().reset();

    // 清空自定义着色器
    ShaderHelper::instance().clearShader(false);

    event_id = MPV_CE->subscribe([this](const std::string& event, void* data) {
        if (event == VideoView::QUALITY_CHANGE) {
            this->setVideoQuality();
        }
    });
    tl_event_id = MPV_E->subscribe([this](MpvEventEnum e) {
        if (e == UPDATE_PROGRESS) {
            if (!LiveDanmaku::instance().live_time) return;
            std::chrono::time_point<std::chrono::system_clock> _zero;
            size_t now = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - _zero)
                             .count();
            this->video->setStatusLabelLeft(
                wiliwili::sec2Time(now - LiveDanmaku::instance().live_time));
        }
    });
}

void LiveActivity::setVideoQuality() {
    if (this->liveUrl.quality_description.empty()) return;

    brls::sync([this]() {
        auto dropdown = BaseDropdown::text(
            "wiliwili/player/quality"_i18n, this->getQualityDescriptionList(),
            [this](int selected) {
                defaultQuality = liveUrl.quality_description[selected].qn;
                this->requestData(this->liveData.roomid);
            },
            this->getCurrentQualityIndex());

        dropdown->registerAction(
            "", brls::ControllerButton::BUTTON_START,
            [dropdown](...) {
                dropdown->dismiss();
                return true;
            },
            true);
    });
}

void LiveActivity::onContentAvailable() {
    brls::Logger::debug("LiveActivity: onContentAvailable");

    this->video->registerAction("", brls::BUTTON_B, [](...) {
        brls::Logger::debug("exit live");
        brls::Application::popActivity();
        return true;
    });

    this->video->setLiveMode();
    this->video->hideVideoProgressSlider();
    this->video->hideDLNAButton();
    this->video->hideSubtitleSetting();
    this->video->hideVideoRelatedSetting();
    this->video->hideVideoSpeedButton();
    this->video->hideBottomLineSetting();
    this->video->disableCloseOnEndOfFile();
    this->video->setFullscreenIcon(true);
    this->video->setTitle(liveData.title);
    this->video->setOnlineCount(liveData.watched_show.text_large);
    this->video->setStatusLabelLeft("");
    this->video->setCustomToggleAction([this]() {
        if (MPVCore::instance().isStopped()) {
            this->onLiveData(this->liveUrl);
        } else if (MPVCore::instance().isPaused()) {
            MPVCore::instance().resume();
        } else {
            this->video->showOSD(false);
            MPVCore::instance().pause();
            ASYNC_RETAIN
            brls::delay(5000, [ASYNC_TOKEN]() {
                ASYNC_RELEASE
                if (MPVCore::instance().isPaused()) {
                    MPVCore::instance().stop();
                }
            });
        }
    });

    // 调整清晰度
    this->registerAction("wiliwili/player/quality"_i18n,
                         brls::ControllerButton::BUTTON_START,
                         [this](brls::View* view) -> bool {
                             this->setVideoQuality();
                             return true;
                         });

    // 根据房间号重新获取高清播放链接
    this->requestData(liveData.roomid);
}

std::vector<std::string> LiveActivity::getQualityDescriptionList() {
    std::vector<std::string> res;
    for (auto& i : liveUrl.quality_description) {
        res.push_back(i.desc);
    }
    return res;
}

int LiveActivity::getCurrentQualityIndex() {
    for (size_t i = 0; i < liveUrl.quality_description.size(); i++) {
        if (liveUrl.quality_description[i].qn == this->liveUrl.current_qn)
            return i;
    }
    return 0;
}

void LiveActivity::onLiveData(const bilibili::LiveUrlResultWrapper& result) {
    brls::Logger::debug("current quality: {}", result.current_qn);
    for (auto& i : result.quality_description) {
        brls::Logger::debug("quality: {}/{}", i.desc, i.qn);
        if (result.current_qn == i.qn) {
            std::string quality = i.desc + " \uE0EF";
            MPV_CE->fire(VideoView::SET_QUALITY, (void*)quality.c_str());
        }
    }
    for (const auto& i : result.durl) {
        brls::Logger::debug("Live stream url: {}", i.url);
        this->video->setUrl(i.url);
        break;
    }
}

void LiveActivity::onError(const std::string& error) {
    brls::Logger::error("ERROR request live data: {}", error);
}

LiveActivity::~LiveActivity() {
    brls::Logger::debug("LiveActivity: delete");
    this->video->stop();
    LiveDanmaku::instance().disconnect();
    // 取消监控mpv
    MPV_CE->unsubscribe(event_id);
    MPV_E->unsubscribe(tl_event_id);
    MPVCore::instance().command_async("set", "loop-playlist", "1");
    LiveDanmakuCore::instance().reset();
}
