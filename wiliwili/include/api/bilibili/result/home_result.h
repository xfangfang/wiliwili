//
// Created by fang on 2022/5/26.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"

namespace bilibili {

class VideoSimpleStateResult {
public:
    int view;
    int danmaku;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoSimpleStateResult, view, danmaku);

class RecommendReasonResult {
public:
    std::string content = "";
    int reason_type;
    // 3: 点赞多
    // 1: 关注
    // 0: 无 ？
};
inline void from_json(const nlohmann::json& nlohmann_json_j,
                      RecommendReasonResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("reason_type")) {
        nlohmann_json_j.at("reason_type").get_to(nlohmann_json_t.reason_type);
    }
    if (nlohmann_json_t.reason_type == 1) {
        nlohmann_json_t.content = "已关注";
    } else if (nlohmann_json_j.contains("content")) {
        nlohmann_json_j.at("content").get_to(nlohmann_json_t.content);
    }
}

class RecommendVideoResult {
public:
    int id;
    std::string bvid;
    int cid;
    std::string pic   = "";
    std::string title = "";
    int duration;
    int pubdate;
    UserSimpleResult owner;
    VideoSimpleStateResult stat;
    int is_followed;
    RecommendReasonResult rcmd_reason;
};
inline void from_json(const nlohmann::json& nlohmann_json_j,
                      RecommendVideoResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("rcmd_reason") &&
        !nlohmann_json_j.at("rcmd_reason").is_null()) {
        nlohmann_json_j.at("rcmd_reason").get_to(nlohmann_json_t.rcmd_reason);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, id, bvid, cid,
                                             pic, title, duration, pubdate,
                                             owner, stat, is_followed));
}

typedef std::vector<RecommendVideoResult> RecommendVideoListResult;

class RecommendVideoListResultWrapper {
public:
    RecommendVideoListResult item;
    int requestIndex = 0;
};
inline void from_json(const nlohmann::json& nlohmann_json_j,
                      RecommendVideoListResultWrapper& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, item));
}

};  // namespace bilibili