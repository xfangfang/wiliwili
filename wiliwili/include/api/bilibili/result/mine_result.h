//
// Created by fang on 2022/7/26.
//

#pragma once

#include "nlohmann/json.hpp"
#include "bilibili/result/dynamic_video.h"

namespace bilibili {

class QrLoginTokenResult {
public:
    std::string url;
    std::string oauthKey;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QrLoginTokenResult, url, oauthKey);

class QrLoginTokenResultV2 {
public:
    std::string url;
    std::string qrcode_key;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QrLoginTokenResultV2, url, qrcode_key);

enum class LoginInfo {
    SUCCESS           = 1,
    OAUTH_KEY_ERROR   = -1,
    OAUTH_KEY_TIMEOUT = -2,
    NEED_SCAN         = -4,
    NEED_CONFIRM      = -5,
    NONE              = -10
};

class QrLoginInfoResult {
public:
    bool status;
    LoginInfo data;
    std::string message;
};

inline void from_json(const nlohmann::json& nlohmann_json_j, QrLoginInfoResult& nlohmann_json_t) {
    if (nlohmann_json_j.at("data").is_number()) {
        nlohmann_json_t.data = LoginInfo(nlohmann_json_j.at("data"));
    } else {
        nlohmann_json_t.data = LoginInfo::SUCCESS;
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, status));
}

class QrLoginInfoResultV2 {
public:
    bool status = false;
    LoginInfo data;
    std::string refresh_token;
};

inline void from_json(const nlohmann::json& nlohmann_json_j, QrLoginInfoResultV2& nlohmann_json_t) {
    switch (nlohmann_json_j.at("code").get<int>()) {
        case 0:
            nlohmann_json_t.status = true;
            nlohmann_json_t.data   = LoginInfo::SUCCESS;
            break;
        case 86101:
            nlohmann_json_t.data = LoginInfo::NEED_SCAN;
            break;
        case 86090:
            nlohmann_json_t.data = LoginInfo::NEED_CONFIRM;
            break;
        case 86038:
            nlohmann_json_t.data = LoginInfo::OAUTH_KEY_TIMEOUT;
            break;
        default:
            nlohmann_json_t.data = LoginInfo::OAUTH_KEY_ERROR;
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, refresh_token));
}

class UserUploadedVideoResult {
public:
    int comment;
    int play;
    std::string pic;
    std::string description;
    std::string copyright;
    std::string title;
    unsigned int video_review;
    std::string author;
    unsigned int mid;
    unsigned int created;
    std::string length;
    unsigned int aid;
    std::string bvid;
    bool is_charging_arc; // 充电专属视频
};
inline void from_json(const nlohmann::json& nlohmann_json_j, UserUploadedVideoResult& nlohmann_json_t) {
    if (nlohmann_json_j.at("play").is_number()) {
        nlohmann_json_j.at("play").get_to(nlohmann_json_t.play);
    } else {
        nlohmann_json_t.play = -1;
    }
    if (nlohmann_json_j.contains("is_charging_arc")) {
        nlohmann_json_j.at("is_charging_arc").get_to(nlohmann_json_t.is_charging_arc);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, comment, pic, description, copyright, title,
                                             video_review, author, mid, created, length, aid, bvid));
}

typedef std::vector<UserUploadedVideoResult> UserUploadedVideoListResult;

class UserUploadedVideoPageResult {
public:
    unsigned int pn;
    unsigned int ps;
    unsigned int count;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserUploadedVideoPageResult, pn, ps, count);

class UserUploadedVideoResultWrapper {
public:
    UserUploadedVideoPageResult page;
    UserUploadedVideoListResult list;
};

inline void from_json(const nlohmann::json& nlohmann_json_j, UserUploadedVideoResultWrapper& nlohmann_json_t) {
    nlohmann_json_j.at("list").at("vlist").get_to(nlohmann_json_t.list);
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, page));
}

class UserDynamicVideoResultWrapper {
public:
    UserUploadedVideoPageResult page;
    DynamicVideoListResult archives;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserDynamicVideoResultWrapper, page, archives);

class UserDynamicCount {
public:
    std::map<std::string, unsigned int> data;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, UserDynamicCount& nlohmann_json_t) {
    if (!nlohmann_json_j.contains("items") || !nlohmann_json_j.at("items").is_array()) return;

    for (auto i : nlohmann_json_j.at("items")) {
        if (i.contains("uid") && i.contains("num"))
            nlohmann_json_t.data[std::to_string(i.at("uid").get<unsigned int>())] = i.at("num").get<unsigned int>();
    }
}

class UserRelationStat {
public:
    unsigned int mid, following, black, follower;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserRelationStat, mid, following, black, follower);

}  // namespace bilibili