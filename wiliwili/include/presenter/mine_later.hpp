//
// Created by 贾海峰 on 2023/7/6.
//

#pragma once

#include "bilibili/result/mine_later_result.h"
#include "presenter.h"

class MineLaterRequest : public Presenter {
public:
    virtual void onWatchLaterList(const bilibili::WatchLaterListWrapper& result);

    virtual void onError(const std::string& error);

    void requestData(bool refresh = false);

    void requestWatchLaterList();
};