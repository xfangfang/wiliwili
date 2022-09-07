//
// Created by fang on 2022/7/14.
//

// register this fragment in main.cpp
//#include "fragment/home_bangumi.hpp"
//    brls::Application::registerXMLView("HomeBangumi", HomeBangumi::create);
// <brls:View xml=@res/xml/fragment/home_bangumi.xml

#pragma once

#include "presenter/home_pgc.hpp"
#include "view/auto_tab_frame.hpp"

class HomeBangumi : public AttachedView, public HomeBangumiRequest {
public:
    HomeBangumi();

    void onCreate() override;

    void onBangumiList(const bilibili::PGCResultWrapper &result) override;

    ~HomeBangumi();

    static View *create();

    void onError(const std::string &error) override;

private:
    BRLS_BIND(AutoTabFrame, tabFrame, "homeBangumi/tabFrame");
};