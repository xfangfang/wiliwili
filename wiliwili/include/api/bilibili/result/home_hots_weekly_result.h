//
// Created by fang on 2022/7/7.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"
#include "home_result.h"

namespace bilibili {

class HotsWeeklyResult {
public:
    int number;
    std::string subject;
    int status;
    std::string name;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsWeeklyResult, number, subject, status, name);

typedef std::vector<HotsWeeklyResult> HotsWeeklyListResult;

class HotsWeeklyResultWrapper {
public:
    HotsWeeklyListResult list;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsWeeklyResultWrapper, list);

class HotsWeeklyVideoResult {
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
    std::string rcmd_reason;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsWeeklyVideoResult, aid, bvid, cid, pic, title, duration, pubdate, owner, stat,
                                   rcmd_reason);

class HotsWeeklyConfig {
public:
    std::string label;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsWeeklyConfig, label);

typedef std::vector<HotsWeeklyVideoResult> HotsWeeklyVideoListResult;

class HotsWeeklyVideoListResultWrapper {
public:
    HotsWeeklyConfig config;
    std::string reminder;
    HotsWeeklyVideoListResult list;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsWeeklyVideoListResultWrapper, config, reminder, list);

};  // namespace bilibili