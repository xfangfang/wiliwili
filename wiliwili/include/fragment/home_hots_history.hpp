//
// Created by fang on 2022/7/7.
//

// register this fragment in main.cpp
//#include "fragment/home_hots_history.hpp"
//    brls::Application::registerXMLView("HomeHotsHistory", HomeHotsHistory::create);
// <brls:View xml=@res/xml/fragment/home_hots_history.xml

#pragma once

#include "presenter/home_hots_history.hpp"
#include "view/auto_tab_frame.hpp"

namespace brls {
class Label;
}
class RecyclingGrid;

class HomeHotsHistory : public AttachedView, public HomeHotsHistoryRequest {
public:
    HomeHotsHistory();

    void onHotsHistoryList(const bilibili::HotsHistoryVideoListResult& result, const std::string& explain) override;

    ~HomeHotsHistory();

    static View* create();

    void onCreate() override;

    void onError(const std::string& error) override;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "home/hots/history/recyclingGrid");
    BRLS_BIND(brls::Label, labelExplain, "home/hots/history/explain");
};