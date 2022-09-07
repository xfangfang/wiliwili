//
// Created by fang on 2022/7/31.
//

#pragma once

#include "nlohmann/json.hpp"
#include "bilibili/result/user_result.h"

namespace bilibili {

class SeasonEpisodeResult {
public:
    unsigned int aid;
    unsigned int cid;
    unsigned int id;  //ep id
    std::string bvid;
    unsigned int duration;
    std::string title;
    std::string long_title;
    std::string subtitle;
    std::string link;
    unsigned int pub_time;

    int progress = -1;  //用于从历史记录加载进播放页
};
inline void from_json(const nlohmann::json& nlohmann_json_j,
                      SeasonEpisodeResult& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, aid, cid, bvid,
                                             duration, title, long_title,
                                             subtitle, pub_time, link, id));
}

class SeasonRatingResult {
public:
    float score        = -1;
    unsigned int count = 0;
};
inline void from_json(const nlohmann::json& nlohmann_json_j,
                      SeasonRatingResult& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, score, count));
}

typedef std::vector<SeasonEpisodeResult> SeasonEpisodeListResult;

class SeasonDetailStat {
public:
    unsigned int views     = 0;
    unsigned int danmakus  = 0;
    unsigned int favorite  = 0;
    unsigned int favorites = 0;
    unsigned int coins     = 0;
    unsigned int share     = 0;
    unsigned int likes     = 0;
    unsigned int reply     = 0;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonDetailStat, views, danmakus, favorite,
                                   favorites, coins, share, likes, reply);

class SeasonPublishStat {
public:
    int is_finish, is_started;
    std::string pub_time_show;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonPublishStat, is_finish, is_started,
                                   pub_time_show);

class SeasonResultWrapper {
public:
    unsigned int season_id;
    std::string season_title;
    std::string season_desc;
    SeasonEpisodeListResult episodes;
    SeasonRatingResult rating;
    std::string evaluate;
    SeasonUserResult up_info;
    SeasonDetailStat stat;
    SeasonPublishStat publish;
};
inline void from_json(const nlohmann::json& nlohmann_json_j,
                      SeasonResultWrapper& nlohmann_json_t) {
    if (nlohmann_json_j.contains("rating")) {
        nlohmann_json_j.at("rating").get_to(nlohmann_json_t.rating);
    }
    if (nlohmann_json_j.contains("up_info")) {
        nlohmann_json_j.at("up_info").get_to(nlohmann_json_t.up_info);
    }
    if (nlohmann_json_j.contains("stat")) {
        nlohmann_json_j.at("stat").get_to(nlohmann_json_t.stat);
    }
    if (nlohmann_json_j.contains("publish")) {
        nlohmann_json_j.at("publish").get_to(nlohmann_json_t.publish);
    }
    if (nlohmann_json_j.contains("new_ep") &&
        nlohmann_json_j.at("new_ep").contains("desc")) {
        nlohmann_json_j.at("new_ep").at("desc").get_to(
            nlohmann_json_t.season_desc);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, season_id,
                                             season_title, episodes, evaluate));
}

};  // namespace bilibili