//
// Created by fang on 2022/7/8.
//

#pragma once

#include "nlohmann/json.hpp"
#include "user_result.h"
#include "home_result.h"

using namespace std;

namespace bilibili {

    class HotsRankPGCConfig {
    public:
        string index_show;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsRankPGCConfig, index_show);

    class HotsRankPGCVideoResult {
    public:
        int rank;
        int season_id;
        string ss_horizontal_cover; //横版封面
        string cover; //竖版封面
        string title;
        string rating;
        HotsRankPGCConfig new_ep;
        VideoSimpleStateResult stat;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsRankPGCVideoResult, title, rank, season_id, ss_horizontal_cover, new_ep, stat);

    typedef vector<HotsRankPGCVideoResult> HotsRankPGCVideoListResult;

    class HotsRankPGCVideoListResultWrapper {
    public:
        string note;
        HotsRankPGCVideoListResult list;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsRankPGCVideoListResultWrapper, note, list);


    class HotsRankVideoResult {
    public:
        int aid;
        string bvid;
        int cid;
        string pic;
        string title;
        int duration;
        int pubdate;
        UserSimpleResult owner;
        VideoSimpleStateResult stat;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsRankVideoResult, aid, bvid, cid, pic, title, duration, pubdate, owner, stat);

    typedef vector<HotsRankVideoResult> HotsRankVideoListResult;

    class HotsRankVideoListResultWrapper {
    public:
        string note;
        HotsRankVideoListResult list;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HotsRankVideoListResultWrapper, note, list);

};