//
// Created by fang on 2022/7/11.
//

#include "fragment/home_hots.hpp"

#include "utils/shortcut_helper.hpp"

HomeHots::HomeHots() {
    this->inflateFromXMLRes("xml/fragment/home_hots.xml");
    brls::Logger::debug("Fragment HomeHots: create");
}

HomeHots::~HomeHots() { brls::Logger::debug("Fragment HomeHotsActivity: delete"); }

brls::View* HomeHots::create() { return new HomeHots(); }

void HomeHots::onCreate() {
    this->registerTabAction(
        "上一项", brls::ControllerButton::BUTTON_LT,
        ShortcutHelper::getLastSub(),
        [this](brls::View* view) -> bool {
            tabFrame->focus2LastTab();
            return true;
        },
        true);

    this->registerTabAction(
        "下一项", brls::ControllerButton::BUTTON_RT,
        ShortcutHelper::getNextSub(),
        [this](brls::View* view) -> bool {
            tabFrame->focus2NextTab();
            return true;
        },
        true);
}