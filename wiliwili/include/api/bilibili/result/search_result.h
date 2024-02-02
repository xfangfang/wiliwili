//
// Created by fang on 2022/8/2.
//

#pragma once

#include "nlohmann/json.hpp"
#include <pystring.h>

namespace bilibili {

class VideoSearchBadgeResult {
public:
    std::string text;
    std::string text_color;
    std::string text_color_night;
    std::string bg_color;
    std::string bg_color_night;
    std::string border_color;
    std::string border_color_night;
    int bg_style;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoSearchBadgeResult, text, bg_color);

typedef std::vector<VideoSearchBadgeResult> VideoSearchBadgeListResult;

class MediaScore {
public:
    float score;
    int user_count;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MediaScore, score, user_count);

class VideoItemSearchResult {
public:
    // 搜索的类型：video/media_bangumi/media_ft 视频/影视/番剧
    std::string type;

    unsigned int aid;
    std::string bvid;
    unsigned int season_id;

    std::string title;  // 部分带xml标识关键词
    std::string subtitle;
    std::string areas;             // PGC 作品国家
    std::string season_type_name;  // eg: 番剧
    std::string styles;            // eg: 奇幻/校园
    std::string desc;
    std::string index_show;  // eg:  全12话
    std::string cv;          // 演员
    std::string staff;       //导演
    MediaScore media_score;

    std::string cover;  // 封面

    unsigned int play;     // 播放量
    unsigned int danmaku;  // 弹幕数量
    unsigned int like;     //点赞量

    unsigned int pubdate;  // 发布日期

    std::string rightBottomBadge;

    VideoSearchBadgeResult badge;
};

inline void from_json(const nlohmann::json &nlohmann_json_j, VideoItemSearchResult &nlohmann_json_t) {
    auto video_type = nlohmann_json_j.at("type").get<std::string>();
    if (video_type == "video") {
        nlohmann_json_j.at("aid").get_to(nlohmann_json_t.aid);
        nlohmann_json_j.at("bvid").get_to(nlohmann_json_t.bvid);
        nlohmann_json_j.at("author").get_to(nlohmann_json_t.subtitle);
        nlohmann_json_j.at("duration").get_to(nlohmann_json_t.rightBottomBadge);
        nlohmann_json_j.at("pic").get_to(nlohmann_json_t.cover);
        nlohmann_json_j.at("play").get_to(nlohmann_json_t.play);
        nlohmann_json_j.at("danmaku").get_to(nlohmann_json_t.danmaku);
        nlohmann_json_j.at("like").get_to(nlohmann_json_t.like);
        nlohmann_json_j.at("pubdate").get_to(nlohmann_json_t.pubdate);
        if (pystring::startswith(nlohmann_json_t.cover, "//")) {
            nlohmann_json_t.cover = "http:" + nlohmann_json_t.cover;
        }
    } else if (video_type == "ketang") {
        nlohmann_json_j.at("aid").get_to(nlohmann_json_t.aid);
        nlohmann_json_j.at("pic").get_to(nlohmann_json_t.cover);
        nlohmann_json_j.at("author").get_to(nlohmann_json_t.subtitle);
        nlohmann_json_j.at("play").get_to(nlohmann_json_t.play);
    } else if (video_type == "media_bangumi" || video_type == "media_ft") {
        // media_bangumi: 番剧
        // media_ft: 影视
        nlohmann_json_j.at("season_id").get_to(nlohmann_json_t.season_id);
        nlohmann_json_j.at("pubtime").get_to(nlohmann_json_t.pubdate);
        nlohmann_json_j.at("cover").get_to(nlohmann_json_t.cover);
        nlohmann_json_j.at("index_show").get_to(nlohmann_json_t.index_show);
        nlohmann_json_j.at("media_score").get_to(nlohmann_json_t.media_score);
        nlohmann_json_j.at("styles").get_to(nlohmann_json_t.styles);
        nlohmann_json_j.at("areas").get_to(nlohmann_json_t.areas);
        nlohmann_json_j.at("desc").get_to(nlohmann_json_t.desc);
        nlohmann_json_t.cv    = pystring::replace(nlohmann_json_j.at("cv").get<std::string>(), "\n", " ");
        nlohmann_json_t.staff = pystring::replace(nlohmann_json_j.at("staff").get<std::string>(), "\n", " ");
        nlohmann_json_j.at("season_type_name").get_to(nlohmann_json_t.season_type_name);

        if (nlohmann_json_j.contains("badges") && nlohmann_json_j.at("badges").is_array() &&
            nlohmann_json_j.at("badges").size() > 0) {
            nlohmann_json_j.at("badges")[0].get_to(nlohmann_json_t.badge);
        }
    }

    nlohmann_json_j.at("title").get_to(nlohmann_json_t.title);

    nlohmann_json_t.title = pystring::replace(nlohmann_json_t.title, "<em class=\"keyword\">", "");
    nlohmann_json_t.title = pystring::replace(nlohmann_json_t.title, "</em>", "");
    nlohmann_json_t.title = pystring::replace(nlohmann_json_t.title, "&quot;", "\"");
}

typedef std::vector<VideoItemSearchResult> VideoItemSearchListResult;

class SearchResult {
public:
    unsigned int page;
    unsigned int pagesize;
    unsigned int numResults;
    unsigned int numPages;
    VideoItemSearchListResult result;
};

inline void from_json(const nlohmann::json &nlohmann_json_j, SearchResult &nlohmann_json_t) {
    if (nlohmann_json_j.contains("result")) {
        nlohmann_json_j.at("result").get_to(nlohmann_json_t.result);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, page, pagesize, numResults, numPages));
}

class SearchHotsResult {
public:
    std::string keyword;
    std::string show_name;
    std::string icon;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SearchHotsResult, keyword, show_name, icon);

typedef std::vector<SearchHotsResult> SearchHotsListResult;

class SearchHotsResultWrapper {
public:
    SearchHotsListResult list;
};

inline void from_json(const nlohmann::json &nlohmann_json_j, SearchHotsResultWrapper &nlohmann_json_t) {
    if (nlohmann_json_j.contains("trending")) {
        nlohmann_json_j.at("trending").at("list").get_to(nlohmann_json_t.list);
    }
}

class SearchSuggest {
public:
    std::string value;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SearchSuggest, value);

class SearchSuggestList {
public:
    std::vector<SearchSuggest> tag;
};
inline void from_json(const nlohmann::json &nlohmann_json_j, SearchSuggestList &nlohmann_json_t) {
    if (nlohmann_json_j.contains("tag") && nlohmann_json_j.at("tag").is_array()) {
        nlohmann_json_j.at("tag").get_to(nlohmann_json_t.tag);
    }
}
}  // namespace bilibili