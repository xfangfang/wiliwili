//
// Created by fang on 2022/7/14.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"
#include "home_result.h"

using namespace std;

namespace bilibili {

    class PGCItemResult{
    public:
        std::string title;
        std::string desc;
        std::string cover;
        int season_id;
        std::string bottom_left_badge; // pic link 不是所有都包含此项
        std::string bottom_right_badge; // text
        std::string badge_info; // pic link
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, PGCItemResult& nlohmann_json_t) {
        if(nlohmann_json_j.contains("bottom_right_badge")){
            nlohmann_json_j.at("bottom_right_badge").at("text").get_to(nlohmann_json_t.bottom_right_badge);
        }else if(nlohmann_json_j.contains("new_ep")){
            nlohmann_json_j.at("new_ep").at("index_show").get_to(nlohmann_json_t.bottom_right_badge);
        }

        if(nlohmann_json_j.contains("badge_info")){
            nlohmann_json_j.at("badge_info").at("img").get_to(nlohmann_json_t.badge_info);
        }

        if(nlohmann_json_j.contains("bottom_left_badge")){
            nlohmann_json_j.at("bottom_left_badge").at("img").get_to(nlohmann_json_t.bottom_left_badge);
        }

        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, title, desc, cover, season_id));
    }

    typedef vector<PGCItemResult> PGCItemListResult;

    class PGCModuleResult{
    public:
        std::string title;
        int size;
        std::string style;
        std::string header;
        PGCItemListResult items;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, PGCModuleResult& nlohmann_json_t) {
        if(nlohmann_json_j.at("headers").size() > 0){
            nlohmann_json_j.at("headers").at(0).at("title").get_to(nlohmann_json_t.header);
        }
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, title, size, style, items));
    }

    typedef vector<PGCModuleResult> PGCModuleListResult;

    class PGCResultWrapper {
    public:
        int has_next;
        std::string next_cursor;
        PGCModuleListResult modules;
    };

    inline void from_json(const nlohmann::json& nlohmann_json_j, PGCResultWrapper& nlohmann_json_t) {
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_next, next_cursor, modules));
    }

};