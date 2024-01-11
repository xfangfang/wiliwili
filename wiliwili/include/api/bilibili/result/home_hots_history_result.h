//
// Created by fang on 2022/7/7.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"
#include "home_result.h"

namespace bilibili {

class HotsHistoryVideoResult {
public:
    int aid;
    std::string bvid;
    int cid;
    std::string pic;
    std::string title;
    int duration;
    int pubdate;
    UserSimpleResult owner;
    VideoSimpleStateResult stat;
    std::string achievement;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsHistoryVideoResult, aid, bvid, cid, pic, title, duration, pubdate, owner, stat,
                                   achievement);

typedef std::vector<HotsHistoryVideoResult> HotsHistoryVideoListResult;

class HotsHistoryVideoListResultWrapper {
public:
    std::string explain;
    HotsHistoryVideoListResult list;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsHistoryVideoListResultWrapper, explain, list);

};  // namespace bilibili