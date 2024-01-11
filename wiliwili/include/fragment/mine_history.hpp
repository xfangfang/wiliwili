//
// Created by fang on 2022/7/28.
//

// register this fragment in main.cpp
//#include "fragment/mine_history.hpp"
//    brls::Application::registerXMLView("MineHistory", MineHistory::create);
// <brls:View xml=@res/xml/fragment/mine_history.xml

#pragma once

#include "view/auto_tab_frame.hpp"
#include "view/recycling_grid.hpp"
#include "presenter/mine_history.hpp"

class MineHistory : public AttachedView, public MineHistoryRequest {
public:
    MineHistory();

    ~MineHistory();

    static View *create();

    void onCreate() override;

    void onHistoryList(const bilibili::HistoryVideoResultWrapper &result) override;

    void onError(const std::string &error) override;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "mine/history/recyclingGrid");
};