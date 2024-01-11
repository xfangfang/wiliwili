//
// Created by fang on 2022/6/14.
//

// register this fragment in main.cpp
//#include "fragment/home_recommends.hpp"
//    brls::Application::registerXMLView("HomeRecommends", HomeRecommends::create);
// <brls:View xml=@res/xml/fragment/home_recommends.xml

#pragma once

#include "presenter/home_recommends.hpp"
#include "view/auto_tab_frame.hpp"

class RecyclingGrid;

class HomeRecommends : public AttachedView, public Home {
public:
    HomeRecommends();

    void onRecommendVideoList(const bilibili::RecommendVideoListResultWrapper &result) override;

    ~HomeRecommends();

    static View *create();

    void onCreate() override;

    void onError(const std::string &error) override;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "home/recommends/recyclingGrid");
};