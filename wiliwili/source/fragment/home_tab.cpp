//
// Created by fang on 2022/6/9.
//

#include "fragment/home_tab.hpp"

HomeTab::HomeTab() {
    this->inflateFromXMLRes("xml/fragment/home_tab.xml");
    brls::Logger::debug("Fragment HomeTab: create");

    this->registerAction("搜索", brls::ControllerButton::BUTTON_Y,
                         [this](brls::View* view)-> bool {

        return true;
    });
}

HomeTab::~HomeTab() {
    brls::Logger::debug("Fragment HomeTab: delete");
}