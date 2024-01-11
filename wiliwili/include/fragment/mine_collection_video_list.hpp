//
// Created by fang on 2022/7/31.
//

// register this fragment in main.cpp
//#include "fragment/mine_collection_video_list.hpp"
//    brls::Application::registerXMLView("MineCollectionVideoList", MineCollectionVideoList::create);
// <brls:View xml=@res/xml/fragment/mine_collection_video_list.xml

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

#include "bilibili.h"
#include "view/recycling_grid.hpp"
#include "bilibili/result/mine_collection_result.h"

class TextBox;

class MineCollectionVideoList : public brls::Box {
public:
    MineCollectionVideoList();

    MineCollectionVideoList(const bilibili::CollectionResult& data, int type);

    ~MineCollectionVideoList() override;

    static View* create();

    void requestCollectionList();

private:
    bilibili::CollectionResult collectionData;

    BRLS_BIND(TextBox, labelTitle, "collection/label/title");
    BRLS_BIND(brls::Label, labelSubtitle, "collection/label/subtitle");
    BRLS_BIND(brls::Image, imageCover, "collection/cover");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "collection/recyclingGrid");
    BRLS_BIND(brls::AppletFrame, appletFrame, "collection/appletFrame");

    unsigned int requestIndex = 1;
    int requestType           = 1;
    bool hasMore              = true;
};