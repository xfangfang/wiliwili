//
// Created by fang on 2022/8/18.
//

#pragma once

#include "bilibili/result/dynamic_video.h"

class DynamicVideoRequest {
public:
    virtual void onDynamicVideoList(const bilibili::DynamicVideoListResult &result, uint index);

    virtual void onError(const std::string& error);

    void setCurrentUser(uint mid);

    void requestData(bool refresh = false);

    void requestDynamicVideoList(uint page=1, const std::string& offset="");

    void requestUserDynamicVideoList(int mid, int pn = 0, int ps = 30);

protected:
    uint currentUser = 0;
    uint currentPage = 1;
    std::string currentOffset = "";

};