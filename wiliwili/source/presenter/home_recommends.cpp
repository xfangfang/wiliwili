//
// Created by fang on 2022/7/10.
//

#include "presenter/home_recommends.hpp"
#include "bilibili.h"

void Home::onRecommendVideoList(
    const bilibili::RecommendVideoListResult &result, int index) {}
void Home::onError(const std::string &error) {}

void Home::requestData(bool refresh) {
    static int current_page = 1;
    if (refresh) {
        current_page = 1;
    }
    this->requestRecommendVideoList(current_page, 24);
    current_page++;
}

void Home::requestRecommendVideoList(int index, int num) {
    bilibili::BilibiliClient::get_recommend(
        index, num,
        [this, index](const bilibili::RecommendVideoListResult &result) {
            this->onRecommendVideoList(result, index);
        },
        [this](const std::string &error) { this->onError(error); });
}