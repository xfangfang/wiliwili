//
// Created by fang on 2022/5/26.
//

#pragma once

#include "nlohmann/json.hpp"

using namespace std;

namespace bilibili {

    class UserSimpleResult {
    public:
        int mid;
        string name;
        string face;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserSimpleResult, mid, name, face);


    class UserSimpleResult2 {
    public:
        string mid;
        string uname;
        string avatar;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserSimpleResult2, mid, uname, avatar);

    class UserSimpleResult3 {
    public:
        uint uid;
        string uname;
        string face;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserSimpleResult3, uid, uname, face);

    class UserResult{
    public:
        int mid = -1;
        int level;
        int following;
        int follower;
        std::string name;
        std::string face;
        std::string sex;
        std::string sign;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserResult, mid, level, following, follower, name, face, sex, sign);


    class SeasonUserResult {
    public:
        uint mid;
        uint follower;
        uint is_follow;
        string uname;
        string avatar;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonUserResult, mid, uname, avatar, follower, is_follow);

}