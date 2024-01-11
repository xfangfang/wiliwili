//
// Created by fang on 2022/6/9.
//

#include <borealis/core/touch/tap_gesture.hpp>

#include "activity/search_activity_tv.hpp"
#include "fragment/home_tab.hpp"
#include "view/custom_button.hpp"
#include "utils/activity_helper.hpp"

using namespace brls::literals;

HomeTab::HomeTab() {
    this->inflateFromXMLRes("xml/fragment/home_tab.xml");
    brls::Logger::debug("Fragment HomeTab: create");
}

void HomeTab::onCreate() {
    this->registerTabAction("wiliwili/search/tab"_i18n, brls::ControllerButton::BUTTON_Y, [](brls::View* view) -> bool {
        if (TVSearchActivity::TV_MODE) {
            Intent::openTVSearch();
        } else {
            brls::Application::getImeManager()->openForText([](const std::string& text) { Intent::openSearch(text); },
                                                            "wiliwili/home/common/search"_i18n, "", 32, "", 0);
        }
        return true;
    });

    this->registerTabAction(
        "上一项", brls::ControllerButton::BUTTON_LB,
        [this](brls::View* view) -> bool {
            tabFrame->focus2LastTab();
            return true;
        },
        true);

    this->registerTabAction(
        "下一项", brls::ControllerButton::BUTTON_RB,
        [this](brls::View* view) -> bool {
            tabFrame->focus2NextTab();
            return true;
        },
        true);

    this->search->registerClickAction([](brls::View* view) -> bool {
        HomeTab::openSearch();
        return true;
    });
    this->search->addGestureRecognizer(new brls::TapGestureRecognizer(this->search));

    this->search->setCustomNavigation([this](brls::FocusDirection direction) {
        if (direction == brls::FocusDirection::DOWN) {
            return (brls::View*)this->tabFrame->getActiveTab();
        } else if (direction == brls::FocusDirection::LEFT) {
            return (brls::View*)this->tabFrame->getSidebar();
        }
        return (brls::View*)nullptr;
    });
}

void HomeTab::openSearch() {
    if (TVSearchActivity::TV_MODE) {
        Intent::openTVSearch();
    } else {
        brls::Application::getImeManager()->openForText([](const std::string& text) { Intent::openSearch(text); },
                                                        "wiliwili/home/common/search"_i18n, "", 32, "", 0);
    }
}

brls::View* HomeTab::create() { return new HomeTab(); }

HomeTab::~HomeTab() { brls::Logger::debug("Fragment HomeTab: delete"); }