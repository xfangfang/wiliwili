//
// Created by fang on 2022/7/10.
//

#include "bilibili.h"
#include "bilibili/result/home_hots_all_result.h"
#include "presenter/home_hots_all.hpp"

void HomeHotsAllRequest::requestData(bool refresh) {
    CHECK_REQUEST
    if (refresh) {
        requestPage = 1;
    }
    this->requestHotsAllVideoList(requestPage, 40);
    requestPage++;
}

void HomeHotsAllRequest::requestHotsAllVideoList(int index, int num) {
    CHECK_AND_SET_REQUEST
    BILI::get_hots_all(
        index, num,
        [this, index](const bilibili::HotsAllVideoListResult &result, bool no_more) {
            if (no_more) {
            } else {
                this->onHotsAllVideoList(result, index);
            }
            UNSET_REQUEST
        },
        [this](BILI_ERR) {
            this->onError(error);
            UNSET_REQUEST
        });
}
