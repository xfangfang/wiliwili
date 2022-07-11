//
// Created by fang on 2022/5/26.
//

#pragma once

#include "bilibili/result/home_result.h"

class Home {
public:
    virtual void onRecommendVideoList(const bilibili::RecommendVideoListResult &result, int index);
    virtual void onError();

    void requestData(bool refresh = false);

    void requestRecommendVideoList(int index = 1, int num = 24);
};
