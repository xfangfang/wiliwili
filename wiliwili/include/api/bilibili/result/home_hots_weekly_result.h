//
// Created by fang on 2022/7/7.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"
#include "home_result.h"

using namespace std;

namespace bilibili {

    class HotsWeeklyResult {
    public:
        int number;
        string subject;
        int status;
        string name;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsWeeklyResult, number, subject, status, name);

    typedef vector<HotsWeeklyResult> HotsWeeklyListResult;

    class HotsWeeklyResultWrapper {
    public:
        HotsWeeklyListResult list;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsWeeklyResultWrapper, list);

    class HotsWeeklyVideoResult {
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
        string rcmd_reason;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsWeeklyVideoResult, aid, bvid, cid, pic, title, duration, pubdate, owner, stat, rcmd_reason);

    class HotsWeeklyConfig {
    public:
        string label;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsWeeklyConfig, label);

    typedef vector<HotsWeeklyVideoResult> HotsWeeklyVideoListResult;

    class HotsWeeklyVideoListResultWrapper {
    public:
        HotsWeeklyConfig config;
        string reminder;
        HotsWeeklyVideoListResult list;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsWeeklyVideoListResultWrapper, config, reminder, list);

};