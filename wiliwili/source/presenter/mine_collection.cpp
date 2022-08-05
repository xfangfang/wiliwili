//
// Created by fang on 2022/7/30.
//

#include "bilibili.h"
#include "presenter/mine_collection.hpp"
#include "utils/config_helper.hpp"


MineCollectionRequest::MineCollectionRequest(){
}

void MineCollectionRequest::onCollectionList(const bilibili::CollectionListResultWrapper &result) {}

void MineCollectionRequest::onError() {}

void MineCollectionRequest::requestData(bool refresh) {
    static int index = 1;
    if(refresh)
        index = 1;
    auto cookie = ProgramConfig::instance().getCookie();
    this->requestCollectionList(cookie["DedeUserID"], index++);
}

void MineCollectionRequest::requestCollectionList(std::string& mid, int index, int num) {
    bilibili::BilibiliClient::get_my_collection_list(mid, index, num,
            [this](const bilibili::CollectionListResultWrapper &result) {
                this->onCollectionList(result);
            }, [](const std::string &error) {
            });
}