//
// Created by fang on 2022/7/13.
//

#include "bilibili.h"
#include "presenter/home_live.hpp"
#include "bilibili/result/home_live_result.h"
#include "borealis/core/i18n.hpp"

using namespace brls::literals;

void HomeLiveRequest::onLiveList(const bilibili::LiveVideoListResult &result, int index) {}
void HomeLiveRequest::onAreaList(const bilibili::LiveFullAreaListResult &result) {}

void HomeLiveRequest::requestData(int main, int sub, int page) {
    CHECK_AND_SET_REQUEST

    // 获取实际的请求信息
    if (main == -1) main = staticMain;
    if (sub == -1) sub = staticSub;
    if (page == -1) page = staticPage + 1;
    staticMain = main;
    staticSub  = sub;
    staticPage = page;

    if (main == 0 && sub == 0) {
        // 使用 V1 接口访问，同时获取全站推荐与关注的直播
        this->requestLiveList(main, sub, page);
    } else {
        // 使用 V2 接口访问，只访问指定分区的推荐
        this->requestLiveListV2(main, sub, page);
    }
}

void HomeLiveRequest::requestLiveList(int parent_area, int area, int page) {
    BILI::get_live_recommend(
        parent_area, area, page, "pc",
        [this, page](const auto &result) {
            UNSET_REQUEST
            bilibili::LiveVideoListResult res = result.my_list;
            res.insert(res.end(), result.card_list.begin(), result.card_list.end());
            if (res.empty()) staticPage--;
            this->onLiveList(res, page);
        },
        [this](BILI_ERR) {
            this->onError(error);
            staticPage--;
            UNSET_REQUEST
        });
}

void HomeLiveRequest::requestLiveListV2(int parent_area, int area, int page) {
    BILI::get_live_recommend_second(
        parent_area, area, page,
        [this, page](const auto &result) {
            UNSET_REQUEST
            if (result.list.empty()) staticPage--;
            this->onLiveList(result.list, page);
        },
        [this](BILI_ERR) {
            this->onError(error);
            staticPage--;
            UNSET_REQUEST
        });
}

void HomeLiveRequest::requestAreaList() {
    BILI::get_live_area_list(
        [this](const auto &result) {
            this->fullAreaList = result.list;
            this->onAreaList(result.list);
        },
        [](BILI_ERR) { brls::Logger::error("area list: {}", error); });
}