//
// Created by fang on 2022/8/2.
//

#include <borealis/core/thread.hpp>

#include "fragment/search_order.hpp"
#include "fragment/search_video.hpp"
#include "activity/search_activity.hpp"

using namespace brls::literals;

SearchOrder::SearchOrder() {
    this->inflateFromXMLRes("xml/fragment/search_order.xml");
    brls::Logger::debug("Fragment SearchOrder: create");
    this->tabFrame->setRefreshAction([this]() {
        AutoTabFrame::focus2Sidebar(this);
        auto *tab = dynamic_cast<SearchVideo*>(this->tabFrame->getActiveTab());
        if (tab != nullptr) tab->requestSearch(SearchActivity::currentKey);
    });
}

void SearchOrder::focusNthTab(int i) { this->tabFrame->focusTab(i); }

SearchOrder::~SearchOrder() {
    brls::Logger::debug("Fragment SearchOrder: delete");
}

void SearchOrder::onCreate() {
    this->registerTabAction("wiliwili/home/common/refresh"_i18n,
        brls::ControllerButton::BUTTON_X,
        [this](brls::View* view) -> bool {
            this->tabFrame->refresh();
            return true;
        });

    this->registerTabAction(
        "上一项", brls::ControllerButton::BUTTON_LT,
        [this](brls::View* view) -> bool {
            tabFrame->focus2LastTab();
            return true;
        },
        true);

    this->registerTabAction(
        "下一项", brls::ControllerButton::BUTTON_RT,
        [this](brls::View* view) -> bool {
            tabFrame->focus2NextTab();
            return true;
        },
        true);
}

brls::View* SearchOrder::create() { return new SearchOrder(); }