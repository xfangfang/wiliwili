//
// Created by fang on 2022/5/26.
//

#pragma once

#include "nlohmann/json.hpp"

namespace bilibili {

class UserSimpleResult {
public:
    int64_t mid = 0;
    std::string name;
    std::string face;
};
inline void to_json(nlohmann::json& nlohmann_json_j, const UserSimpleResult& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, mid, name, face));
}
inline void from_json(const nlohmann::json& nlohmann_json_j, UserSimpleResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("face")) {
        nlohmann_json_j.at("face").get_to(nlohmann_json_t.face);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, mid, name));
}

class UserSimpleResult2 {
public:
    std::string mid;
    std::string uname;
    std::string avatar;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserSimpleResult2, mid, uname, avatar);

class UserSimpleResult3 {
public:
    int64_t uid = -1;
    std::string uname;
    std::string face;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserSimpleResult3, uid, uname, face);

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

}  // namespace bilibili