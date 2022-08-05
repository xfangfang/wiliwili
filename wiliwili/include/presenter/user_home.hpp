//
// Created by fang on 2022/6/1.
//

#pragma once

#include "bilibili.h"
#include "bilibili/result/home_result.h"

class UserHome {
public:
    virtual void onError(){}
    virtual void onUserInfo(const bilibili::UserResult& data){}
    virtual void onUserNotLogin(){}

    void requestData() {
        this->getUserInfo();
    }

    void getUserInfo(){
        bilibili::BilibiliClient::get_my_info([this](const bilibili::UserResult& data){
            this->userInfo = data;
            this->onUserInfo(this->userInfo);
        }, [this](const std::string& error){
            this->onUserNotLogin();
        });
    }

private:
    bilibili::UserResult userInfo;
};

