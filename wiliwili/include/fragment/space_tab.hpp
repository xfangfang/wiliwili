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

namespace brls {
class Box;
}

class SpaceTab : public AttachedView, public Home {
public:
    SpaceTab();

    void onRecommendVideoList(const bilibili::RecommendVideoListResultWrapper &result) override;

    ~SpaceTab() override;

    static View *create();

    void onCreate() override;

    void draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext *ctx) override;

    void onLayout() override;

    void willDisappear(bool resetState) override;

    void willAppear(bool resetState) override;

    void onResume();

    void onError(const std::string &error) override;

private:
    BRLS_BIND(brls::Box, recyclingGrid, "space/container");
    brls::Rect oldRect = brls::Rect(-1, -1, -1, -1);
};