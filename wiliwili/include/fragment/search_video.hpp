//
// Created by fang on 2022/8/2.
//

// register this fragment in main.cpp
//#include "fragment/search_video.hpp"
//    brls::Application::registerXMLView("SearchVideo", SearchVideo::create);
// <brls:View xml=@res/xml/fragment/search_video.xml

#pragma once

#include <atomic>
#include <memory>

#include "view/auto_tab_frame.hpp"

class RecyclingGrid;

class SearchVideo : public AttachedView {
public:
    SearchVideo();

    ~SearchVideo();

    static View* create();

    void requestSearch(const std::string& key);

    void _requestSearch(const std::string& key);

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "search/video/recyclingGrid");

    unsigned int requestIndex = 1;
};