//
// Created by fang on 2022/7/14.
//

#include "presenter/home_pgc.hpp"
#include "bilibili.h"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"

using namespace brls::literals;

void HomeBangumiRequest::onBangumiList(const bilibili::PGCResultWrapper& result) {}

void HomeBangumiRequest::onError(const std::string& error) {}

void HomeBangumiRequest::requestData(bool refresh) {
    CHECK_REQUEST
    if (refresh) {
        this->next_cursor  = "0";
        this->refresh_flag = 0;
    } else {
        this->refresh_flag = 1;
    }

    this->requestBangumiList(refresh_flag, next_cursor);
}

void HomeBangumiRequest::requestBangumiList(int is_refresh, const std::string& cursor) {
    CHECK_AND_SET_REQUEST
    BILI::get_bangumi(
        is_refresh, cursor,
        [this](const bilibili::PGCResultWrapper& result) {
            this->next_cursor = result.next_cursor;
            this->onBangumiList(result);
            UNSET_REQUEST
        },
        [this](BILI_ERR) {
            this->onError(error);
            UNSET_REQUEST
        });
}

void HomeCinemaRequest::onCinemaList(const bilibili::PGCResultWrapper& result) {}

void HomeCinemaRequest::onError(const std::string& error) {}

void HomeCinemaRequest::requestData(bool refresh) {
    CHECK_REQUEST
    if (refresh) {
        this->next_cursor  = "0";
        this->refresh_flag = 0;
    } else {
        this->refresh_flag = 1;
    }

    this->requestCinemaList(refresh_flag, next_cursor);
}

void HomeCinemaRequest::requestCinemaList(int is_refresh, const std::string& cursor) {
    CHECK_AND_SET_REQUEST
    BILI::get_cinema(
        is_refresh, cursor,
        [this](const bilibili::PGCResultWrapper& result) {
            this->next_cursor = result.next_cursor;
            this->onCinemaList(result);
            UNSET_REQUEST
        },
        [this](BILI_ERR) {
            this->onError(error);
            UNSET_REQUEST
        });
}

DataSourcePGCVideoList::DataSourcePGCVideoList(const bilibili::PGCModuleResult& result) : videoList(result) {
    if (!result.url.empty()) {
        showMore = true;
    }
}

RecyclingGridItem* DataSourcePGCVideoList::cellForRow(RecyclingGrid* recycler, size_t index) {
    if (index == videoList.items.size()) {
        // show more button
        RecyclingGridItemViewMoreCard* item = (RecyclingGridItemViewMoreCard*)recycler->dequeueReusableCell("CellMore");
        return item;
    }

    RecyclingGridItemPGCVideoCard* item = (RecyclingGridItemPGCVideoCard*)recycler->dequeueReusableCell("Cell");

    bilibili::PGCItemResult& r = this->videoList.items[index];
    if (item->isVertical()) {
        item->setCard(r.cover + ImageHelper::v_ext, r.title, r.desc, r.badge_info, r.bottom_left_badge,
                      r.bottom_right_badge);
    } else {
        item->setCard(r.cover + ImageHelper::h_ext, r.title, r.desc, r.badge_info, r.bottom_left_badge,
                      r.bottom_right_badge);
    }
    return item;
}

size_t DataSourcePGCVideoList::getItemCount() {
    if (this->showMore) {
        return videoList.items.size() + 1;
    }
    return videoList.items.size();
}

void DataSourcePGCVideoList::onItemSelected(RecyclingGrid* recycler, size_t index) {
    if (index == videoList.items.size()) {
        if (this->videoList.module_id == 1741) {
            // 我的追番
            brls::Application::pushActivity(
                new brls::Activity(brls::Box::createFromXMLResource("fragment/mine_bangumi_anime.xml")),
                brls::TransitionAnimation::NONE);
        } else if (this->videoList.module_id == 1745) {
            // 我的追剧
            brls::Application::pushActivity(
                new brls::Activity(brls::Box::createFromXMLResource("fragment/mine_bangumi_series.xml")),
                brls::TransitionAnimation::NONE);
        } else {
            Intent::openPgcFilter(this->videoList.url);
        }
    } else {
        Intent::openSeasonBySeasonId(videoList.items[index].season_id);
    }
}

void DataSourcePGCVideoList::appendData(const bilibili::PGCItemListResult& data) {
    this->videoList.items.insert(this->videoList.items.end(), data.begin(), data.end());
}

void DataSourcePGCVideoList::clearData() { this->videoList.items.clear(); }