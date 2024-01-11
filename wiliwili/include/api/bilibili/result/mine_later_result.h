//
// Created by 贾海峰 on 2023/7/6.
//

#pragma once

#include "nlohmann/json.hpp"

namespace bilibili {

class Up {
public:
    std::string name;
    int mid;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Up, name, mid);

class Stat {
public:
    int view, danmaku;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Stat, view, danmaku);

class WatchLaterItem {
public:
    std::string pic;
    Up owner;
    int duration;  // video length in seconds
    std::string title;
    int aid;
    Stat stat;
    std::string bvid;
    int cid;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WatchLaterItem, aid, pic, title, duration, owner, stat, bvid, cid);

typedef std::vector<WatchLaterItem> WatchLaterList;

class WatchLaterListWrapper {
public:
    int count;
    WatchLaterList list;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WatchLaterListWrapper, count, list);
}  // namespace bilibili