//
// Created by fang on 2022/6/9.
//

// register this view in main.cpp
//#include "view/dynamic_tab.hpp"
//    brls::Application::registerXMLView("DynamicTab", DynamicTab::create);
// <brls:View xml=@res/xml/fragment/dynamic_tab.xml

#pragma once

#include <borealis.hpp>

class DynamicTab : public brls::Box {

public:
    DynamicTab();

    ~DynamicTab();

    static View *create() {
        return new DynamicTab();
    }

private:
    // BRLS_BIND(brls::Label, label, "DynamicTab/label")

};