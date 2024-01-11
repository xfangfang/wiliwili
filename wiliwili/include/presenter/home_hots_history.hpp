//
// Created by fang on 2022/7/6.
//

#pragma once

#include "bilibili/result/home_hots_history_result.h"
#include "presenter.h"

class HomeHotsHistoryRequest : public Presenter {
public:
    virtual void onHotsHistoryList(const bilibili::HotsHistoryVideoListResult& result, const std::string& explain);
    virtual void onError(const std::string& error);

    void requestData();

    /// 获取入站必刷视频列表
    void requestHotsHistoryVideoList();
};
