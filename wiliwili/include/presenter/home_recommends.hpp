//
// Created by fang on 2022/5/26.
//

#pragma once

#include "bilibili/result/home_result.h"
#include "presenter.h"

class Home : public Presenter {
public:
    virtual void onRecommendVideoList(
        const bilibili::RecommendVideoListResultWrapper &result);
    virtual void onError(const std::string &error);

    void requestData(bool refresh = false);

    void requestRecommendVideoList(int index = 1, int num = 30);
};
