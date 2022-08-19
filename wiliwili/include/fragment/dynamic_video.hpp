//
// Created by fang on 2022/8/18.
//

// register this fragment in main.cpp
//#include "fragment/dynamic_video.hpp"
//    brls::Application::registerXMLView("DynamicVideo", DynamicVideo::create);
// <brls:View xml=@res/xml/fragment/dynamic_video.xml

#pragma once

#include <borealis.hpp>
#include "presenter/dynamic_video.hpp"
#include "view/auto_tab_frame.hpp"

class RecyclingGrid;

class DynamicVideo : public AttachedView, public DynamicVideoRequest {

public:
    DynamicVideo();

    ~DynamicVideo();

    static View *create();

    void onCreate() override;

    void changeUser(uint mid);

    void onDynamicVideoList(const bilibili::DynamicVideoListResult &result, uint index) override;

    void onError(const std::string& error) override;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "dynamic/video/recyclingGrid");

};