//
// Created by fang on 2022/8/19.
//

#pragma once

#include "bilibili/result/dynamic_video.h"
#include "presenter.h"

class DynamicTabRequest : public Presenter {
public:
    virtual void onUpList(const bilibili::DynamicUpListResultWrapper& result);

    virtual void onError(const std::string& error);

    void requestData();

    void requestUpList();
};