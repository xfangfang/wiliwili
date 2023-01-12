//
// Created by fang on 2022/8/4.
//

#include "activity/live_player_activity.hpp"
#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "view/danmaku_core.hpp"
#include "view/grid_dropdown.hpp"
#include "bilibili.h"

using namespace brls::literals;

LiveActivity::LiveActivity(const bilibili::LiveVideoResult& live)
    : liveData(live) {
    brls::Logger::debug("LiveActivity: create: {}", live.roomid);
    this->setCommonData();
}

LiveActivity::LiveActivity(int roomid) {
    brls::Logger::debug("LiveActivity: create: {}", roomid);
    this->liveData.roomid = roomid;
    this->setCommonData();
}

void LiveActivity::setCommonData() {
    globalShowDanmaku       = DanmakuCore::DANMAKU_ON;
    globalBottomBar         = MPVCore::BOTTOM_BAR;
    DanmakuCore::DANMAKU_ON = false;
    MPVCore::BOTTOM_BAR     = false;
    eventSubscribeID =
        MPVCore::instance().getEvent()->subscribe([this](MpvEventEnum event) {
            switch (event) {
                case MpvEventEnum::QUALITY_CHANGE_REQUEST:
                    this->setVideoQuality();
                    break;
                default:
                    break;
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
    this->video->registerAction(
        "toggleDanmaku", brls::ControllerButton::BUTTON_X,
        [](brls::View* view) -> bool { return true; }, true);

    this->video->hideDanmakuButton();
    this->video->setFullscreenIcon(true);
    this->video->setTitle(liveData.title);
    this->video->setOnlineCount(liveData.watched_show.text_large);
    this->video->setCloseOnEndOfFile(false);

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
            MPVCore::instance().qualityStr = i.desc + " \uE0EF";
            MPVCore::instance().getEvent()->fire(MpvEventEnum::QUALITY_CHANGED);
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
    DanmakuCore::DANMAKU_ON = globalShowDanmaku;
    MPVCore::BOTTOM_BAR     = globalBottomBar;
    // 取消监控mpv
    MPVCore::instance().getEvent()->unsubscribe(eventSubscribeID);
}