//
// Created by fang on 2022/7/7.
//

// register this fragment in main.cpp
//#include "fragment/home_hots_weekly.hpp"
//    brls::Application::registerXMLView("HomeHotsWeekly", HomeHotsWeekly::create);
// <brls:View xml=@res/xml/fragment/home_hots_weekly.xml

#pragma once

#include "presenter/home_hots_weekly.hpp"
#include "view/auto_tab_frame.hpp"

namespace brls {
class Label;
};
class RecyclingGrid;
class RecyclingGridItemVideoCard;

class HomeHotsWeekly : public AttachedView, public HomeHotsWeeklyRequest {
public:
    HomeHotsWeekly();

    ~HomeHotsWeekly() override;

    // 重写点击判断函数，目的是让右上角超出组件显示区域外的按钮也能被检测点击事件
    brls::View* hitTest(brls::Point point) override;

    static View* create();

    void onCreate() override;

    void onHotsWeeklyList(const bilibili::HotsWeeklyListResult& result) override;
    void onHotsWeeklyVideoList(const bilibili::HotsWeeklyVideoListResult& result, const std::string& label,
                               const std::string& reminder) override;
    void onError(const std::string& error) override;

    void switchChannel();

private:
    BRLS_BIND(brls::Label, weekly_reminder, "home/hots/weekly/reminder");
    BRLS_BIND(brls::Label, weekly_label, "home/hots/weekly/label");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "home/hots/weekly/recyclingGrid");
    BRLS_BIND(brls::Box, weekly_box, "home/hots/weekly/box");
    int currentChannel = 0;
};