//
// Created by fang on 2022/7/10.
//

#include "presenter/home_recommends.hpp"
#include "bilibili.h"

void Home::onRecommendVideoList(
    const bilibili::RecommendVideoListResultWrapper &result) {}
void Home::onError(const std::string &error) {}

void Home::requestData(bool refresh) {
    CHECK_REQUEST
    static int current_page = 1;
    if (refresh) {
        current_page = 1;
    }
    this->requestRecommendVideoList(current_page, 24);
    current_page++;
}

void Home::requestRecommendVideoList(int index, int num) {
    CHECK_AND_SET_REQUEST
    bilibili::BilibiliClient::get_recommend(
        index, num,
        [this](const bilibili::RecommendVideoListResultWrapper &result) {
            this->onRecommendVideoList(result);
            UNSET_REQUEST
        },
        [this](const std::string &error) {
            this->onError(error);
            UNSET_REQUEST
        });
}