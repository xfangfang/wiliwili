//
// Created by fang on 2022/7/6.
//

#pragma once

#include "bilibili/result/home_hots_weekly_result.h"

class HomeHotsHistoryRequest {
public:
    virtual void onHotsHistoryList(const bilibili::HotsHistoryVideoListResult &result, const string& explain);
    virtual void onError();

    void requestData();

    /// 获取入站必刷视频列表
    void requestHotsHistoryVideoList();

};
