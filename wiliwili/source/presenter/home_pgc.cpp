//
// Created by fang on 2022/7/14.
//

#include "presenter/home_pgc.hpp"
#include "bilibili.h"

void HomeBangumiRequest::onBangumiList(
    const bilibili::PGCResultWrapper& result) {}

void HomeBangumiRequest::onError(const std::string& error) {}

void HomeBangumiRequest::requestData(bool refresh) {
    if (refresh) {
        this->next_cursor  = "0";
        this->refresh_flag = 0;
    } else {
        this->refresh_flag = 1;
    }

    this->requestBangumiList(refresh_flag, next_cursor);
}

void HomeBangumiRequest::requestBangumiList(int is_refresh,
                                            std::string cursor) {
    bilibili::BilibiliClient::get_bangumi(
        is_refresh, cursor,
        [this](const bilibili::PGCResultWrapper& result) {
            this->next_cursor = result.next_cursor;
            this->onBangumiList(result);
        },
        [this](const std::string& error) { this->onError(error); });
}

void HomeCinemaRequest::onCinemaList(const bilibili::PGCResultWrapper& result) {
}

void HomeCinemaRequest::onError(const std::string& error) {}

void HomeCinemaRequest::requestData(bool refresh) {
    if (refresh) {
        this->next_cursor  = "0";
        this->refresh_flag = 0;
    } else {
        this->refresh_flag = 1;
    }

    this->requestCinemaList(refresh_flag, next_cursor);
}

void HomeCinemaRequest::requestCinemaList(int is_refresh, std::string cursor) {
    bilibili::BilibiliClient::get_cinema(
        is_refresh, cursor,
        [this](const bilibili::PGCResultWrapper& result) {
            this->next_cursor = result.next_cursor;
            this->onCinemaList(result);
        },
        [this](const std::string& error) { this->onError(error); });
}

DataSourcePGCVideoList::DataSourcePGCVideoList(bilibili::PGCModuleResult result)
    : videoList(result) {
    if (!result.url.empty()) {
        showMore = true;
    }
}

RecyclingGridItem* DataSourcePGCVideoList::cellForRow(RecyclingGrid* recycler,
                                                      size_t index) {
    if (index == videoList.items.size()) {
        // show more button
        RecyclingGridItemViewMoreCard* item =
            (RecyclingGridItemViewMoreCard*)recycler->dequeueReusableCell(
                "CellMore");
        return item;
    }

    RecyclingGridItemPGCVideoCard* item =
        (RecyclingGridItemPGCVideoCard*)recycler->dequeueReusableCell("Cell");

    bilibili::PGCItemResult& r = this->videoList.items[index];
    if (item->isVertical()) {
        item->setCard(r.cover + "@312w_420h_1c.jpg", r.title, r.desc,
                      r.badge_info, r.bottom_left_badge, r.bottom_right_badge);
    } else {
        item->setCard(r.cover + "@672w_378h_1c.jpg", r.title, r.desc,
                      r.badge_info, r.bottom_left_badge, r.bottom_right_badge);
    }
    return item;
}

size_t DataSourcePGCVideoList::getItemCount() {
    if (this->showMore) {
        return videoList.items.size() + 1;
    }
    return videoList.items.size();
}

void DataSourcePGCVideoList::onItemSelected(RecyclingGrid* recycler,
                                            size_t index) {
    if (index == videoList.items.size()) {
        if (this->videoList.module_id == 1741) {
            // 我的追番
        } else if (this->videoList.module_id == 1745) {
            // 我的追剧
        } else {
            brls::Application::pushActivity(
                new PGCIndexActivity(this->videoList.url));
        }
    } else {
        brls::Application::pushActivity(
            new PlayerSeasonActivity(videoList.items[index].season_id));
    }
}

void DataSourcePGCVideoList::appendData(
    const bilibili::PGCItemListResult& data) {
    this->videoList.items.insert(this->videoList.items.end(), data.begin(),
                                 data.end());
}

void DataSourcePGCVideoList::clearData() { this->videoList.items.clear(); }