//
// Created by fang on 2022/7/28.
//

#include "bilibili.h"
#include "presenter/mine_history.hpp"


void MineHistoryRequest::onHistoryList(const bilibili::HistoryVideoResultWrapper &result) {}

void MineHistoryRequest::onError() {}

void MineHistoryRequest::requestData(bool refresh) {
    if(refresh)
        this->cursor = bilibili::HistoryVideoListCursor();
    this->requestHistoryVideoList();
}

void MineHistoryRequest::requestHistoryVideoList() {
    bilibili::BilibiliClient::get_my_history(cursor, [this](const bilibili::HistoryVideoResultWrapper &result){
        this->onHistoryList(result);
        this->cursor = result.cursor;
    }, [](const std::string & error){});
}