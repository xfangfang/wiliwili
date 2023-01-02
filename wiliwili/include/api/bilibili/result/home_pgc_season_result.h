//
// Created by fang on 2022/7/31.
//

#pragma once

#include "nlohmann/json.hpp"
#include "bilibili/result/user_result.h"

namespace bilibili {

class EpisodesBadge {
public:
    std::string text, bg_color, bg_color_night;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EpisodesBadge, text, bg_color,
                                   bg_color_night);

class SeasonEpisodeResult {
public:
    SeasonEpisodeResult() = default;
    SeasonEpisodeResult(std::string title) : title(title) { id = 0; }
    size_t index;
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
    EpisodesBadge badge_info;

    int progress = -1;  // 用于从历史记录加载进播放页
};
inline void from_json(const nlohmann::json& nlohmann_json_j,
                      SeasonEpisodeResult& nlohmann_json_t) {
    // 如果是合集，则没有bvid, duration, long_title, subtitle
    if (nlohmann_json_j.contains("bvid")) {
        nlohmann_json_j.at("bvid").get_to(nlohmann_json_t.bvid);
    }
    if (nlohmann_json_j.contains("duration")) {
        nlohmann_json_j.at("duration").get_to(nlohmann_json_t.duration);
    }
    if (nlohmann_json_j.contains("long_title")) {
        nlohmann_json_j.at("long_title").get_to(nlohmann_json_t.long_title);
    }
    if (nlohmann_json_j.contains("subtitle")) {
        nlohmann_json_j.at("subtitle").get_to(nlohmann_json_t.subtitle);
    }

    if (nlohmann_json_j.contains("badge_info")) {
        nlohmann_json_j.at("badge_info").get_to(nlohmann_json_t.badge_info);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, aid, cid,
                                             title, pub_time, link, id));
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

class SeasonSection {
public:
    SeasonEpisodeListResult episodes;
    size_t id;
    std::string title;
    int type;
    int attr;
};
inline void from_json(const nlohmann::json& nlohmann_json_j,
                      SeasonSection& nlohmann_json_t) {
    if (nlohmann_json_j.contains("episodes")) {
        nlohmann_json_j.at("episodes").get_to(nlohmann_json_t.episodes);
        for (size_t i = 0; i < nlohmann_json_t.episodes.size(); i++) {
            nlohmann_json_t.episodes[i].index = i;
        }
    }
    NLOHMANN_JSON_EXPAND(
        NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, id, title, type, attr));
}

typedef std::vector<SeasonSection> SeasonSections;

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
    SeasonSections section;
    size_t show_season_type;  //1,4: 第*话; 3: 第*集, 2:电影 不显示前缀;
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
    if (nlohmann_json_j.contains("episodes")) {
        nlohmann_json_j.at("episodes").get_to(nlohmann_json_t.episodes);
        for (size_t i = 0; i < nlohmann_json_t.episodes.size(); i++) {
            nlohmann_json_t.episodes[i].index = i;
        }
    }
    if (nlohmann_json_j.contains("section")) {
        nlohmann_json_j.at("section").get_to(nlohmann_json_t.section);
    }
    if (nlohmann_json_j.contains("show_season_type")) {
        nlohmann_json_j.at("show_season_type")
            .get_to(nlohmann_json_t.show_season_type);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, season_id,
                                             season_title, evaluate));
}

};  // namespace bilibili