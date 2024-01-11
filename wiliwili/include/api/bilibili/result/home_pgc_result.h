//
// Created by fang on 2022/7/14.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"
#include "home_result.h"

namespace bilibili {

class PGCItemResult {
public:
    std::string title;
    std::string desc;
    std::string cover;
    int season_id;
    std::string bottom_left_badge;   // pic link 不是所有都包含此项
    std::string bottom_right_badge;  // text
    std::string badge_info;          // pic link
};
inline void from_json(const nlohmann::json& nlohmann_json_j, PGCItemResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("bottom_right_badge")) {
        nlohmann_json_j.at("bottom_right_badge").at("text").get_to(nlohmann_json_t.bottom_right_badge);
    } else if (nlohmann_json_j.contains("new_ep") && nlohmann_json_j.at("new_ep").contains("index_show")) {
        nlohmann_json_j.at("new_ep").at("index_show").get_to(nlohmann_json_t.bottom_right_badge);
    }

    if (nlohmann_json_j.contains("badge_info") && nlohmann_json_j.at("badge_info").contains("img")) {
        nlohmann_json_j.at("badge_info").at("img").get_to(nlohmann_json_t.badge_info);
    }

    if (nlohmann_json_j.contains("bottom_left_badge")) {
        nlohmann_json_j.at("bottom_left_badge").at("img").get_to(nlohmann_json_t.bottom_left_badge);
    }

    if (nlohmann_json_j.contains("progress")) {
        nlohmann_json_j.at("progress").get_to(nlohmann_json_t.desc);
    } else if (nlohmann_json_j.contains("desc")) {
        nlohmann_json_j.at("desc").get_to(nlohmann_json_t.desc);
    }

    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, title, cover, season_id));
}

typedef std::vector<PGCItemResult> PGCItemListResult;

class PGCModuleResult {
public:
    std::string title;
    int size;
    int module_id;
    std::string style;
    std::string header;
    std::string url;
    PGCItemListResult items;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, PGCModuleResult& nlohmann_json_t) {
    if (nlohmann_json_j.at("headers").size() > 0) {
        nlohmann_json_j.at("headers").at(0).at("title").get_to(nlohmann_json_t.header);
        nlohmann_json_j.at("headers").at(0).at("url").get_to(nlohmann_json_t.url);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, title, size, style, items, module_id));
}

typedef std::vector<PGCModuleResult> PGCModuleListResult;

class PGCResultWrapper {
public:
    int has_next;
    std::string next_cursor;
    PGCModuleListResult modules;
};

inline void from_json(const nlohmann::json& nlohmann_json_j, PGCResultWrapper& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_next, next_cursor, modules));
}

class PGCIndexResult {
public:
    std::string title;
    std::string cover;
    int season_type;
    int season_id;
    int is_finish;
    int media_id;
    std::string order;       // eg: 800万追番
    std::string index_show;  // eg: 全12话
    std::string badge_info;  // pic link
};
inline void from_json(const nlohmann::json& nlohmann_json_j, PGCIndexResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("badge_info")) {
        nlohmann_json_j.at("badge_info").at("img").get_to(nlohmann_json_t.badge_info);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, title, season_type, season_id, index_show, is_finish,
                                             media_id, cover, order));
}

typedef std::vector<PGCIndexResult> PGCIndexListResult;

class PGCIndexResultWrapper {
public:
    PGCIndexListResult list;
    int has_next;
    int num;
    int size;
    int total;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, PGCIndexResultWrapper& nlohmann_json_t) {
    if (nlohmann_json_j.contains("list")) {
        nlohmann_json_j.at("list").get_to(nlohmann_json_t.list);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_next, num, size, total));
}

class PGCIndexFilterValue {
public:
    std::string keyword;  // -1
    std::string name;     // 全部
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PGCIndexFilterValue, keyword, name);

typedef std::vector<PGCIndexFilterValue> PGCIndexFilterValueList;

class PGCIndexFilter {
public:
    std::string field;  // eg: area
                        //        std::string name; // eg: 地区
    PGCIndexFilterValueList values;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PGCIndexFilter, field, values);

typedef std::vector<PGCIndexFilter> PGCIndexFilterList;

class PGCIndexFilterWrapper {
public:
    PGCIndexFilterList filter;
    int index_type;
    std::string index_name;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, PGCIndexFilterWrapper& nlohmann_json_t) {
    if (nlohmann_json_j.contains("order")) {
        PGCIndexFilter order;
        order.field = "order";

        for (auto& i : nlohmann_json_j.at("order")) {
            PGCIndexFilterValue value;
            i.at("field").get_to(value.keyword);
            i.at("name").get_to(value.name);
            order.values.emplace_back(value);
        }
        nlohmann_json_t.filter.emplace_back(order);
    }

    if (nlohmann_json_j.contains("filter")) {
        auto filters = nlohmann_json_j.at("filter").get<PGCIndexFilterList>();
        nlohmann_json_t.filter.insert(nlohmann_json_t.filter.end(), filters.begin(), filters.end());
    }
}

typedef std::map<std::string, PGCIndexFilterWrapper> PGCIndexFilters;  // index_type -> PGCIndexFilterWrapper
};                                                                     // namespace bilibili