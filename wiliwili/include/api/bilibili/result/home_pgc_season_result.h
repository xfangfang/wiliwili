//
// Created by fang on 2022/7/31.
//

#pragma once

#include "nlohmann/json.hpp"
#include "bilibili/result/user_result.h"

namespace bilibili {

class SeasonStatusResult {
public:
    size_t last_ep_id;
    size_t last_time;
    int follow;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonStatusResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("progress")) {
        auto& p = nlohmann_json_j.at("progress");
        p.at("last_ep_id").get_to(nlohmann_json_t.last_ep_id);
        p.at("last_time").get_to(nlohmann_json_t.last_time);
    } else {
        nlohmann_json_t.last_ep_id = 0;
        nlohmann_json_t.last_time  = 0;
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, follow));
}

class EpisodesBadge {
public:
    std::string text, bg_color, bg_color_night;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EpisodesBadge, text, bg_color, bg_color_night);

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
inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonEpisodeResult& nlohmann_json_t) {
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
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, aid, cid, title, pub_time, link, id));
}

class SeasonRatingResult {
public:
    float score        = -1;
    unsigned int count = 0;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonRatingResult& nlohmann_json_t) {
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
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonDetailStat, views, danmakus, favorite, favorites, coins, share, likes, reply);

class SeasonPublishStat {
public:
    int is_finish, is_started;
    std::string pub_time_show;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonPublishStat, is_finish, is_started, pub_time_show);

class SeasonSection {
public:
    SeasonEpisodeListResult episodes;
    size_t id;
    std::string title;
    int type;
    int attr;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonSection& nlohmann_json_t) {
    if (nlohmann_json_j.contains("episodes")) {
        nlohmann_json_j.at("episodes").get_to(nlohmann_json_t.episodes);
        for (size_t i = 0; i < nlohmann_json_t.episodes.size(); i++) {
            nlohmann_json_t.episodes[i].index = i;
        }
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, id, title, type, attr));
}

class SeasonSeriesItemStat {
public:
    size_t series_follow, views;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonSeriesItemStat, views, series_follow);

class SeasonSeriesItem {
public:
    SeasonSeriesItem() : season_id(0) {}

    EpisodesBadge badge_info;
    std::string season_title, cover, subtitle;
    size_t season_id;
    SeasonSeriesItemStat stat;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonSeriesItem& nlohmann_json_t) {
    if (nlohmann_json_j.contains("new_ep")) {
        auto& ep = nlohmann_json_j.at("new_ep");
        ep.at("index_show").get_to(nlohmann_json_t.subtitle);
    }

    if (nlohmann_json_j.contains("horizontal_cover_1610") && !nlohmann_json_j.at("horizontal_cover_1610").is_null() &&
        !nlohmann_json_j.at("horizontal_cover_1610").get<std::string>().empty()) {
        nlohmann_json_j.at("horizontal_cover_1610").get_to(nlohmann_json_t.cover);
    } else if (nlohmann_json_j.contains("horizontal_cover_169") &&
               !nlohmann_json_j.at("horizontal_cover_169").is_null() &&
               !nlohmann_json_j.at("horizontal_cover_169").get<std::string>().empty()) {
        nlohmann_json_j.at("horizontal_cover_169").get_to(nlohmann_json_t.cover);
    } else if (nlohmann_json_j.contains("cover")) {
        nlohmann_json_j.at("cover").get_to(nlohmann_json_t.cover);
    }

    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, badge_info, season_id, season_title, stat));
}

class SeasonSeriesTitle {
public:
    std::string series_title;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonSeriesTitle, series_title);

class SeasonRecommendItemStat {
public:
    size_t follow, view;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonRecommendItemStat, view, follow);

class SeasonRecommendItem {
public:
    std::string cover, title, subtitle;
    float score;
    SeasonRecommendItemStat stat;
    size_t season_id;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonRecommendItem& nlohmann_json_t) {
    if (nlohmann_json_j.contains("new_ep")) {
        auto& ep = nlohmann_json_j.at("new_ep");
        ep.at("cover").get_to(nlohmann_json_t.cover);
        ep.at("index_show").get_to(nlohmann_json_t.subtitle);
    }
    if (nlohmann_json_j.contains("rating")) {
        auto& rating = nlohmann_json_j.at("rating");
        rating.at("score").get_to(nlohmann_json_t.score);
    } else {
        nlohmann_json_t.score = -1;
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, season_id, title, stat));
}

typedef std::vector<SeasonSection> SeasonSections;
typedef std::vector<SeasonSeriesItem> SeasonSeries;
typedef std::vector<SeasonRecommendItem> SeasonRecommend;

class SeasonRecommendWrapper {
public:
    SeasonRecommend season;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonRecommendWrapper& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, season));
}

class SeasonResultWrapper {
public:
    unsigned int season_id;
    std::string cover;
    std::string season_title;
    std::string season_desc;
    SeasonEpisodeListResult episodes;
    SeasonRatingResult rating;
    std::string evaluate;
    SeasonUserResult up_info;
    SeasonDetailStat stat;
    SeasonPublishStat publish;
    SeasonSections section;
    size_t type;
    SeasonStatusResult user_status;
    SeasonSeries seasons;
    //    SeasonSeriesTitle series;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, SeasonResultWrapper& nlohmann_json_t) {
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
    if (nlohmann_json_j.contains("new_ep") && nlohmann_json_j.at("new_ep").contains("desc")) {
        nlohmann_json_j.at("new_ep").at("desc").get_to(nlohmann_json_t.season_desc);
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
    if (nlohmann_json_j.contains("seasons")) {
        nlohmann_json_j.at("seasons").get_to(nlohmann_json_t.seasons);
    }
    //    if (nlohmann_json_j.contains("series")) {
    //        nlohmann_json_j.at("series").get_to(nlohmann_json_t.series);
    //    }
    if (nlohmann_json_j.contains("type")) {
        nlohmann_json_j.at("type").get_to(nlohmann_json_t.type);
    }
    if (nlohmann_json_j.contains("user_status")) {
        nlohmann_json_j.at("user_status").get_to(nlohmann_json_t.user_status);
    }
    if (nlohmann_json_j.contains("cover")) {
        nlohmann_json_j.at("cover").get_to(nlohmann_json_t.cover);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, season_id, season_title, evaluate));
}

};  // namespace bilibili