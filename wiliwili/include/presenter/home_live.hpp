//
// Created by fang on 2022/7/13.
//

#pragma once

#include "bilibili/result/home_live_result.h"

class HomeLiveRequest {
public:
    virtual void onLiveList(const bilibili::LiveVideoListResult& result,
                            int index, bool no_more);

    virtual void onError(const std::string& error) = 0;

    void requestData(int area_index = -1);

    void requestLiveList(int parent_area, int area, int page);

    std::vector<std::string> getAreaList();

private:
    bilibili::LiveAreaListResult areaList;
};