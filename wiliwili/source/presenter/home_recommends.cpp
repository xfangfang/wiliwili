//
// Created by fang on 2022/7/10.
//

#include <borealis/core/style.hpp>
#include "presenter/home_recommends.hpp"
#include "bilibili.h"

void Home::onRecommendVideoList(const bilibili::RecommendVideoListResultWrapper &result) {}
void Home::onError(const std::string &error) {}

void Home::requestData(bool refresh, FeedType type) {
    CHECK_REQUEST
    if (refresh) {
        requestPage = 1;
    }

    // 手动刷新是 3，自动加载是 4，初始加载是 0
    int freshType = refresh ? 3 : (requestPage == 1 ? 0 : 4);

    // 官方规则：精选 10, 手动刷新 30，初始加载 10, 自动加载 15
    //    int num = 10;
    //    if (type == FeedType::V1) {
    //        num = refresh ? 30 : (current_page == 1 ? 10 : 15);
    //    }

    // wiliwili: 精选 10，其他 30
    int num = type == FeedType::V1 ? 30 : 10;

    this->requestRecommendVideoList(requestPage, num, freshType);
    requestPage++;
}

void Home::requestRecommendVideoList(int index, int num, int fresh, FeedType type) {
    CHECK_AND_SET_REQUEST
    static int y_num = (int)brls::getStyle().getMetric("wiliwili/grid/span/4");
    BILI::get_recommend(
        index, num, 0, type == FeedType::V1 ? "V1" : "CLIENT_SELECTED", 3, y_num,
        [this](const bilibili::RecommendVideoListResultWrapper &result) {
            this->onRecommendVideoList(result);
            UNSET_REQUEST
        },
        [this](BILI_ERR) {
            this->onError(error);
            UNSET_REQUEST
        });
}