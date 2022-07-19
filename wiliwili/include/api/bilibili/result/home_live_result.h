//
// Created by fang on 2022/7/13.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"

using namespace std;

namespace bilibili {

    class LiveAreaResult {
    public:
        int id;
        string title;
        int area_v2_id;
        int area_v2_parent_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveAreaResult, id, title, area_v2_id, area_v2_parent_id);

    class LiveVideoResult {
    public:
        int roomid; // small_card_v1 roomid 标记为id
        int uid;
        string title;
        string uname;
        int online;
        string play_url;
        string cover;
        string area_name;
    };

    inline void from_json(const nlohmann::json& nlohmann_json_j, LiveVideoResult& nlohmann_json_t) {
        if(nlohmann_json_j.contains("roomid")){
            nlohmann_json_j.at("roomid").get_to(nlohmann_json_t.roomid);
        } else if(nlohmann_json_j.contains("id")){
            nlohmann_json_j.at("id").get_to(nlohmann_json_t.roomid);
        } else {
            nlohmann_json_t.roomid = -1;
        }
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, uid, title, uname, online, play_url, cover, area_name));
    }

    typedef vector<LiveVideoResult> LiveVideoListResult;
    typedef vector<LiveAreaResult> LiveAreaListResult;

    class LiveResultWrapper {
    public:
        LiveVideoListResult card_list;
        LiveAreaListResult live_list;
        int has_more;
    };

    inline void from_json(const nlohmann::json& nlohmann_json_j, LiveResultWrapper& nlohmann_json_t) {
        for(auto i : nlohmann_json_j.at("card_list")){
            string card_type = i.at("card_type");
            if(card_type.compare("area_entrance_v1") == 0){
                i.at("card_data").at("area_entrance_v1").at("list").get_to(nlohmann_json_t.live_list);
            }else if(card_type.compare("second_card_v1") == 0){
                nlohmann_json_t.card_list.push_back(i.at("card_data").at("second_card_v1").get<LiveVideoResult>());
            }
            else if(card_type.compare("small_card_v1") == 0){
                nlohmann_json_t.card_list.push_back(i.at("card_data").at("small_card_v1").get<LiveVideoResult>());
            }
        }
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_more));
    }
}