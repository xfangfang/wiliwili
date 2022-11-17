//
// Created by fang on 2022/7/28.
//

#include "bilibili.h"
#include "presenter/mine_history.hpp"

void MineHistoryRequest::onHistoryList(
    const bilibili::HistoryVideoResultWrapper &result) {}

void MineHistoryRequest::onError(const std::string &error) {}

void MineHistoryRequest::requestData(bool refresh) {
    CHECK_REQUEST
    if (refresh) this->cursor = bilibili::HistoryVideoListCursor();
    this->requestHistoryVideoList();
}

void MineHistoryRequest::requestHistoryVideoList() {
    CHECK_AND_SET_REQUEST
    bilibili::BilibiliClient::get_my_history(
        cursor,
        [this](const bilibili::HistoryVideoResultWrapper &result) {
            this->onHistoryList(result);
            this->cursor = result.cursor;
            UNSET_REQUEST
        },
        [this](const std::string &error) {
            this->onError(error);
            UNSET_REQUEST
        });
}