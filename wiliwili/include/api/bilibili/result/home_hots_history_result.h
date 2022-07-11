//
// Created by fang on 2022/7/7.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"
#include "home_result.h"

using namespace std;

namespace bilibili {

    class HotsHistoryVideoResult {
    public:
        int aid;
        string bvid;
        int cid;
        string pic;
        string title;
        int duration;
        int pubdate;
        UserSimpleResult owner;
        VideoSimpleStateResult stat;
        string achievement;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsHistoryVideoResult, aid, bvid, cid, pic, title, duration, pubdate, owner, stat, achievement);

    typedef vector<HotsHistoryVideoResult> HotsHistoryVideoListResult;

    class HotsHistoryVideoListResultWrapper {
    public:
        string explain;
        HotsHistoryVideoListResult list;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsHistoryVideoListResultWrapper, explain, list);

};