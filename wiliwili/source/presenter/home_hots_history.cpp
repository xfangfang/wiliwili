//
// Created by fang on 2022/7/10.
//

#include "bilibili.h"
#include "presenter/home_hots_history.hpp"

void HomeHotsHistoryRequest::onHotsHistoryList(const bilibili::HotsHistoryVideoListResult& result,
                                               const std::string& explain) {}
void HomeHotsHistoryRequest::onError(const std::string& error) {}

void HomeHotsHistoryRequest::requestData() {
    CHECK_REQUEST
    this->requestHotsHistoryVideoList();
}

void HomeHotsHistoryRequest::requestHotsHistoryVideoList() {
    CHECK_AND_SET_REQUEST
    BILI::get_hots_history(
        [this](const bilibili::HotsHistoryVideoListResult& result, const std::string& explain) {
            this->onHotsHistoryList(result, explain);
            UNSET_REQUEST
        },
        [this](BILI_ERR) {
            this->onError(error);
            UNSET_REQUEST
        });
}