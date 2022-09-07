//
// Created by fang on 2022/5/26.
//

#pragma once

#include "nlohmann/json.hpp"

namespace bilibili {

class UserSimpleResult {
public:
    int mid;
    std::string name;
    std::string face;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserSimpleResult, mid, name, face);

class UserSimpleResult2 {
public:
    std::string mid;
    std::string uname;
    std::string avatar;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserSimpleResult2, mid, uname, avatar);

class UserSimpleResult3 {
public:
    unsigned int uid;
    std::string uname;
    std::string face;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserSimpleResult3, uid, uname, face);

class UserResult {
public:
    int mid = -1;
    int level;
    int following;
    int follower;
    float coins;
    std::string name;
    std::string face;
    std::string sex;
    std::string sign;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserResult, mid, level, following, follower,
                                   name, face, sex, sign, coins);

class SeasonUserResult {
public:
    unsigned int mid       = 0;
    unsigned int follower  = 0;
    unsigned int is_follow = 0;
    std::string uname;
    std::string avatar;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonUserResult, mid, uname, avatar,
                                   follower, is_follow);

}  // namespace bilibili