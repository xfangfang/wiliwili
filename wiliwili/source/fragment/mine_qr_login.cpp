//
// Created by fang on 2022/7/25.
//

#include <utility>
#include <borealis/core/application.hpp>
#include <borealis/core/thread.hpp>
#include <borealis/views/label.hpp>

#include "fragment/mine_qr_login.hpp"
#include "utils/config_helper.hpp"
#include "bilibili/result/mine_result.h"

MineQrLogin::MineQrLogin(loginStatusEvent cb) : loginCb(std::move(cb)) {
    this->inflateFromXMLRes("xml/fragment/mine_qr_login.xml");
    brls::Logger::debug("Fragment MineQrLogin: create");
    this->getLoginUrl();
}

MineQrLogin::~MineQrLogin() {
    brls::Logger::debug("Fragment MineQrLoginActivity: delete");
    this->cancel = true;
}

brls::Box* MineQrLogin::create(const loginStatusEvent& cb) { return new MineQrLogin(cb); }

void MineQrLogin::onError() {
    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN]() {
        ASYNC_RELEASE
        this->qrImage->setQRMainColor(brls::Application::getTheme().getColor("font/grey"));
        qrImage->setImageFromQRContent("");
        this->hint->setText("wiliwili/mine/login/network_error"_i18n);
    });
}
void MineQrLogin::onLoginUrlChange(const std::string& url) {
    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN, url]() {
        ASYNC_RELEASE
        qrImage->setQRMainColor(RGBA(0, 0, 0, 255));
        qrImage->setImageFromQRContent(url);
    });
}

void MineQrLogin::onLoginStateChange(const std::string& msg) {
    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN, msg]() {
        ASYNC_RELEASE
        this->hint->setText(msg);
    });
}

void MineQrLogin::onLoginSuccess() { this->dismiss(); }

void MineQrLogin::onLoginError() {
    ASYNC_RETAIN
    brls::sync([ASYNC_TOKEN]() {
        ASYNC_RELEASE
        this->qrImage->setImageFromQRContent("");
        this->qrImage->setQRMainColor(brls::Application::getTheme().getColor("font/grey"));
    });
}

void MineQrLogin::getLoginUrl() {
    ASYNC_RETAIN
    BILI::get_login_url_v2(
        [ASYNC_TOKEN](const std::string& url, const std::string& key) {
            brls::sync([ASYNC_TOKEN, url, key]() {
                ASYNC_RELEASE
                this->oauthKey  = key;
                this->login_url = url;
                this->onLoginUrlChange(url);
                this->checkLogin();
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                brls::Logger::error("getLoginUrl: {}", error);
                this->onError();
            });
        });
}

void MineQrLogin::checkLogin() {
    if (cancel) {
        brls::Logger::debug("cancel check login");
        return;
    }
    brls::Logger::debug("check login");
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_login_info_v2(
        this->oauthKey, "wiliwili - " + APPVersion::instance().getPlatform(), ProgramConfig::instance().getDeviceID(),
        [ASYNC_TOKEN](bilibili::LoginInfo info) {
            // 这一句是从网络线程回来的：fire() 会调用订阅者（刷新列表、关闭登录框），
            // 全是界面操作，而且要把存活检查放在它前面。以前 fire() 在 ASYNC_RELEASE
            // 之前、又在网络线程上跑，登录框被关掉之后那个请求仍会 fire 已经释放的
            // 事件对象，PS5 上表现为退出登录后扫码时的 SIGBUS。
            brls::sync([ASYNC_TOKEN, info]() {
                ASYNC_RELEASE
                this->loginCb.fire(info);
                brls::Logger::debug("return code:{}", (int)info);
                switch (info) {
                case bilibili::LoginInfo::OAUTH_KEY_TIMEOUT:
                case bilibili::LoginInfo::OAUTH_KEY_ERROR:
                    this->onLoginError();
                    this->onLoginStateChange("wiliwili/mine/login/qr_not_available"_i18n);
                    break;
                case bilibili::LoginInfo::NEED_CONFIRM:
                    this->onLoginStateChange("wiliwili/mine/login/need_confirm"_i18n);
                    {
                        ASYNC_RETAIN
                        brls::Threading::delay(1000, [ASYNC_TOKEN]() {
                            ASYNC_RELEASE
                            this->checkLogin();
                        });
                    }
                    break;
                case bilibili::LoginInfo::NEED_SCAN:
                    this->onLoginStateChange("wiliwili/mine/login/need_scan"_i18n);
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
                    brls::Logger::error("return unknown code:{}", (int)info);
                    break;
                }
            });
        });
}