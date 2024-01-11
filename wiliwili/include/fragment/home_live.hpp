//
// Created by fang on 2022/7/12.
//

// register this fragment in main.cpp
//#include "fragment/home_live.hpp"
//    brls::Application::registerXMLView("HomeLive", HomeLive::create);
// <brls:View xml=@res/xml/fragment/home_live.xml

#pragma once

#include "view/auto_tab_frame.hpp"
#include "presenter/home_live.hpp"

namespace brls {
class Label;
};
class RecyclingGrid;

class HomeLive : public AttachedView, public HomeLiveRequest {
public:
    HomeLive();

    void onLiveList(const bilibili::LiveVideoListResult &result, int index) override;

    void switchChannel();

    ~HomeLive() override;

    void onCreate() override;

    void onError(const std::string &error) override;

    static View *create();

private:
    BRLS_BIND(brls::Label, live_label, "home/live/label");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "home/live/recyclingGrid");
    BRLS_BIND(brls::Box, live_box, "home/live/box");
};