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

#include "fragment/search_interface.hpp"

class RecyclingGrid;

class SearchVideo : public SearchAttachedView {
public:
    SearchVideo();

    ~SearchVideo() override;

    static brls::View* create();

    void requestSearch(const std::string& key) override;

    void _requestSearch(const std::string& key);

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "search/video/recyclingGrid");

    std::string requestOrder;
    unsigned int requestIndex = 1;
};