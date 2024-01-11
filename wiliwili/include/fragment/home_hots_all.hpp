//
// Created by fang on 2022/7/6.
//

// register this fragment in main.cpp
//#include "fragment/home_hots_all.hpp"
//    brls::Application::registerXMLView("HomeHotsAll", HomeHotsAll::create);
// <brls:View xml=@res/xml/fragment/home_hots_all.xml

#pragma once

#include "presenter/home_hots_all.hpp"
#include "view/auto_tab_frame.hpp"

class RecyclingGrid;

class HomeHotsAll : public AttachedView, public HomeHotsAllRequest {
public:
    HomeHotsAll();

    ~HomeHotsAll();

    void onHotsAllVideoList(const bilibili::HotsAllVideoListResult &result, int index) override;

    static View *create();

    void onCreate() override;

    void onError(const std::string &error) override;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "home/hots/all/recyclingGrid");
};