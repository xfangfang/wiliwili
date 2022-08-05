//
// Created by fang on 2022/8/4.
//

#include "activity/live_player_activity.hpp"
#include "view/video_view.hpp"
#include "bilibili.h"

LiveActivity::LiveActivity(const bilibili::LiveVideoResult& live):liveData(live) {
    brls::Logger::debug("LiveActivity: create");
}

void LiveActivity::onContentAvailable() {
    brls::Logger::debug("LiveActivity: onContentAvailable");

    if(!liveData.play_url.empty()){
        this->video->start(liveData.play_url);
    }

    bilibili::BilibiliClient::get_live_url(liveData.roomid, [this](const bilibili::LiveUrlResultWrapper& result){
        for(auto i : result.durl){
            this->video->start(i.url);
            break;
        }

    });
}

LiveActivity::~LiveActivity() {
    brls::Logger::debug("LiveActivity: delete");
    this->video->stop();
}