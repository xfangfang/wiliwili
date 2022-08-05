//
// Created by fang on 2022/7/28.
//

#pragma once

#include "nlohmann/json.hpp"
#include "bilibili/result/user_result.h"

using namespace std;

namespace bilibili {

    class CollectionResult {
    public:
        u_int id;
        std::string cover;
        std::string title;
        u_int ctime;
        u_int media_count;
        u_int attr;
        UserSimpleResult upper;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, CollectionResult& nlohmann_json_t) {
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, cover, id, title, ctime, media_count, attr, upper));
    }

    typedef std::vector<CollectionResult> CollectionListResult;

    class CollectionListResultWrapper {
    public:
        CollectionListResult list;
        bool has_more;
        int count;
        UserSimpleResult upper;
        u_int index;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, CollectionListResultWrapper& nlohmann_json_t) {
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_more, list, count));
    }

    class VideoCollectionStateResult {
    public:
        u_int collect;
        u_int danmaku;
        u_int play;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoCollectionStateResult, collect, danmaku, play);

    class CollectionVideoResult{
    public:
        int id;
        int type;
        string title;
        string cover;
        UserSimpleResult upper;
        VideoCollectionStateResult cnt_info;
        int duration;
        u_int pubtime;
        string bvid;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, CollectionVideoResult& nlohmann_json_t) {
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, id, type, title, cover, upper, cnt_info, duration, pubtime, bvid));
    }

    typedef std::vector<CollectionVideoResult>  CollectionVideoListResult;


    class CollectionVideoListResultWrapper {
    public:
        CollectionVideoListResult medias;
        bool has_more;
        CollectionResult info;
        u_int index;
    };
    inline void from_json(const nlohmann::json& nlohmann_json_j, CollectionVideoListResultWrapper& nlohmann_json_t) {
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_more, medias, info));
    }
};