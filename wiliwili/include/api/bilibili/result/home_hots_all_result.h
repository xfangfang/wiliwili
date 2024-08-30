//
// Created by fang on 2022/7/6.
//

#pragma once

#include "bilibili/util/json.hpp"
#include "user_result.h"
#include "home_result.h"

namespace bilibili {

class HotsAllVideoResult {
public:
    uint64_t aid;
    std::string bvid;
    uint64_t cid;
    std::string pic;
    std::string title;
    int duration;
    int pubdate;
    UserSimpleResult owner;
    VideoSimpleStateResult stat;
    RecommendReasonResult rcmd_reason;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsAllVideoResult, aid, bvid, cid, pic, title, duration, pubdate, owner, stat);

typedef std::vector<HotsAllVideoResult> HotsAllVideoListResult;

class HotsAllVideoListResultWrapper {
public:
    HotsAllVideoListResult list;
    bool no_more;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsAllVideoListResultWrapper, list, no_more);

};  // namespace bilibili