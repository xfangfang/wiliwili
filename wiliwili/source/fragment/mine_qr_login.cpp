//
// Created by fang on 2022/7/25.
//

#include "fragment/mine_qr_login.hpp"

MineQrLogin::MineQrLogin(loginStatusEvent cb):loginCb(cb) {
    this->inflateFromXMLRes("xml/fragment/mine_qr_login.xml");
    brls::Logger::debug("Fragment MineQrLogin: create");
    this->getLoginUrl();
}

MineQrLogin::~MineQrLogin() {
    brls::Logger::debug("Fragment MineQrLoginActivity: delete");
    this->cancel = true;
}

brls::Box* MineQrLogin::create(loginStatusEvent cb) {
    return new MineQrLogin(cb);
}