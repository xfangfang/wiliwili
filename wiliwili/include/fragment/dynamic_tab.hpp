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

    ~DynamicTab() override;

    void onUpList(const bilibili::DynamicUpListResultWrapper& result) override;

    void onError(const std::string& error) override;

    void onVideoError(const std::string& error) override;

    void onArticleError(const std::string& error) override;

    static View* create();

    void onCreate() override;

    void changeUser(int64_t mid);

    void onDynamicVideoList(const bilibili::DynamicVideoListResult& result, unsigned int index) override;

    void onDynamicArticleList(const bilibili::DynamicArticleListResult& result, unsigned int index) override;

private:
    BRLS_BIND(AutoTabFrame, tabFrame, "dynamic/tab/frame");
    BRLS_BIND(RecyclingGrid, upRecyclingGrid, "dynamic/up/recyclingGrid");
    BRLS_BIND(RecyclingGrid, videoRecyclingGrid, "dynamic/videoList");
    BRLS_BIND(RecyclingGrid, articleRecyclingGrid, "dynamic/articleList");

    // 动态页会切换用户或者刷新时同时加载视频和图文
    // 当某视频页或图文页处于隐藏时，autoTabFrame 的实现会导致无法获取该页宽度，导致列表无法正常渲染
    // 这里临时将数据储存在下面变量中，在对应页面展示时再加载数据
    bilibili::DynamicVideoListResult tempVideoList;
    bilibili::DynamicArticleListResult tempArticleList;
};