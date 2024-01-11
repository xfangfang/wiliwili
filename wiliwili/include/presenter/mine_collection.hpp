//
// Created by fang on 2022/7/30.
//

#pragma once

#include "bilibili/result/mine_collection_result.h"
#include "presenter.h"

class MineCollectionRequest : public Presenter {
public:
    MineCollectionRequest();

    virtual void onCollectionList(const bilibili::CollectionListResultWrapper& result);

    virtual void onError(const std::string& error);

    /**
     * 设置请求类型
     * @param type 1: 我的收藏 2: 我的订阅
     */
    void setRequestType(int type);

    int getRequestType();

    void requestData(bool refresh = false);

    void requestCollectionList(std::string& mid, int index = 1, int num = 20);

private:
    size_t index    = 1;
    bool hasMore    = true;
    int requestType = 1;
};