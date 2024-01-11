//
// Created by fang on 2022/7/6.
//

#pragma once

#include "bilibili/result/home_hots_weekly_result.h"

class HomeHotsWeeklyRequest {
public:
    virtual void onHotsWeeklyList(const bilibili::HotsWeeklyListResult& result) {}
    virtual void onHotsWeeklyVideoList(const bilibili::HotsWeeklyVideoListResult& result, const std::string& label,
                                       const std::string& reminder) {}
    virtual void onError(const std::string& error) {}

    void requestData();

    /// 获取 每周推荐 整体列表
    void requestHotsWeeklyList();

    /// 获取 每周推荐 某一周视频列表
    void requestHotsWeeklyVideoList(int number);

    /// 通过 weeklyList 列表项的序号请求周视频 index: [0, ]
    void requestHotsWeeklyVideoListByIndex(size_t index);

    std::vector<std::string> getWeeklyList();

protected:
    bilibili::HotsWeeklyListResult weeklyList;
};
