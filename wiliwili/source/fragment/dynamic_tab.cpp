//
// Created by fang on 2022/6/9.
//

#include "fragment/dynamic_tab.hpp"

DynamicTab::DynamicTab() {
    this->inflateFromXMLRes("xml/fragment/dynamic_tab.xml");
    brls::Logger::debug("Fragment DynamicTab: create");
}

DynamicTab::~DynamicTab() {
    brls::Logger::debug("Fragment DynamicTabActivity: delete");
}