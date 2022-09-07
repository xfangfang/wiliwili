//
// Created by fang on 2022/8/2.
//

#include "fragment/search_tab.hpp"
#include "fragment/search_video.hpp"
#include "fragment/search_bangumi.hpp"
#include "fragment/search_cinema.hpp"
#include "fragment/search_hots.hpp"

SearchTab::SearchTab() {
    this->inflateFromXMLRes("xml/fragment/search_tab.xml");
    brls::Logger::debug("Fragment SearchTab: create");
}

SearchTab::~SearchTab() {
    brls::Logger::debug("Fragment SearchTabActivity: delete");
}

brls::View *SearchTab::create() {
    return new SearchTab();
}

void SearchTab::requestData(const std::string& key){
    try {
        this->searchVideoTab->requestSearch(key);
        this->searchBangumiTab->requestSearch(key);
        this->searchCinemaTab->requestSearch(key);
        brls::sync([this](){
            this->focusNthTab(1);
        });
    } catch (brls::ViewNotFoundException const& e) {
        brls::Logger::error("ViewNotFoundException: {}", e.what());
    }

}

void SearchTab::passEventToSearchHots(UpdateSearchEvent *updateSearchEvent) {
    this->searchHotsTab->updateSearchEvent = updateSearchEvent;
}


void SearchTab::focusNthTab(int i) {
    this->tabFrame->focusTab(i);
}
