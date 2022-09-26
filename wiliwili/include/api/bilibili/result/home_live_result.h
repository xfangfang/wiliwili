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
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveUrlResultWrapper, current_qn, durl,
                                   quality_description);

class LiveAreaResult {
public:
    LiveAreaResult() = default;
    LiveAreaResult(int id, std::string title, int area_v2_id,
                   int area_v2_parent_id)
        : id(id),
          title(title),
          area_v2_id(area_v2_id),
          area_v2_parent_id(area_v2_parent_id) {}
    int id;
    std::string title;
    int area_v2_id;
    int area_v2_parent_id;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveAreaResult, id, title, area_v2_id,
                                   area_v2_parent_id);

/**
 * 直播间相关信息
 */
class ShowInfo {
public:
    int num                = 0;   // eg: 14130
    std::string text_small = "";  // eg: 1.4万
    std::string text_large = "";  // eg: 1.4万人看过
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShowInfo, num, text_small, text_large);

class LiveVideoResult {
public:
    int roomid;  // small_card_v1 roomid 标记为id
    int uid;
    std::string title;
    std::string uname;
    int online;
    std::string play_url;
    std::string cover;
    std::string area_name;
    bool following = false;  //是否为我关注的主播
    ShowInfo watched_show;
    int current_qn = 10000;
    LiveQualityList quality_description;
};

inline void from_json(const nlohmann::json& nlohmann_json_j,
                      LiveVideoResult& nlohmann_json_t) {
    if (nlohmann_json_j.contains("roomid")) {
        nlohmann_json_j.at("roomid").get_to(nlohmann_json_t.roomid);
    } else if (nlohmann_json_j.contains("id")) {
        nlohmann_json_j.at("id").get_to(nlohmann_json_t.roomid);
    } else {
        nlohmann_json_t.roomid = -1;
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(
        NLOHMANN_JSON_FROM, uid, title, uname, online, play_url, cover,
        area_name, watched_show, current_qn, quality_description));
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

inline void from_json(const nlohmann::json& nlohmann_json_j,
                      LiveResultWrapper& nlohmann_json_t) {
    for (auto i : nlohmann_json_j.at("card_list")) {
        std::string card_type = i.at("card_type");
        auto& card_data       = i.at("card_data");

        if (card_type.compare("area_entrance_v1") == 0) {
            card_data.at("area_entrance_v1")
                .at("list")
                .get_to(nlohmann_json_t.live_list);
        } else if (card_type.compare("second_card_v1") == 0) {
            nlohmann_json_t.card_list.push_back(
                card_data.at("second_card_v1").get<LiveVideoResult>());
        } else if (card_type.compare("small_card_v1") == 0) {
            nlohmann_json_t.card_list.push_back(
                card_data.at("small_card_v1").get<LiveVideoResult>());
        } else if (card_type.compare("my_idol_v1") == 0) {
            card_data.at("my_idol_v1")
                .at("list")
                .get_to(nlohmann_json_t.my_list);
            for (auto& up : nlohmann_json_t.my_list) {
                up.following = true;
            }
        }
    }
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, has_more));
}
}  // namespace bilibili