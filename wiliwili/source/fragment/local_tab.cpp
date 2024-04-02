//
// Created by Anonymous on 2024/3/31.
//

#include "fragment/local_tab.hpp"

LocalTab::LocalTab() {
    this->inflateFromXMLRes("xml/fragment/local_tab.xml");
    brls::Logger::debug("Fragment LocalTab: create");
}

LocalTab::~LocalTab() { brls::Logger::debug("Fragment LocalTabActivity: delete"); }

brls::View* LocalTab::create() { return new LocalTab(); }

void LocalTab::onCreate() {

}

