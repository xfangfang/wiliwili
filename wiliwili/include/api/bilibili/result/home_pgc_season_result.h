//
// Created by fang on 2022/7/31.
//

#pragma once

#include "nlohmann/json.hpp"
#include "bilibili/result/user_result.h"

using namespace std;

namespace bilibili {

    class SeasonEpisodeResult {
    public:
        uint aid;
        uint cid;
        std::string bvid;
        uint duration;
        std::string title;
        std::string long_title;
        std::string subtitle;
        uint pub_time;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonEpisodeResult& nlohmann_json_t) {
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, aid, cid, bvid, duration, title,
                                                 long_title, subtitle, pub_time));
    }

    class SeasonRatingResult {
    public:
        float score;
        uint count;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonRatingResult& nlohmann_json_t) {
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, score, count));
    }


    typedef vector<SeasonEpisodeResult> SeasonEpisodeListResult;

    class SeasonResultWrapper {
    public:
        uint season_id;
        std::string season_title;
        SeasonEpisodeListResult episodes;
        SeasonRatingResult rating;
        std::string evaluate;
        SeasonUserResult up_info;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonResultWrapper& nlohmann_json_t) {
        if(nlohmann_json_j.contains("rating")){
            nlohmann_json_j.at("rating").get_to(nlohmann_json_t.rating);
        }
        if(nlohmann_json_j.contains("up_info")){
            nlohmann_json_j.at("up_info").get_to(nlohmann_json_t.up_info);
        }
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, season_id, season_title, episodes, evaluate));
    }

};