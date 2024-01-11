//
// Created by fang on 2022/6/1.
//

#pragma once

#include "bilibili.h"
#include "bilibili/result/home_result.h"
#include <borealis/core/logger.hpp>
#include "utils/config_helper.hpp"

class UserHome {
public:
    virtual void onError() {}
    virtual void onUserInfo(const bilibili::UserResult& data) {}
    virtual void onUserDynamicStat(const bilibili::UserDynamicCount& data) {}
    virtual void onUserRelationStat(const bilibili::UserRelationStat& data) {}
    virtual void onUserNotLogin() {}

    void requestData() {
        this->getUserInfo();
        auto mid = ProgramConfig::instance().getUserID();
        this->getUserDynamicStat(mid);
        this->getUserRelationStat(mid);
    }

    void getUserInfo() {
        BILI::get_my_info(
            [this](const bilibili::UserResult& data) {
                this->userInfo = data;
                this->onUserInfo(this->userInfo);
            },
            [this](BILI_ERR) {
                brls::Logger::error("getUserInfo: {}", error);
                this->onUserNotLogin();
            });
    }

    void getUserDynamicStat(const std::string& mid) {
        BILI::get_user_dynamic_count(
            mid, [this](const bilibili::UserDynamicCount& data) { this->onUserDynamicStat(data); },
            [](BILI_ERR) { brls::Logger::error("getUserDynamicStat: {}", error); });
    }

    void getUserRelationStat(const std::string& mid) {
        BILI::get_user_relation(
            mid, [this](const bilibili::UserRelationStat& data) { this->onUserRelationStat(data); },
            [](BILI_ERR) { brls::Logger::error("getUserRelationStat: {}", error); });
    }

private:
    bilibili::UserResult userInfo;
};
