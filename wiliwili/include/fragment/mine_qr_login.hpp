//
// Created by fang on 2022/7/25.
//

// register this fragment in main.cpp
//#include "fragment/mine_qr_login.hpp"
//    brls::Application::registerXMLView("MineQrLogin", MineQrLogin::create);
// <brls:View xml=@res/xml/fragment/mine_qr_login.xml

#pragma once

#include <borealis/core/i18n.hpp>
#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

#include "bilibili.h"
#include "view/qr_image.hpp"

namespace brls {
class Label;
}
class QRImage;

using namespace brls::literals;

typedef brls::Event<bilibili::LoginInfo> loginStatusEvent;

class MineQrLogin : public brls::Box {
public:
    explicit MineQrLogin(loginStatusEvent cb);

    ~MineQrLogin() override;

    static brls::Box* create(const loginStatusEvent& cb);

    void onError();

    void onLoginUrlChange(const std::string& url);

    void onLoginStateChange(const std::string& msg);

    void onLoginSuccess();

    void onLoginError();

    void getLoginUrl();

    void checkLogin();

private:
    BRLS_BIND(QRImage, qrImage, "mine/qr");
    BRLS_BIND(brls::Label, hint, "mine/hint");
    std::string oauthKey;
    std::string login_url;
    bool cancel = false;
    loginStatusEvent loginCb;
};