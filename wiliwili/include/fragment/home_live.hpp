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

    void onLiveList(const bilibili::LiveVideoListResult &result, int index,
                    bool no_more) override;

    ~HomeLive();

    void onCreate() override;

    static View *create();

private:
    BRLS_BIND(brls::Label, live_note, "home/live/note");
    BRLS_BIND(brls::Label, live_label, "home/live/label");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "home/live/recyclingGrid");
    ;
};