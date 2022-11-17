//
// Created by fang on 2022/7/31.
//

// register this fragment in main.cpp
//#include "fragment/mine_collection_video_list.hpp"
//    brls::Application::registerXMLView("MineCollectionVideoList", MineCollectionVideoList::create);
// <brls:View xml=@res/xml/fragment/mine_collection_video_list.xml

#pragma once

#include <borealis.hpp>
#include "bilibili.h"
#include "view/recycling_grid.hpp"

class MineCollectionVideoList : public brls::Box {
public:
    MineCollectionVideoList(const bilibili::CollectionResult& data);

    ~MineCollectionVideoList();

    static View* create(const bilibili::CollectionResult& data);

    void requestCollectionList();

private:
    bilibili::CollectionResult collectionData;

    BRLS_BIND(brls::Label, labelTitle, "collection/label/title");
    BRLS_BIND(brls::Label, labelSubtitle, "collection/label/subtitle");
    BRLS_BIND(brls::Image, imageCover, "collection/cover");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "collection/recyclingGrid");

    unsigned int requestIndex = 1;
};