//
// Created by fang on 2022/8/2.
//

#include <borealis/core/thread.hpp>

#include "fragment/search_tab.hpp"
#include "fragment/search_video.hpp"
#include "fragment/search_bangumi.hpp"
#include "fragment/search_cinema.hpp"
#include "fragment/search_hots.hpp"
#include "fragment/search_history.hpp"

SearchTab::SearchTab() {
    this->inflateFromXMLRes("xml/fragment/search_tab.xml");
    brls::Logger::debug("Fragment SearchTab: create");

    this->registerAction(
        "上一项", brls::ControllerButton::BUTTON_LB,
        [this](brls::View* view) -> bool {
            tabFrame->focus2LastTab();
            return true;
        },
        true);

    this->registerAction(
        "下一项", brls::ControllerButton::BUTTON_RB,
        [this](brls::View* view) -> bool {
            tabFrame->focus2NextTab();
            return true;
        },
        true);
}

SearchTab::~SearchTab() { brls::Logger::debug("Fragment SearchTabActivity: delete"); }

brls::View* SearchTab::create() { return new SearchTab(); }

void SearchTab::requestData(const std::string& key) {
    try {
        this->searchVideoTab->requestSearch(key);
        this->searchBangumiTab->requestSearch(key);
        this->searchCinemaTab->requestSearch(key);
        this->searchHistoryTab->requestHistory();
        brls::sync([this]() { this->focusNthTab(2); });
    } catch (brls::ViewNotFoundException const& e) {
        brls::Logger::error("ViewNotFoundException: {}", e.what());
    }
}

void SearchTab::focusNthTab(int i) { this->tabFrame->focusTab(i); }

SearchHistory* SearchTab::getSearchHistoryTab() { return searchHistoryTab; };

SearchHots* SearchTab::getSearchHotsTab() { return searchHotsTab; }

SearchVideo* SearchTab::getSearchVideoTab() { return searchVideoTab; }

SearchBangumi* SearchTab::getSearchBangumiTab() { return searchBangumiTab; }

SearchCinema* SearchTab::getSearchCinemaTab() { return searchCinemaTab; }
