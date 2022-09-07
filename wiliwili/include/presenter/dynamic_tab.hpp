//
// Created by fang on 2022/8/19.
//

#pragma once

#include "bilibili/result/dynamic_video.h"

class DynamicTabRequest {
public:
    virtual void onUpList(const bilibili::DynamicUpListResultWrapper &result);

    virtual void onError(const std::string& error);

    void requestData();

    void requestUpList();

};