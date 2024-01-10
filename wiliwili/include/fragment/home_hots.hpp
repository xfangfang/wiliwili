//
// Created by fang on 2022/7/11.
//

// register this fragment in main.cpp
//#include "fragment/home_hots.hpp"
//    brls::Application::registerXMLView("HomeHots", HomeHots::create);
// <brls:View xml=@res/xml/fragment/home_hots.xml

#pragma once

#include "view/auto_tab_frame.hpp"

class HomeHots : public AttachedView {
public:
    HomeHots();

    ~HomeHots();

    static View *create();

    void onCreate() override;

private:
    BRLS_BIND(AutoTabFrame, tabFrame, "homeHots/tabFrame");
};