//
// Created by fang on 2023/1/18.
//

// register this fragment in main.cpp
//#include "fragment/mine_bangumi.hpp"
//    brls::Application::registerXMLView("MineBangumi", MineBangumi::create);
// <brls:View xml=@res/xml/fragment/mine_bangumi.xml/>

#pragma once

#include "presenter/mine_bangumi.hpp"
#include "view/auto_tab_frame.hpp"

class RecyclingGrid;

class MineBangumi : public AttachedView, public MineBangumiRequest {
public:
    MineBangumi();

    ~MineBangumi() override;

    void onCreate() override;

    void onError(const std::string &error) override;

    void onBangumiList(const bilibili::BangumiCollectionWrapper &result) override;

    static View *create();

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "mine/bangumi/recyclingGrid");
};