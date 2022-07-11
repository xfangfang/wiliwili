//
// Created by fang on 2022/7/6.
//

#pragma once

#include "bilibili/result/home_hots_all_result.h"


class HomeHotsAllRequest {
public:
    virtual void onHotsAllVideoList(const bilibili::HotsAllVideoListResult &result, int index){}
    virtual void onError(){}

    void requestData(bool refresh = false);

    void requestHotsAllVideoList(int index = 1, int num = 40);
};
