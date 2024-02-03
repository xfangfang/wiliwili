//
// Created by fang on 2022/8/2.
//

// register this fragment in main.cpp
//#include "fragment/search_order.hpp"
//    brls::Application::registerXMLView("SearchOrder", SearchOrder::create);
// <brls:View xml=@res/xml/fragment/search_video.xml

#pragma once

#include "view/auto_tab_frame.hpp"

class SearchOrder : public AttachedView {
public:
    SearchOrder();

    ~SearchOrder() override;

    void focusNthTab(int i);

    void onCreate() override;

    static View* create();

private:
    BRLS_BIND(AutoTabFrame, tabFrame, "search/video/tabFrame");
};