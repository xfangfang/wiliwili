//
// Created by fang on 2023/1/18.
//

#pragma once

#include "presenter/presenter.h"
#include "bilibili/result/mine_bangumi_result.h"

class MineBangumiRequest : public Presenter {
public:
    virtual void onBangumiList(const bilibili::BangumiCollectionWrapper& result) {}
    virtual void onError(const std::string& error) {}

    MineBangumiRequest();

    /**
     * 设置请求类型
     * @param type 1: 追番 2: 追剧
     */
    void setRequestType(size_t type);

    void requestData(bool refresh = false);

    void requestBangumiVideoList();

protected:
    bilibili::BangumiCollectionWrapper bangumiCollection;
    size_t requestType;
    bool requestEnd;
};