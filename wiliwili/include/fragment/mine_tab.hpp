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
#include "view/auto_tab_frame.hpp"

typedef brls::Event<bilibili::LoginInfo> loginStatusEvent;
class MineHistory;
class MineCollection;

class MineTab : public AttachedView, public UserHome{

public:
    MineTab();

    ~MineTab();

    void onCreate() override;

    void onUserInfo(const bilibili::UserResult& data) override;

    void onUserNotLogin() override;

    static View *create();

private:
    BRLS_BIND(AutoTabFrame, tabFrame, "mine/tab/frame");
    BRLS_BIND(brls::Box, boxGotoUserSpace, "user_home/goto_userspace");
    BRLS_BIND(brls::Image, imageUserAvater, "mine/image/avatar");
    BRLS_BIND(brls::Label, labelUserName, "mine/label/username");
    BRLS_BIND(MineHistory, mineHistory, "mine/history");
    BRLS_BIND(MineCollection, mineCollection, "mine/collection");

    brls::ActionIdentifier boxGotoUserSpaceClickID = -1;
    loginStatusEvent loginCb;
};