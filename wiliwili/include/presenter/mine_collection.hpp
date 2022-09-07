//
// Created by fang on 2022/7/30.
//

#pragma once

#include "bilibili/result/mine_collection_result.h"

class MineCollectionRequest {
public:
    MineCollectionRequest();

    virtual void onCollectionList(
        const bilibili::CollectionListResultWrapper& result);

    virtual void onError(const std::string& error);

    void requestData(bool refresh = false);

    void requestCollectionList(std::string& mid, int index = 1, int num = 20);

private:
    int index    = 1;
    bool hasMore = true;
};