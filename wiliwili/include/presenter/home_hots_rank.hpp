//
// Created by fang on 2022/7/8.
//

#pragma once

#include "bilibili.h"
#include "bilibili/result/home_hots_rank.h"

class RankType {
public:
    RankType(std::string key, int type, int id, std::string extra = "all")
        : type(type), id(id), key(key), extra(extra) {}

    int type;  // 0: 用户投稿， 1: 官方番剧
    int id;
    std::string key;
    std::string extra;
};

class HomeHotsRankRequest {
public:
    virtual void onHotsRankList(const bilibili::HotsRankVideoListResult& result, const std::string& note) {}
    virtual void onHotsRankPGCList(const bilibili::HotsRankPGCVideoListResult& result, const std::string& note) {}
    virtual void onError(const std::string& error) {}

    std::vector<RankType> rankList = {
        {"全站", 0, 0},     {"番剧", 1, 1},   {"国产动画", 1, 4}, {"国创相关", 0, 168},     {"纪录片", 1, 3},
        {"动画", 0, 1},     {"音乐", 0, 3},   {"舞蹈", 0, 129},   {"游戏", 0, 4},           {"知识", 0, 36},
        {"科技", 0, 188},   {"运动", 0, 234}, {"汽车", 0, 223},   {"生活", 0, 160},         {"美食", 0, 211},
        {"动物圈", 0, 217}, {"鬼畜", 0, 119}, {"时尚", 0, 155},   {"娱乐", 0, 5},           {"影视", 0, 181},
        {"电影", 1, 2},     {"电视剧", 1, 5}, {"综艺", 1, 7},     {"原创", 0, 0, "origin"}, {"新人", 0, 0, "rookie"},
    };

    std::vector<std::string> getRankList();

    void requestData(size_t index = 0);

    /// 主页 热门 排行榜 投稿视频
    void requestHotsRankVideoList(int rid, std::string type);

    /// 主页 热门 排行榜 官方
    void requestHotsRankPGCVideoList(int season_type, int day = 3);
};
