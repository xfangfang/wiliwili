//
// Created by fang on 2022/7/13.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"

namespace bilibili {

class LiveQuality {
public:
    int qn;
    std::string desc;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveQuality, qn, desc);

typedef std::vector<LiveQuality> LiveQualityList;

class LiveUrlResult {
public:
    std::string url;
    unsigned int order;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveUrlResult, url, order);

typedef std::vector<LiveUrlResult> LiveUrlListResult;

class LiveUrlResultWrapper {
public:
    int current_qn;
    LiveQualityList quality_description;
    LiveUrlListResult durl;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveUrlResultWrapper, current_qn, durl, quality_description);
class LiveQnDesc {
public:
    int qn;
    std::string desc;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveQnDesc, qn, desc);

class LiveStreamUrlInfo {
public:
    std::string host;
    std::string extra;
    int stream_ttl;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveStreamUrlInfo, host, extra, stream_ttl);

class LiveStreamFormatCodec {
public:
    std::string codec_name;
    int current_qn;
    std::vector<int> accept_qn;
    std::string base_url;
    std::vector<LiveStreamUrlInfo> url_info;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveStreamFormatCodec, codec_name, current_qn, accept_qn, base_url, url_info);

class LiveStreamFormat {
public:
    std::string format_name;
    std::vector<LiveStreamFormatCodec> codec;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveStreamFormat, format_name, codec);

class LiveStream {
public:
    std::string protocol_name;
    std::vector<LiveStreamFormat> format;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveStream, protocol_name, format);

class LivePlayUrl {
public:
    std::vector<LiveQnDesc> g_qn_desc;
    std::vector<LiveStream> stream;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LivePlayUrl, g_qn_desc, stream);

class LivePlayUrlInfo {
public:
    LivePlayUrl playurl;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, LivePlayUrlInfo& nlohmann_json_t) {
    if (!nlohmann_json_j.at("playurl").is_null()) {
        nlohmann_json_j.at("playurl").get_to(nlohmann_json_t.playurl);
    }
}

class LiveRoomPlayInfo {
public:
    int room_id;
    int64_t uid;
    int live_status;
    size_t live_time;
    LivePlayUrlInfo playurl_info;
    bool is_locked;  // 是否被封禁
    int lock_till;   // 封禁解锁时间
};
inline void from_json(const nlohmann::json& nlohmann_json_j, LiveRoomPlayInfo& nlohmann_json_t) {
    if (!nlohmann_json_j.at("playurl_info").is_null()) {
        nlohmann_json_j.at("playurl_info").get_to(nlohmann_json_t.playurl_info);
    }
    NLOHMANN_JSON_EXPAND(
        NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, room_id, uid, live_status, live_time, is_locked, lock_till));
}

class LivePayInfo {
public:
    int permission;
    std::string pic;

    std::string message;  // 额外的提示信息
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LivePayInfo, permission, pic);

class LivePayLink {
public:
    std::string start_time;
    std::string end_time;
    std::string goods_link;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LivePayLink, start_time, end_time, goods_link);

class LiveAreaResult {
public:
    LiveAreaResult() = default;
    LiveAreaResult(int id, std::string title, int area_v2_id, int area_v2_parent_id)
        : id(id), title(title), area_v2_id(area_v2_id), area_v2_parent_id(area_v2_parent_id) {}
    int id;
    std::string title;
    int area_v2_id;
    int area_v2_parent_id;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveAreaResult, id, title, area_v2_id, area_v2_parent_id);

/**
 * 直播间相关信息
 */
class ShowInfo {
public:
    int num = 0;             // eg: 14130
    std::string text_small;  // eg: 1.4万
    std::string text_large;  // eg: 1.4万人看过
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShowInfo, num, text_small, text_large);

class LiveVideoResult {
public:
    int roomid;  // small_card_v1 roomid 标记为id
    int uid;
    std::string title;
    std::string uname;
    int online;
    std::string cover;
    std::string area_name;
    bool following = false;  //是否为我关注的主播，自定义数据
    ShowInfo watched_show;
};

inline void from_json(const nlohmann::json& nlohmann_json_j, LiveVideoResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("roomid")) {
        nlohmann_json_j.at("roomid").get_to(nlohmann_json_t.roomid);
    } else if (nlohmann_json_j.contains("id")) {
        nlohmann_json_j.at("id").get_to(nlohmann_json_t.roomid);
    } else {
        nlohmann_json_t.roomid = -1;
    }
    NLOHMANN_JSON_EXPAND(
        NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, uid, title, uname, online, cover, area_name, watched_show));
}

typedef std::vector<LiveVideoResult> LiveVideoListResult;
typedef std::vector<LiveAreaResult> LiveAreaListResult;

class LiveResultWrapper {
public:
    LiveVideoListResult card_list;
    LiveVideoListResult my_list;  // 我关注的主播
    LiveAreaListResult live_list;
    int has_more;
};

inline void from_json(const nlohmann::json& nlohmann_json_j, LiveResultWrapper& nlohmann_json_t) {
    for (auto i : nlohmann_json_j.at("card_list")) {
        std::string card_type = i.at("card_type");
        auto& card_data       = i.at("card_data");

        if (card_type.compare("area_entrance_v1") == 0) {
            card_data.at("area_entrance_v1").at("list").get_to(nlohmann_json_t.live_list);
        } else if (card_type.compare("second_card_v1") == 0) {
            nlohmann_json_t.card_list.push_back(card_data.at("second_card_v1").get<LiveVideoResult>());
        } else if (card_type.compare("small_card_v1") == 0) {
            nlohmann_json_t.card_list.push_back(card_data.at("small_card_v1").get<LiveVideoResult>());
        } else if (card_type.compare("my_idol_v1") == 0) {
            card_data.at("my_idol_v1").at("list").get_to(nlohmann_json_t.my_list);
            for (auto& up : nlohmann_json_t.my_list) {
                up.following = true;
            }
        }
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_more));
}

// 细化到二级分区的直播推荐
class LiveSecondResultWrapper {
public:
    LiveVideoListResult list;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, LiveSecondResultWrapper& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, list));
}

// 二级分区列表
class LiveSubAreaResult {
public:
    int id        = 0;
    int parent_id = 0;
    std::string name;
    std::string pic;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveSubAreaResult, id, parent_id, name, pic)
typedef std::vector<LiveSubAreaResult> LiveSubAreaListResult;

// 包含全部直播分区的列表 (一级分区列表)
class LiveFullAreaResult {
public:
    int id = 0;
    std::string name;
    LiveSubAreaListResult area_list;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, LiveFullAreaResult& nlohmann_json_t) {
    if (nlohmann_json_j.at("area_list").is_array()) {
        nlohmann_json_j.at("area_list").get_to(nlohmann_json_t.area_list);
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, id, name))
    if (nlohmann_json_t.id == 0) {
        LiveSubAreaResult all{0, 0, "全部推荐", ""};
        nlohmann_json_t.area_list.insert(nlohmann_json_t.area_list.begin(), all);
    }
}

typedef std::vector<LiveFullAreaResult> LiveFullAreaListResult;

class LiveFullAreaResultWrapper {
public:
    LiveFullAreaListResult list;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, LiveFullAreaResultWrapper& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, list))
}

}  // namespace bilibili