//
// Created by fang on 2022/8/22.
//

#include "activity/setting_activity.hpp"

SettingActivity::SettingActivity() {
    brls::Logger::debug("SettingActivity: create");
}

void SettingActivity::onContentAvailable() {
    brls::Logger::debug("SettingActivity: onContentAvailable");
}

SettingActivity::~SettingActivity() {
    brls::Logger::debug("SettingActivity: delete");
}