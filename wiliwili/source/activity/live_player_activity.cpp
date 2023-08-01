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
#include "live/danmaku_live.hpp"
#include "live/extract_messages.hpp"
#include "live/ws_utils.hpp"
#include "bilibili.h"

using namespace brls::literals;

static char comma = ',';
static std::string tem = "0,0,0,0,";//临时方案
#define t_s(x) std::to_string(x)
static void process_danmaku(danmaku_t *dan){
    //做其他处理
    //...

    //弹幕加载到视频中去
    double time = MPVCore::instance().getPlaybackTime() + 0.1;
    std::string combined_attr = t_s(time) + comma + t_s(dan->dan_type) + comma
                                + t_s(dan->dan_size) + comma + t_s(dan->dan_color) + comma
                                + tem + t_s(dan->user_level);
    DanmakuCore::instance().addSingleDanmaku(DanmakuItem(std::move(dan->dan), combined_attr));
}

static void onDanmakuReceived(std::string&& message) {
    const std::string& msg = message;
    std::vector<uint8_t> payload(msg.begin(), msg.end());
    std::vector<std::string> messages = parse_packet(payload);

    if(messages.size() == 0){
        return;
    }

    for(auto &&live_msg : extract_messages(messages)){
        if(live_msg.type == danmaku){
            process_danmaku((danmaku_t *)live_msg.ptr);
            free(live_msg.ptr);
        }
        
    }
}

LiveActivity::LiveActivity(const bilibili::LiveVideoResult& live)
    : liveData(live) {
    brls::Logger::debug("LiveActivity: create: {}", live.roomid);
    MPVCore::instance().command_str("set loop-playlist force");
    this->setCommonData();
    GA("open_live", {{"id", std::to_string(live.roomid)}})
    LiveDanmaku::instance().setonMessage(onDanmakuReceived);
}

LiveActivity::LiveActivity(int roomid, const std::string& name,
                           const std::string& views) {
    brls::Logger::debug("LiveActivity: create: {}", roomid);
    MPVCore::instance().command_str("set loop-playlist force");
    this->liveData.roomid                  = roomid;
    this->liveData.title                   = name;
    this->liveData.watched_show.text_large = views;
    this->setCommonData();
    GA("open_live", {{"id", std::to_string(roomid)}})
    LiveDanmaku::instance().setonMessage(onDanmakuReceived);
}

void LiveActivity::setCommonData() {
    DanmakuCore::instance().reset();
    LiveDanmaku::instance().connect(liveData.roomid, 0 /*liveData.uid*/);

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
        BaseDropdown::text(
            "wiliwili/player/quality"_i18n, this->getQualityDescriptionList(),
            [this](int selected) {
                defaultQuality = liveUrl.quality_description[selected].qn;
                this->requestData(this->liveData.roomid);
            },
            this->getCurrentQualityIndex());
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
    // 取消监控mpv
    MPV_CE->unsubscribe(eventSubscribeID);
    MPVCore::instance().command_str("set loop-playlist 1");
}
