//
// Created by fang on 2022/7/10.
//

#include "bilibili.h"
#include "presenter/home_hots_history.hpp"


void HomeHotsHistoryRequest::onHotsHistoryList(const bilibili::HotsHistoryVideoListResult &result, const string& explain){}
void HomeHotsHistoryRequest::onError(){}

void HomeHotsHistoryRequest::requestData() {
    this->requestHotsHistoryVideoList();
}

void HomeHotsHistoryRequest::requestHotsHistoryVideoList() {
    bilibili::BilibiliClient::get_hots_history(
            [this](const bilibili::HotsHistoryVideoListResult& result, const string& explain){
                this->onHotsHistoryList(result, explain);
            }, [](const std::string &error) {
            });
}