//
// Created by fang on 2022/9/13.
//

#pragma once

#include "bilibili.h"
#include "bilibili/result/home_live_result.h"
#include "presenter/presenter.h"

class LiveDataRequest : public Presenter {
public:
    virtual void onLiveData(const bilibili::LiveUrlResultWrapper& result) {}

    virtual void onError(const std::string& error) {}

    void requestData(int roomid) {
        bilibili::BilibiliClient::get_live_url(
            roomid, defaultQuality,
            [this](const bilibili::LiveUrlResultWrapper& result) {
                liveUrl = result;
                onLiveData(result);
            },
            [this](const std::string& error) { this->onError(error); });
    }

    static inline int defaultQuality = 10000;
    bilibili::LiveUrlResultWrapper liveUrl;
};