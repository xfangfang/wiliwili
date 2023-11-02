//
// Created by fang on 2022/8/4.
//

#include "activity/live_player_activity.hpp"
#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "view/danmaku_core.hpp"
#include "view/subtitle_core.hpp"
#include "view/grid_dropdown.hpp"
#include "utils/shader_helper.hpp"
#include "utils/config_helper.hpp"
#include "live/danmaku_live.hpp"
#include "live/extract_messages.hpp"
#include "live/ws_utils.hpp"
#include "bilibili.h"

using namespace brls::literals;

static void process_danmaku(const danmaku_t* dan) {
    //TODO:做其他处理
    //...

    brls::Logger::debug("process_danmaku: {}", dan->dan);

    //弹幕加载到视频中去
    DanmakuCore::instance().addLiveDanmaku(DanmakuItem(dan));

    danmaku_t_free(dan);
}

static void onDanmakuReceived(std::string&& message) {
    const std::string& msg = message;
    std::vector<uint8_t> payload(msg.begin(), msg.end());
    std::vector<std::string> messages = parse_packet(payload);

    if (messages.size() == 0) {
        return;
    }

    for (const auto& live_msg : extract_messages(messages)) {
        if (live_msg.type == danmaku) {
            if (!live_msg.ptr) continue;
            process_danmaku((danmaku_t*)live_msg.ptr);
            free(live_msg.ptr);
        } else if (live_msg.type == watched_change) {
            //TODO: 更新在线人数
            free(live_msg.ptr);
        }
    }
}

LiveActivity::LiveActivity(const bilibili::LiveVideoResult& live)
    : liveData(live) {
    brls::Logger::debug("LiveActivity: create: {}", live.roomid);
    MPVCore::instance().command_async("set", "loop-playlist", "force");
    this->setCommonData();
    GA("open_live", {{"id", std::to_string(live.roomid)}})
    LiveDanmaku::instance().setonMessage(onDanmakuReceived);
    DanmakuCore::instance().live_mode_on();
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
    LiveDanmaku::instance().setonMessage(onDanmakuReceived);
    DanmakuCore::instance().live_mode_on();
}

void LiveActivity::setCommonData() {
    DanmakuCore::instance().reset();
    LiveDanmaku::instance().connect(
        liveData.roomid, std::stoi(ProgramConfig::instance().getUserID()));

    // 清空字幕
    SubtitleCore::instance().reset();

    // 清空自定义着色器
    ShaderHelper::instance().clearShader(false);

    eventSubscribeID =
        MPV_CE->subscribe([this](const std::string& event, void* data) {
            if (event == VideoView::QUALITY_CHANGE) {
                this->setVideoQuality();
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

    this->video->hideDLNAButton();
    this->video->hideSubtitleSetting();
    this->video->hideVideoRelatedSetting();
    this->video->hideVideoSpeedButton();
    this->video->hideBottomLineSetting();
    this->video->disableCloseOnEndOfFile();
    this->video->setFullscreenIcon(true);
    this->video->setTitle(liveData.title);
    this->video->setOnlineCount(liveData.watched_show.text_large);

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
    DanmakuCore::instance().live_mode_off();
    // 取消监控mpv
    MPV_CE->unsubscribe(eventSubscribeID);
    MPVCore::instance().command_async("set", "loop-playlist", "1");
    DanmakuCore::instance().live_danmaku_reset();
}
