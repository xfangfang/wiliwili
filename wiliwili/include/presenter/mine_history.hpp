//
// Created by fang on 2022/7/28.
//

#pragma once

#include "bilibili/result/mine_history_result.h"
#include "presenter.h"

class MineHistoryRequest : public Presenter {
public:
    virtual void onHistoryList(const bilibili::HistoryVideoResultWrapper& result);

    virtual void onError(const std::string& error);

    void requestData(bool refresh = false);

    void requestHistoryVideoList();

protected:
    bilibili::HistoryVideoListCursor cursor;
};