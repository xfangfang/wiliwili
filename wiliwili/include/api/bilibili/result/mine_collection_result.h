//
// Created by fang on 2022/7/28.
//

#pragma once

#include "nlohmann/json.hpp"
#include "bilibili/result/user_result.h"

namespace bilibili {

// 当前页面类型，1为收藏夹列表 2为订阅列表
#define COLLECTION_UI_TYPE_1 1
#define COLLECTION_UI_TYPE_2 2

// 订阅列表页面单个订阅的类型
#define SUBSCRIPTION_TYPE_1 11  // 订阅的收藏夹
#define SUBSCRIPTION_TYPE_2 21  // 订阅的合集

class SimpleCollectionList {
public:
    uint64_t id;
    uint64_t fid;
    uint64_t mid;
    unsigned int attr;
    std::string title;
    unsigned int fav_state;
    unsigned int media_count;
    size_t index;

    bool operator<(const SimpleCollectionList& item) const {
        if (this->fav_state == item.fav_state) {
            return this->index < item.index;
        }
        return this->fav_state > item.fav_state;
    }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimpleCollectionList, id, fid, mid, attr, title, fav_state, media_count);

typedef std::vector<SimpleCollectionList> SimpleCollectionListResult;

class CollectionResult {
public:
    uint64_t id  = 0;
    uint64_t fid = 0;
    int type    = SUBSCRIPTION_TYPE_2;  // 视频列表的类型, 11: 收藏夹 21: 合集
    std::string cover;
    std::string title;
    std::string intro;
    unsigned int ctime = 0, mtime = 0;
    unsigned int media_count = 0, view_count = 0;
    unsigned int attr = 0;
    UserSimpleResult upper;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, CollectionResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("view_count")) {
        nlohmann_json_j.at("view_count").get_to(nlohmann_json_t.view_count);
    }
    if (nlohmann_json_j.contains("fid")) {
        nlohmann_json_j.at("fid").get_to(nlohmann_json_t.fid);
    }
    if (nlohmann_json_j.contains("ctime")) {
        nlohmann_json_j.at("ctime").get_to(nlohmann_json_t.ctime);
    }
    if (nlohmann_json_j.contains("mtime")) {
        nlohmann_json_j.at("mtime").get_to(nlohmann_json_t.mtime);
    }
    if (nlohmann_json_j.contains("intro")) {
        nlohmann_json_j.at("intro").get_to(nlohmann_json_t.intro);
    }
    if (nlohmann_json_j.contains("attr")) {
        nlohmann_json_j.at("attr").get_to(nlohmann_json_t.attr);
    }
    if (nlohmann_json_j.contains("type")) {
        nlohmann_json_j.at("type").get_to(nlohmann_json_t.type);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, cover, id, title, media_count, upper));
}

typedef std::vector<CollectionResult> CollectionListResult;

class CollectionListResultWrapper {
public:
    CollectionListResult list;
    bool has_more;
    int count;
    unsigned int index;  // 当前数据的索引值
};
inline void from_json(const nlohmann::json& nlohmann_json_j, CollectionListResultWrapper& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_more, list, count));
}

class VideoCollectionStateResult {
public:
    unsigned int collect;
    unsigned int danmaku;
    unsigned int play;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoCollectionStateResult, collect, danmaku, play);

class CollectionVideoResult {
public:
    int id = 0;
    // 默认设置为 2: UGC 视频（合集api没有设置这个属性，所以设置一个默认值）
    int type = 2;
    std::string title;
    std::string cover;
    std::string intro;
    UserSimpleResult upper;
    VideoCollectionStateResult cnt_info;
    int duration         = 0;
    unsigned int pubtime = 0;
    std::string bvid;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, CollectionVideoResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("type")) {
        nlohmann_json_j.at("type").get_to(nlohmann_json_t.type);
    }
    if (nlohmann_json_j.contains("intro")) {
        nlohmann_json_j.at("intro").get_to(nlohmann_json_t.intro);
    }
    NLOHMANN_JSON_EXPAND(
        NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, id, title, cover, upper, cnt_info, duration, pubtime, bvid));
}

typedef std::vector<CollectionVideoResult> CollectionVideoListResult;

class CollectionVideoListResultWrapper {
public:
    CollectionVideoListResult medias;
    bool has_more = false;
    CollectionResult info;
    unsigned int index = 0;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, CollectionVideoListResultWrapper& nlohmann_json_t) {
    if (nlohmann_json_j.at("medias").is_array()) {
        nlohmann_json_j.at("medias").get_to(nlohmann_json_t.medias);
    }
    if (nlohmann_json_j.contains("has_more")) {
        nlohmann_json_j.at("has_more").get_to(nlohmann_json_t.has_more);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, info));
}

class SimpleCollectionListResultWrapper {
public:
    int count;
    SimpleCollectionListResult list;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimpleCollectionListResultWrapper, count, list);

};  // namespace bilibili