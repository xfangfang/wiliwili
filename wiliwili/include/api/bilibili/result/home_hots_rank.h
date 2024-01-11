//
// Created by fang on 2022/7/8.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"
#include "home_result.h"

namespace bilibili {

class HotsRankPGCConfig {
public:
    std::string index_show;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsRankPGCConfig, index_show);

class HotsRankPGCVideoResult {
public:
    int rank;
    int season_id;
    std::string ss_horizontal_cover;  //横版封面
    std::string cover;                //竖版封面
    std::string title;
    std::string rating;
    HotsRankPGCConfig new_ep;
    VideoSimpleStateResult stat;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsRankPGCVideoResult, title, rank, season_id, ss_horizontal_cover, new_ep, stat);

typedef std::vector<HotsRankPGCVideoResult> HotsRankPGCVideoListResult;

class HotsRankPGCVideoListResultWrapper {
public:
    std::string note;
    HotsRankPGCVideoListResult list;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsRankPGCVideoListResultWrapper, note, list);

class HotsRankVideoResult {
public:
    int aid;
    std::string bvid;
    int cid;
    std::string pic;
    std::string title;
    int duration;
    int pubdate;
    UserSimpleResult owner;
    VideoSimpleStateResult stat;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsRankVideoResult, aid, bvid, cid, pic, title, duration, pubdate, owner, stat);

typedef std::vector<HotsRankVideoResult> HotsRankVideoListResult;

class HotsRankVideoListResultWrapper {
public:
    std::string note;
    HotsRankVideoListResult list;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsRankVideoListResultWrapper, note, list);

};  // namespace bilibili