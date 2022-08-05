//
// Created by fang on 2022/8/2.
//

#include "fragment/search_tab.hpp"
#include "fragment/search_video.hpp"
#include "fragment/search_bangumi.hpp"
#include "fragment/search_cinema.hpp"

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
    } catch (brls::ViewNotFoundException const& e) {
        Logger::error("ViewNotFoundException: {}", e.what());
    }

}