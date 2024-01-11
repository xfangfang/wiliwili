//
// Created by fang on 2022/7/30.
//

// register this fragment in main.cpp
//#include "fragment/mine_collection.hpp"
//    brls::Application::registerXMLView("MineCollection", MineCollection::create);
// <brls:View xml=@res/xml/fragment/mine_collection.xml

#pragma once

#include "view/auto_tab_frame.hpp"
#include "presenter/mine_collection.hpp"
#include "view/recycling_grid.hpp"

class MineCollection : public AttachedView, public MineCollectionRequest {
public:
    MineCollection();

    ~MineCollection() override;

    static View *create();

    void onCreate() override;

    void onCollectionList(const bilibili::CollectionListResultWrapper &result) override;

    void onError(const std::string &error) override;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "mine/collection/recyclingGrid");
};