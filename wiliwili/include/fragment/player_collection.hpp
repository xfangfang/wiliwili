//
// Created by fang on 2022/12/28.
//

// register this fragment in main.cpp
//#include "fragment/player_collection.hpp"
//    brls::Application::registerXMLView("PlayerCollection", PlayerCollection::create);
// <brls:View xml=@res/xml/fragment/player_collection.xml

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

class RecyclingGrid;

enum class VideoType {
    Plain      = 2,   // 普通视频
    Audio      = 12,  // 音频
    Collection = 21,  // 视频合集
    Episode    = 24,  // 番剧
};

namespace bilibili {
class SimpleCollectionListResultWrapper;
};
class RecyclingGrid;

class PlayerCollection : public brls::Box {
public:
    PlayerCollection(int rid, int type);

    ~PlayerCollection() override;

    std::string getAddCollectionList();

    std::string getDeleteCollectionList();

    bool isFavorite();

    void onCollectionList(const bilibili::SimpleCollectionListResultWrapper& result);

    /// 获取收藏列表
    void getCollectionList(int rid, int type);

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "player/collection/recyclingGrid");
};