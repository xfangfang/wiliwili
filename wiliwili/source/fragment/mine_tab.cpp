//
// Created by fang on 2022/6/9.
//

#include "fragment/mine_tab.hpp"

MineTab::MineTab() {
    this->inflateFromXMLRes("xml/fragment/mine_tab.xml");
    brls::Logger::debug("Fragment MineTab: create");
    this->requestData();
    this->registerAction("refresh", brls::ControllerButton::BUTTON_Y, [this](brls::View* view)-> bool {

        return true;
    });

    BRLS_REGISTER_CLICK_BY_ID("user_home/goto_userspace", [](View *) -> bool{
        auto dialog = new brls::Dialog((brls::Box*)brls::View::createFromXMLResource("tabs/user_home_qr_login.xml"));
        dialog->addButton("Cancel", [](){});
        dialog->open();
        return true;
    });
}

MineTab::~MineTab() {
    brls::Logger::debug("Fragment MineTabActivity: delete");
}