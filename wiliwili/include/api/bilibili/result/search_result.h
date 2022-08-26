//
// Created by fang on 2022/8/2.
//

#pragma once

#include "nlohmann/json.hpp"
#include "pystring.h"

using namespace std;

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

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoSearchBadgeResult, text);

    typedef vector<VideoSearchBadgeResult> VideoSearchBadgeListResult;


    class VideoItemSearchResult {
    public:
        std::string type; // 搜索的类型：video/media_bangumi/media_ft 视频/影视/番剧

        uint aid;
        std::string bvid;
        uint season_id;

        std::string title; // 部分带xml标识关键词
        std::string subtitle;

        std::string cover; // 封面

        uint play; // 播放量
        uint danmaku; // 弹幕数量
        uint like; //点赞量

        uint pubdate; // 发布日期

        std::string rightBottomBadge;

        VideoSearchBadgeListResult badges;
    };

    inline void from_json(const nlohmann::json &nlohmann_json_j, VideoItemSearchResult &nlohmann_json_t) {
        if (nlohmann_json_j.at("type") == "video") {
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
        } else {
            nlohmann_json_j.at("season_id").get_to(nlohmann_json_t.season_id);
            nlohmann_json_j.at("pubtime").get_to(nlohmann_json_t.pubdate);
            nlohmann_json_j.at("cover").get_to(nlohmann_json_t.cover);
            nlohmann_json_j.at("index_show").get_to(nlohmann_json_t.rightBottomBadge);

            nlohmann_json_t.subtitle =
                    nlohmann_json_j.at("areas").get<std::string>() +
                    nlohmann_json_j.at("styles").get<std::string>();
        }

        if (nlohmann_json_j.contains("badges") && !nlohmann_json_j.at("badges").is_null()) {
            nlohmann_json_j.at("badges").get_to(nlohmann_json_t.badges);
        }


        nlohmann_json_j.at("title").get_to(nlohmann_json_t.title);

        nlohmann_json_t.title = pystring::replace(nlohmann_json_t.title, "<em class=\"keyword\">", "");
        nlohmann_json_t.title = pystring::replace(nlohmann_json_t.title, "</em>", "");
    }

    typedef vector<VideoItemSearchResult> VideoItemSearchListResult;


    class SearchResult {
    public:
        uint page;
        uint pagesize;
        uint numResults;
        uint numPages;
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
        string keyword;
        string show_name;
        string icon;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SearchHotsResult, keyword, show_name, icon);

    typedef vector<SearchHotsResult> SearchHotsListResult;

    class SearchHotsResultWrapper {
    public:
        SearchHotsListResult list;
    };

    inline void from_json(const nlohmann::json &nlohmann_json_j, SearchHotsResultWrapper &nlohmann_json_t) {
        if (nlohmann_json_j.contains("trending")) {
            nlohmann_json_j.at("trending").at("list").get_to(nlohmann_json_t.list);
        }
    }
}