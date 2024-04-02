//
// Created by Anonymous on 2024/4/2.
//

#include "activity/local_player_activity.hpp"

#include "view/video_view.hpp"

LocalPlayerActivity::~LocalPlayerActivity() {
    brls::Logger::debug("LocalPlayerActivity: delete");
    this->video->stop();
}

void LocalPlayerActivity::onContentAvailable() {
    this->video->setPath(this->filepath);
    this->video->setTitle(this->filepath.filename().string());
    MPV_E->fire(MpvEventEnum::RESET);
}
