//
// Created by fang on 2022/6/9.
//

// register this view in main.cpp
//#include "view/dynamic_tab.hpp"
//    brls::Application::registerXMLView("DynamicTab", DynamicTab::create);
// <brls:View xml=@res/xml/fragment/dynamic_tab.xml

#pragma once

#include "presenter/dynamic_tab.hpp"
#include "presenter/dynamic_video.hpp"
#include "view/auto_tab_frame.hpp"

class RecyclingGrid;
class AutoTabFrame;
class DynamicVideo;

typedef brls::Event<int64_t> UserSelectedEvent;

class DynamicTab : public AttachedView, public DynamicTabRequest, public DynamicVideoRequest {
public:
    DynamicTab();

    ~DynamicTab();

    virtual void onUpList(const bilibili::DynamicUpListResultWrapper& result) override;

    virtual void onError(const std::string& error) override;

    static View* create();

    void onCreate() override;

    void changeUser(int64_t mid);

    void onDynamicVideoList(const bilibili::DynamicVideoListResult& result, unsigned int index) override;

private:
    BRLS_BIND(RecyclingGrid, upRecyclingGrid, "dynamic/up/recyclingGrid");
    BRLS_BIND(RecyclingGrid, videoRecyclingGrid, "dynamic/videoList");
};