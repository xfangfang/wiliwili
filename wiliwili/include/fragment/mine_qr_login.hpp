//
// Created by fang on 2022/7/25.
//

// register this fragment in main.cpp
//#include "fragment/mine_qr_login.hpp"
//    brls::Application::registerXMLView("MineQrLogin", MineQrLogin::create);
// <brls:View xml=@res/xml/fragment/mine_qr_login.xml

#pragma once

#include <borealis.hpp>
#include "bilibili.h"
#include "view/qr_image.hpp"

typedef brls::Event<bilibili::LoginInfo> loginStatusEvent;

class MineQrLogin : public brls::Box {
public:
    MineQrLogin(loginStatusEvent cb);

    ~MineQrLogin();

    static brls::Box* create(loginStatusEvent cb);

    void onError() {
        ASYNC_RETAIN
        brls::sync([ASYNC_TOKEN]() {
            ASYNC_RELEASE
            this->qrImage->setQRMainColor(
                brls::Application::getTheme().getColor("font/grey"));
            qrImage->setImageFromQRContent("");
            this->hint->setText("网络错误，请重试");
        });
    }
    void onLoginUrlChange(std::string url) {
        ASYNC_RETAIN
        brls::sync([ASYNC_TOKEN, url]() {
            ASYNC_RELEASE
            qrImage->setQRMainColor(RGBA(0, 0, 0, 255));
            qrImage->setImageFromQRContent(url);
        });
    }

    void onLoginStateChange(std::string msg) {
        ASYNC_RETAIN
        brls::sync([ASYNC_TOKEN, msg]() {
            ASYNC_RELEASE
            this->hint->setText(msg);
        });
    }

    void onLoginSuccess() { this->dismiss(); }

    void onLoginError() {
        ASYNC_RETAIN
        brls::sync([ASYNC_TOKEN]() {
            ASYNC_RELEASE
            this->qrImage->setImageFromQRContent("");
            this->qrImage->setQRMainColor(
                brls::Application::getTheme().getColor("font/grey"));
        });
    }

    void getLoginUrl() {
        ASYNC_RETAIN
        bilibili::BilibiliClient::get_login_url(
            [ASYNC_TOKEN](const std::string& url, const std::string& key) {
                ASYNC_RELEASE
                this->oauthKey  = key;
                this->login_url = url;
                this->onLoginUrlChange(url);
                this->checkLogin();
            },
            [this](const std::string& error) { this->onError(); });
    }

    void checkLogin() {
        if (cancel) {
            brls::Logger::debug("cancel check login");
            return;
        }
        brls::Logger::debug("check login");
        ASYNC_RETAIN
        bilibili::BilibiliClient::get_login_info(
            this->oauthKey, [ASYNC_TOKEN](bilibili::LoginInfo info) {
                this->loginCb.fire(info);
                brls::Logger::debug("return code:{}", info);
                ASYNC_RELEASE
                switch (info) {
                    case bilibili::LoginInfo::OAUTH_KEY_TIMEOUT:
                    case bilibili::LoginInfo::OAUTH_KEY_ERROR:
                        this->onLoginError();
                        this->onLoginStateChange(
                            "二维码失效，请重新进入对话框");
                        break;
                    case bilibili::LoginInfo::NEED_CONFIRM:
                        this->onLoginStateChange("等待确认");
                        {
                            ASYNC_RETAIN
                            brls::Threading::delay(1000, [ASYNC_TOKEN]() {
                                ASYNC_RELEASE
                                this->checkLogin();
                            });
                        }
                        break;
                    case bilibili::LoginInfo::NEED_SCAN:
                        this->onLoginStateChange("等待扫码");
                        {
                            ASYNC_RETAIN
                            brls::Threading::delay(3000, [ASYNC_TOKEN]() {
                                ASYNC_RELEASE
                                this->checkLogin();
                            });
                        }
                        break;
                    case bilibili::LoginInfo::SUCCESS:
                        this->onLoginSuccess();
                        break;
                    default:
                        brls::Logger::error("return unknown code:{}", info);
                        break;
                }
            });
    }

private:
    BRLS_BIND(QRImage, qrImage, "mine/qr");
    BRLS_BIND(brls::Label, hint, "mine/hint");
    std::string oauthKey;
    std::string login_url;
    bool cancel = false;
    loginStatusEvent loginCb;
};