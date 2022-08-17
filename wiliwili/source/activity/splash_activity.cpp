//
// Created by fang on 2022/7/16.
//

#include "activity/splash_activity.hpp"

SplashActivity::SplashActivity() {
    brls::Logger::debug("SplashActivityActivity: create");
}

void SplashActivity::onContentAvailable() {
    brls::Logger::debug("SplashActivityActivity: onContentAvailable");
}


SplashActivity::~SplashActivity() {
    brls::Logger::debug("SplashActivityActivity: delete");
}

