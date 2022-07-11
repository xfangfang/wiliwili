//
// Created by fang on 2022/7/6.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"
#include "home_result.h"

using namespace std;

namespace bilibili {

    class HotsAllVideoResult {
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
        RecommendReasonResult rcmd_reason;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsAllVideoResult, aid, bvid, cid, pic, title, duration, pubdate, owner, stat);

    typedef vector<HotsAllVideoResult> HotsAllVideoListResult;

    class HotsAllVideoListResultWrapper {
    public:
        HotsAllVideoListResult list;
        bool no_more;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsAllVideoListResultWrapper, list, no_more);

};