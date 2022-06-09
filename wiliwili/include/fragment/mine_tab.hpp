//
// Created by fang on 2022/6/9.
//

// register this fragment in main.cpp
//#include "fragment/mine_tab.hpp"
//    brls::Application::registerXMLView("MineTab", MineTab::create);
// <brls:View xml=@res/xml/fragment/mine_tab.xml

#pragma once

#include <borealis.hpp>
#include "activity/player_activity.hpp"
#include "presenter/user_home.hpp"
#include "view/video_grid.hpp"

class MineTab : public brls::Box, public UserHome{

public:
    MineTab();

    ~MineTab();

    static View *create() {
        return new MineTab();
    }

private:
    BRLS_BIND(VideoGrid, videoGrid, "user_home/video_grid");
    BRLS_BIND(brls::ScrollingFrame, videoGridScrollingFrame, "user_home/video_scroll");

};