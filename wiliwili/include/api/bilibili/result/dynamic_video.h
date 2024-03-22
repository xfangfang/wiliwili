//
// Created by fang on 2022/8/18.
//

#pragma once

#include "bilibili/util/json.hpp"
#include "user_result.h"
#include "home_result.h"
#include "dynamic_article.h"

namespace bilibili {

class DynamicVideoResult {
public:
    int aid = 0;
    std::string bvid;
    std::string pic;
    std::string title;
    int duration = 0;
    int pubdate  = 0;
    UserSimpleResult owner;
    VideoSimpleStateResult stat;
};
inline void to_json(nlohmann::json& nlohmann_json_j, const DynamicVideoResult& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, aid, bvid, pic, title, duration, pubdate, stat));
}
inline void from_json(const nlohmann::json& nlohmann_json_j, DynamicVideoResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("owner")) {
        nlohmann_json_j.at("owner").get_to(nlohmann_json_t.owner);
    } else if (nlohmann_json_j.contains("author")) {
        nlohmann_json_j.at("author").get_to(nlohmann_json_t.owner);
    }
    if (nlohmann_json_j.at("duration").is_number()) {
        nlohmann_json_j.at("duration").get_to(nlohmann_json_t.duration);
    }
    if (nlohmann_json_j.at("pubdate").is_number()) {
        nlohmann_json_j.at("pubdate").get_to(nlohmann_json_t.pubdate);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, aid, bvid, pic, title, stat));
}

typedef std::vector<DynamicVideoResult> DynamicVideoListResult;
typedef std::vector<DynamicArticleResult> DynamicArticleListResult;

template <typename Item>
class DynamicListResultWrapper {
public:
    Item items;
    bool has_more{};
    std::string offset;
    std::string update_baseline;
    unsigned int update_num{};
    unsigned int page{};
};
template <typename Item>
inline void from_json(const nlohmann::json& nlohmann_json_j, DynamicListResultWrapper<Item>& nlohmann_json_t) {
    if (nlohmann_json_j.contains("items") && nlohmann_json_j.at("items").is_array()) {
        nlohmann_json_j.at("items").get_to(nlohmann_json_t.items);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_more, offset, update_baseline, update_num));
}

class DynamicUpResult {
public:
    bool has_update;
    std::string face;
    std::string uname;
    int64_t mid;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DynamicUpResult, has_update, face, uname, mid);

typedef std::vector<DynamicUpResult> DynamicUpListResult;

class DynamicUpListResultWrapper {
public:
    DynamicUpListResult items;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, DynamicUpListResultWrapper& nlohmann_json_t) {
    if (nlohmann_json_j.contains("items") && nlohmann_json_j.at("items").is_array()) {
        nlohmann_json_j.at("items").get_to(nlohmann_json_t.items);
    }
}
};  // namespace bilibili