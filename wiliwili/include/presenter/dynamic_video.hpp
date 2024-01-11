//
// Created by fang on 2022/8/18.
//

#pragma once

#include "bilibili/result/dynamic_video.h"
#include "presenter.h"

class DynamicVideoRequest : public Presenter {
public:
    virtual void onDynamicVideoList(const bilibili::DynamicVideoListResult& result, unsigned int index);

    virtual void onError(const std::string& error);

    void setCurrentUser(int64_t mid);

    void requestData(bool refresh = false);

    void requestDynamicVideoList(unsigned int page = 1, const std::string& offset = "");

    void requestUserDynamicVideoList(int64_t mid, int pn = 0, int ps = 30);

protected:
    int64_t currentUser       = 0;
    unsigned int currentPage  = 1;
    std::string currentOffset = "";
};