//
// Created by fang on 2022/7/16.
//

#include "activity/splash_activity.hpp"
#include "view/video_view.hpp"



SplashActivity::SplashActivity() {
    brls::Logger::debug("SplashActivityActivity: create");
}

void SplashActivity::onContentAvailable() {
    brls::Logger::debug("SplashActivityActivity: onContentAvailable");

//    this->video->setUrl("http://vjs.zencdn.net/v/oceans.mp4");
//    this->video->resume();

}


SplashActivity::~SplashActivity() {
    brls::Logger::debug("SplashActivityActivity: delete");
}

