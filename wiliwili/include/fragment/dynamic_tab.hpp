//
// Created by fang on 2022/6/9.
//

// register this view in main.cpp
//#include "view/dynamic_tab.hpp"
//    brls::Application::registerXMLView("DynamicTab", DynamicTab::create);
// <brls:View xml=@res/xml/fragment/dynamic_tab.xml

#pragma once

#include <borealis.hpp>
#include "presenter/dynamic_tab.hpp"
#include "presenter/dynamic_video.hpp"
#include "view/auto_tab_frame.hpp"

class RecyclingGrid;
class AutoTabFrame;
class DynamicVideo;

typedef brls::Event<uint> UserSelectedEvent;

class DynamicTab : public AttachedView, public DynamicTabRequest, DynamicVideoRequest{

public:
    DynamicTab();

    ~DynamicTab();

    virtual void onUpList(const bilibili::DynamicUpListResultWrapper &result) override;

    virtual void onError(const string& error) override;

    static View* create();

    void onCreate() override;

    void changeUser(uint mid);

    void onDynamicVideoList(const bilibili::DynamicVideoListResult &result, uint index) override;

private:
    BRLS_BIND(RecyclingGrid, upRecyclingGrid, "dynamic/up/recyclingGrid");
    BRLS_BIND(RecyclingGrid, videoRecyclingGrid, "dynamic/videoList");
};