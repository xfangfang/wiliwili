//
// Created by fang on 2022/8/4.
//

#include "activity/live_player_activity.hpp"
#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "bilibili.h"

using namespace brls::literals;

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
        brls::Logger::debug("exit live");
        brls::Application::popActivity();
        return true;
    });
    this->video->registerAction(
        "toggleDanmaku", brls::ControllerButton::BUTTON_X,
        [](brls::View* view) -> bool { return true; }, true);

    this->video->hideDanmakuButton();
    this->video->setFullscreenIcon(true);
    this->video->setTitle(liveData.title);
    this->video->setOnlineCount(liveData.watched_show.text_large);
    this->video->setCloseOnEndOfFile(false);

    // 调整清晰度
    this->registerAction(
        "wiliwili/player/quality"_i18n, brls::ControllerButton::BUTTON_START,
        [this](brls::View* view) -> bool {
            if (this->liveUrl.quality_description.empty()) return true;
            brls::Application::pushActivity(
                new brls::Activity(new brls::Dropdown(
                    "wiliwili/player/quality"_i18n,
                    this->getQualityDescriptionList(),
                    [this](int _selected) {
                        defaultQuality =
                            liveUrl.quality_description[_selected].qn;
                        this->requestData(this->liveData.roomid);
                    },
                    this->getCurrentQualityIndex())));

            return true;
        });

    // 使用api接口提供的播放链接，清晰度不高, switch上播放会报错退出
    // if (!liveData.play_url.empty()) {
    //     this->video->start(liveData.play_url);
    // }
    brls::Logger::debug("live default url: {}", liveData.play_url);

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
    }
    for (auto i : result.durl) {
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
    MPVCore::instance().showDanmaku = globalShowDanmaku;
    MPVCore::instance().BOTTOM_BAR  = globalBottomBar;
}