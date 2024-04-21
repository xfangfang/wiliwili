//
// Created by fang on 2022/5/26.
//

#pragma once

#include "bilibili/util/json.hpp"

namespace bilibili {

class UserSimpleResult {
public:
    int64_t mid = 0;
    std::string name;
    std::string face;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserSimpleResult, mid, name, face);

class UserResult {
public:
    int64_t mid   = -1;
    int level     = 0;
    int following = 0;
    int follower  = 0;
    float coins   = 0;
    std::string name;
    std::string face;
    std::string sex;
    std::string sign;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserResult, mid, level, following, follower, name, face, sex, sign, coins);

class SeasonUserResult {
public:
    int64_t mid            = 0;
    unsigned int follower  = 0;
    unsigned int is_follow = 0;
    std::string uname;
    std::string avatar;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonUserResult, mid, uname, avatar, follower, is_follow);

class UserCommentVip {
public:
    std::string nickname_color;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserCommentVip, nickname_color);

class UserDynamicResult {
public:
    int64_t mid = 0;
    std::string name;
    std::string face;
    UserCommentVip vip;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserDynamicResult, mid, name, face, vip);

class LevelInfo {
public:
    int current_level{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LevelInfo, current_level);

class UserCommentResult {
public:
    std::string mid, uname, avatar;
    UserCommentVip vip;
    int is_senior_member{};
    LevelInfo level_info{};
    bool is_uploader{};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserCommentResult, mid, uname, avatar, is_senior_member, level_info, vip);

}  // namespace bilibili