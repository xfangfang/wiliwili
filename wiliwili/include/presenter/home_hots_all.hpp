//
// Created by fang on 2022/7/6.
//

#pragma once

#include "bilibili/result/home_hots_all_result.h"
#include "presenter.h"

class HomeHotsAllRequest : public Presenter {
public:
    HomeHotsAllRequest() : requestPage(1) {}
    virtual void onHotsAllVideoList(const bilibili::HotsAllVideoListResult& result, int index) {}
    virtual void onError(const std::string& error) {}

    void requestData(bool refresh = false);

    void requestHotsAllVideoList(int index = 1, int num = 40);

private:
    int requestPage;
};
