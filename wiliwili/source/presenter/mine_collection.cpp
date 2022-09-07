//
// Created by fang on 2022/7/30.
//


#include "presenter/mine_collection.hpp"
#include "utils/config_helper.hpp"
#include "borealis/core/logger.hpp"
#include "bilibili.h"


MineCollectionRequest::MineCollectionRequest(){
}

void MineCollectionRequest::onCollectionList(const bilibili::CollectionListResultWrapper &result) {}

void MineCollectionRequest::onError(const std::string& error) {}

void MineCollectionRequest::requestData(bool refresh) {
    if(refresh){
        index = 1;
        hasMore = true;
    }

    if(hasMore){
        auto userID = ProgramConfig::instance().getUserID();
        this->requestCollectionList(userID, index);
    }
}

void MineCollectionRequest::requestCollectionList(std::string& mid, int i, int num) {
    bilibili::BilibiliClient::get_my_collection_list(mid, i, num,
            [this](const bilibili::CollectionListResultWrapper &result) {
                if(index != result.index){
                    brls::Logger::error("MineCollectionRequest::requestCollectionList: request: {}, got: {}",
                                        index, result.index);
                    return;
                }
                hasMore = result.has_more;
                index = result.index + 1;
                this->onCollectionList(result);
            }, [this](const std::string &error) {
                this->onError(error);
            });
}