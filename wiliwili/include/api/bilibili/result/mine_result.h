//
// Created by fang on 2022/7/26.
//

#pragma once

#include "bilibili/util/json.hpp"
#include "bilibili/result/dynamic_video.h"
#include "bilibili/result/inbox_result.h"

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
    uint64_t mid;
    unsigned int created;
    std::string length;
    uint64_t aid;
    std::string bvid;
    bool is_charging_arc;  // 充电专属视频

    // 合集相关信息
    struct MetaInfo {
        uint64_t id = 0;  // season_id
        std::string title;
        std::string cover;
        std::string intro;
        unsigned int ptime    = 0;  // 合集更新时间
        unsigned int ep_count = 0;  // 合集视频数量

        // 合集统计信息
        struct StatInfo {
            uint64_t season_id = 0;
            int view           = 0;
            int danmaku        = 0;
            int reply          = 0;
            int favorite       = 0;
            int coin           = 0;
            int share          = 0;
            int like           = 0;
        };
        StatInfo stat;
    };
    MetaInfo meta;
    uint64_t season_id = 0;  // 合集 ID，如果不是合集则为 0
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

    // 解析 meta 字段
    if (nlohmann_json_j.contains("meta") && !nlohmann_json_j.at("meta").is_null()) {
        auto& meta = nlohmann_json_j.at("meta");
        if (meta.contains("id")) {
            meta.at("id").get_to(nlohmann_json_t.meta.id);
        }
        if (meta.contains("title")) {
            meta.at("title").get_to(nlohmann_json_t.meta.title);
        }
        if (meta.contains("cover")) {
            meta.at("cover").get_to(nlohmann_json_t.meta.cover);
        }
        if (meta.contains("intro")) {
            meta.at("intro").get_to(nlohmann_json_t.meta.intro);
        }
        if (meta.contains("ptime")) {
            meta.at("ptime").get_to(nlohmann_json_t.meta.ptime);
        }
        if (meta.contains("ep_count")) {
            meta.at("ep_count").get_to(nlohmann_json_t.meta.ep_count);
        }

        // 解析 meta.stat 字段
        if (meta.contains("stat") && !meta.at("stat").is_null()) {
            auto& stat = meta.at("stat");
            if (stat.contains("season_id")) {
                stat.at("season_id").get_to(nlohmann_json_t.meta.stat.season_id);
            }
            if (stat.contains("view")) {
                stat.at("view").get_to(nlohmann_json_t.meta.stat.view);
            }
            if (stat.contains("danmaku")) {
                stat.at("danmaku").get_to(nlohmann_json_t.meta.stat.danmaku);
            }
            if (stat.contains("reply")) {
                stat.at("reply").get_to(nlohmann_json_t.meta.stat.reply);
            }
            if (stat.contains("favorite")) {
                stat.at("favorite").get_to(nlohmann_json_t.meta.stat.favorite);
            }
            if (stat.contains("coin")) {
                stat.at("coin").get_to(nlohmann_json_t.meta.stat.coin);
            }
            if (stat.contains("share")) {
                stat.at("share").get_to(nlohmann_json_t.meta.stat.share);
            }
            if (stat.contains("like")) {
                stat.at("like").get_to(nlohmann_json_t.meta.stat.like);
            }
        }
    }

    // 解析 season_id 字段
    if (nlohmann_json_j.contains("season_id")) {
        nlohmann_json_j.at("season_id").get_to(nlohmann_json_t.season_id);
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
    size_t dyn_num;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserDynamicCount, dyn_num);

class UserRelationStat {
public:
    uint64_t mid, following, black, follower;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserRelationStat, mid, following, black, follower);

}  // namespace bilibili