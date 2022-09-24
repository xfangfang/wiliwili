//
// Created by fang on 2022/8/4.
//

#include "activity/live_player_activity.hpp"
#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "bilibili.h"

LiveActivity::LiveActivity(const bilibili::LiveVideoResult& live)
    : liveData(live) {
    brls::Logger::debug("LiveActivity: create: {}", live.roomid);

    globalShowDanmaku               = MPVCore::instance().showDanmaku;
    globalBottomBar                 = MPVCore::instance().BOTTOM_BAR;
    MPVCore::instance().showDanmaku = false;
    MPVCore::instance().BOTTOM_BAR  = false;
}

LiveActivity::LiveActivity(int roomid) {
    brls::Logger::debug("LiveActivity: create: {}", roomid);
    this->liveData.roomid = roomid;

    globalShowDanmaku               = MPVCore::instance().showDanmaku;
    globalBottomBar                 = MPVCore::instance().BOTTOM_BAR;
    MPVCore::instance().showDanmaku = false;
    MPVCore::instance().BOTTOM_BAR  = false;
}

void LiveActivity::onContentAvailable() {
    brls::Logger::debug("LiveActivity: onContentAvailable");

    this->video->registerAction("", brls::BUTTON_B, [](...) {
        brls::Application::popActivity();
        return true;
    });
    this->video->registerAction(
        "toggleDanmaku", brls::ControllerButton::BUTTON_X,
        [](brls::View* view) -> bool { return true; }, true);

    this->video->hideDanmakuButton();

    // 使用api接口提供的播放链接，清晰度不高, switch上播放会报错退出
    // if (!liveData.play_url.empty()) {
    //     this->video->start(liveData.play_url);
    // }
    brls::Logger::debug("live default url: {}", liveData.play_url);

    // 根据房间号重新获取高清播放链接
    this->requestData(liveData.roomid);
}

void LiveActivity::onLiveData(const bilibili::LiveUrlResultWrapper& result) {
    for (auto i : result.durl) {
        brls::Logger::debug("palyurl: {}", i.url);
        this->video->start(i.url);
        break;
    }
}

void LiveActivity::onError(const std::string& error) {
    brls::Logger::error("ERROR request live data: {}", error);
}

LiveActivity::~LiveActivity() {
    brls::Logger::debug("LiveActivity: delete");
    this->video->stop();
    MPVCore::instance().showDanmaku = globalShowDanmaku;
    MPVCore::instance().BOTTOM_BAR  = globalBottomBar;
}