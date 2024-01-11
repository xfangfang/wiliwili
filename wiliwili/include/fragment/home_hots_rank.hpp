//
// Created by fang on 2022/7/7.
//

// register this fragment in main.cpp
//#include "fragment/home_hots_rank.hpp"
//    brls::Application::registerXMLView("HomeHotsRank", HomeHotsRank::create);
// <brls:View xml=@res/xml/fragment/home_hots_rank.xml

#pragma once

#include "presenter/home_hots_rank.hpp"
#include "view/auto_tab_frame.hpp"

namespace brls {
class Label;
}
class RecyclingGrid;
class SVGImage;

class HomeHotsRank : public AttachedView, public HomeHotsRankRequest {
public:
    HomeHotsRank();

    void onCreate() override;

    void onHotsRankList(const bilibili::HotsRankVideoListResult& result, const std::string& note) override;

    void onHotsRankPGCList(const bilibili::HotsRankPGCVideoListResult& result, const std::string& note) override;

    void onError(const std::string& error) override;

    void switchChannel();

    // 重写点击判断函数，目的是让右上角超出组件显示区域外的按钮也能被检测点击事件
    brls::View* hitTest(brls::Point point) override;

    ~HomeHotsRank() override;

    static View* create() { return new HomeHotsRank(); }

private:
    BRLS_BIND(brls::Label, rank_note, "home/hots/rank/note");
    BRLS_BIND(SVGImage, rank_note_icon, "home/hots/rank/note/icon");
    BRLS_BIND(brls::Label, rank_label, "home/hots/rank/label");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "home/hots/rank/recyclingGrid");
    BRLS_BIND(brls::Box, rank_box, "home/hots/rank/box");
    int currentChannel = 0;
};