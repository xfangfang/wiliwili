//
// Created by fang on 2022/6/1.
//

#pragma once

#include "bilibili.h"
#include "bilibili/result/home_result.h"

class UserHome {
public:
//    virtual void onRecommendVideoList(const bilibili::RecommendVideoListResult &result){}
    virtual void onError(){}
    virtual void onLoginUrlChange(std::string url){}
    virtual void onLoginStateChange(std::string msg){}
    virtual void onLoginSuccess(){}
    virtual void onLoginError(){}

    void requestData() {
//        this->requestRecommendVideoList(1, 24);
    }

    void requestLogin() {
        this->getLoginUrl();
    }

    void getLoginUrl(){
        bilibili::BilibiliClient::get_login_url([this](std::string url, std::string oauthKey){
            brls::Logger::debug("url:{} oauth:{}",url,oauthKey);
            this->login_url = url;
            this->oauthKey = oauthKey;
            this->onLoginUrlChange(url);
            this->checkLogin();
        });
    }

    void checkLogin(){
        brls::Logger::debug("check login");
        bilibili::BilibiliClient::get_login_info(this->oauthKey, [this](bilibili::LoginInfo info){
            brls::Logger::debug("return code:{}",info);
            switch(info){
                case bilibili::LoginInfo::OAUTH_KEY_TIMEOUT:
                case bilibili::LoginInfo::OAUTH_KEY_ERROR:
                    this->onLoginError();
                    this->onLoginStateChange("need refresh qrcode");
                    break;
                case bilibili::LoginInfo::NEED_CONFIRM:
                    this->onLoginStateChange("wait for confirm");
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    this->checkLogin();
                    break;
                case bilibili::LoginInfo::NEED_SCAN:
                    this->onLoginStateChange("wait for scan");
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                    this->checkLogin();
                    break;
                case bilibili::LoginInfo::SUCCESS:
                    this->onLoginStateChange("success");
                    this->onLoginSuccess();
                    break;
                default:
                    brls::Logger::error("return unknown code:{}",info);
                    break;
            }
        });
    }

//    void requestRecommendVideoList(int index = 1, int num = 24) {
//        Logger::debug("max threads: {}",CPR_DEFAULT_THREAD_POOL_MAX_THREAD_NUM);
//        bilibili::BilibiliClient::get_recommend(index, num,
//                                                [this](const bilibili::RecommendVideoListResult &result){
//                                                    brls::Threading::sync([this, result]() {
//                                                        //todo: 当还没获取到推荐列表时，切换页面会销毁当前窗口，从而导致this不可用
//                                                        //解决方案：sidebar切换页面不销毁。
//                                                        this->onRecommendVideoList(result);
//                                                    });
//                                                }, [](const std::string &error) {
//                    Logger::error(error);
//                });
//    }

private:
    std::string oauthKey;
    std::string login_url;
};

