//
// Created by fang on 2022/5/26.
//

#pragma once

#include "bilibili/result/home_result.h"
#include "presenter.h"

enum class FeedType { V1, CLIENT_SELECTED };
class Home : public Presenter {
public:
    Home() : requestPage(1) {}

    virtual void onRecommendVideoList(const bilibili::RecommendVideoListResultWrapper &result);
    virtual void onError(const std::string &error);

    void requestData(bool refresh = false, FeedType type = FeedType::V1);

    void requestRecommendVideoList(int index = 1, int num = 30, int fresh = 0, FeedType type = FeedType::V1);

private:
    int requestPage;
};
