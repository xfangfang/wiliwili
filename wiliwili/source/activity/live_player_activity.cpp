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
    this->setCommonData();
    GA("open_live", {{"id", std::to_string(live.roomid)}})
    GA("open_live", {{"live_id", std::to_string(live.roomid)}})
    LiveDanmaku::instance().setonMessage(onDanmakuReceived);
}

LiveActivity::LiveActivity(int roomid, const std::string& name,
                           const std::string& views) {
    brls::Logger::debug("LiveActivity: create: {}", roomid);
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
            if (!liveRoomPlayInfo.live_time) return;
            std::chrono::time_point<std::chrono::system_clock> _zero;
            size_t now = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - _zero)
                             .count();
            this->video->setStatusLabelLeft(
                wiliwili::sec2Time(now - liveRoomPlayInfo.live_time));
        } else if (e == MPV_FILE_ERROR) {
            this->video->showOSD(false);
            this->video->setStatusLabelLeft("播放错误");

            switch (MPVCore::instance().mpv_error_code) {
                case MPV_ERROR_UNKNOWN_FORMAT:
                    this->video->setOnlineCount("暂不支持当前视频格式");
                    break;
                case MPV_ERROR_LOADING_FAILED:
                    this->video->setOnlineCount("加载失败");
                    // 加载失败时，获取直播间信息，查看是否直播间已经关闭
                    // 如果直播间信息获取失败，则认定为断网，每隔N秒重试一次
                    this->requestData(liveData.roomid);
                default:
                    this->video->setOnlineCount(
                        {mpv_error_string(MPVCore::instance().mpv_error_code)});
            }
        } else if (e == END_OF_FILE) {
            // flv 直播遇到网络错误不会报错，而是输出 END_OF_FILE
            // 直播间关闭时也可能进入这里
            this->video->setOnlineCount("加载失败");
            this->requestData(liveData.roomid);
        }
    });
}

void LiveActivity::setVideoQuality() {
    if (this->liveUrl.accept_qn.empty()) return;

    brls::sync([this]() {
        auto dropdown = BaseDropdown::text(
            "wiliwili/player/quality"_i18n, this->getQualityDescriptionList(),
            [this](int selected) {
                defaultQuality = liveUrl.accept_qn[selected];
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
            this->onLiveData(this->liveRoomPlayInfo);
        } else if (MPVCore::instance().isPaused()) {
            MPVCore::instance().resume();
        } else {
            this->video->showOSD(false);
            MPVCore::instance().pause();
            brls::cancelDelay(toggleDelayIter);
            ASYNC_RETAIN
            toggleDelayIter = brls::delay(5000, [ASYNC_TOKEN]() {
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
    for (auto& i : liveUrl.accept_qn) {
        res.push_back(getQualityDescription(i));
    }
    return res;
}

int LiveActivity::getCurrentQualityIndex() {
    for (size_t i = 0; i < liveUrl.accept_qn.size(); i++) {
        if (liveUrl.accept_qn[i] == this->liveUrl.current_qn) return i;
    }
    return 0;
}

void LiveActivity::onLiveData(const bilibili::LiveRoomPlayInfo& result) {
    // todo：定时获取在线人数
    this->video->setOnlineCount(liveData.watched_show.text_large);

    if (result.live_status != 1) {
        // 未开播
        brls::Logger::error("LiveActivity: not live");
        this->video->showOSD(false);
        this->video->setStatusLabelLeft("未开播");
        return;
    }
    brls::Logger::debug("current quality: {}", liveUrl.current_qn);
    for (auto& i : liveUrl.accept_qn) {
        auto desc = getQualityDescription(i);
        brls::Logger::debug("live quality: {}/{}", desc, i);
        if (liveUrl.current_qn == i) {
            std::string quality = desc + " \uE0EF";
            MPV_CE->fire(VideoView::SET_QUALITY, (void*)quality.c_str());
        }
    }
    // todo: 允许使用备用链接
    for (const auto& i : liveUrl.url_info) {
        auto url = i.host + liveUrl.base_url + i.extra;

        // 设置视频链接
        brls::Logger::debug("Live stream url: {}", url);
        this->video->setUrl(url);
        break;
    }
}

void LiveActivity::onError(const std::string& error) {
    brls::Logger::error("ERROR request live data: {}", error);
    this->video->showOSD(false);
    this->video->setOnlineCount(error);

    // 每隔1秒自动重试
    brls::cancelDelay(errorDelayIter);
    ASYNC_RETAIN
    errorDelayIter = brls::delay(1000, [ASYNC_TOKEN]() {
        ASYNC_RELEASE
        this->requestData(liveData.roomid);
    });
}

LiveActivity::~LiveActivity() {
    brls::Logger::debug("LiveActivity: delete");
    LiveDanmaku::instance().disconnect();
    // 取消监控mpv
    MPV_CE->unsubscribe(event_id);
    MPV_E->unsubscribe(tl_event_id);
    // 在取消监控之后再停止播放器，避免在播放器停止时触发事件 (尤其是：END_OF_FILE)
    this->video->stop();
    LiveDanmakuCore::instance().reset();
    brls::cancelDelay(toggleDelayIter);
    brls::cancelDelay(errorDelayIter);
}
