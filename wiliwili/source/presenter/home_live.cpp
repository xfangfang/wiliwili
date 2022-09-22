//
// Created by fang on 2022/7/13.
//

#include "bilibili.h"
#include "presenter/home_live.hpp"
#include "bilibili/result/home_live_result.h"
#include "borealis/core/i18n.hpp"

using namespace brls::literals;

void HomeLiveRequest::onLiveList(const bilibili::LiveVideoListResult &result,
                                 int index, bool no_more) {}

void HomeLiveRequest::onError() { printf("error"); }

// area_index -1: 加载下一页，默认
//             0<=N<areaList.size(): 加载指定分区
void HomeLiveRequest::requestData(int area_index) {
    static int index        = 0;
    static int current_page = 1;
    if (area_index >= (int)this->areaList.size()) area_index = 0;

    int parent_area, area;
    if (this->areaList.empty()) {
        // 初次访问，请求推荐分区
        current_page = 1;
        parent_area  = 99999;
        area         = 99999;
    } else if (area_index < 0) {
        // 默认参数，加载当前分区的下一页
        auto live_area = this->areaList[index];
        parent_area    = live_area.area_v2_parent_id;
        area           = live_area.area_v2_id;
    } else {
        // 指定新的分区
        index          = area_index;
        current_page   = 1;
        auto live_area = this->areaList[index];
        parent_area    = live_area.area_v2_parent_id;
        area           = live_area.area_v2_id;
    }
    this->requestLiveList(parent_area, area, current_page);
    current_page++;
}

void HomeLiveRequest::requestLiveList(int parent_area, int area, int page) {
    bilibili::BilibiliClient::get_live_recommend(
        parent_area, area, page, "pc",
        [this, page](const bilibili::LiveResultWrapper &result) {
            if (result.live_list.size() > 0) this->areaList = result.live_list;
            bilibili::LiveVideoListResult res = result.my_list;
            res.insert(res.end(), result.card_list.begin(),
                       result.card_list.end());
            this->onLiveList(res, page, result.has_more);
        },
        [](const std::string &error) {});
}

std::vector<std::string> HomeLiveRequest::getAreaList() {
    std::vector<std::string> list;
    for (auto i : this->areaList) list.push_back(i.title);
    if (list.empty()) {
        list.push_back("wiliwili/home/common/refresh"_i18n);
    }
    return list;
}