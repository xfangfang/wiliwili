//
// Created by fang on 2022/8/2.
//

// register this fragment in main.cpp
//#include "fragment/search_tab.hpp"
//    brls::Application::registerXMLView("SearchTab", SearchTab::create);
// <brls:View xml=@res/xml/fragment/search_tab.xml

#pragma once

#include <borealis.hpp>

#include "bilibili.h"
#include "bilibili/result/search_result.h"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "activity/player_activity.hpp"

class SearchVideo;
class SearchBangumi;
class SearchCinema;
class SearchHots;
typedef brls::Event<std::string> UpdateSearchEvent;

class DataSourceSearchVideoList : public RecyclingGridDataSource {
public:
    DataSourceSearchVideoList(bilibili::VideoItemSearchListResult result)
        : list(result) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item =
            (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::VideoItemSearchResult& r = this->list[index];
        item->setCard(r.cover + "@672w_378h_1c.jpg", r.title, r.subtitle,
                      r.pubdate, r.play, r.danmaku, "");
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto video = list[index];
        if (!video.bvid.empty()) {
            brls::Application::pushActivity(
                new PlayerActivity(list[index].bvid));
        } else if (video.season_id != 0) {
            brls::Application::pushActivity(
                new PlayerSeasonActivity(list[index].season_id));
        }
    }

    void appendData(const bilibili::VideoItemSearchListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::VideoItemSearchListResult list;
};

class DataSourceSearchPGCList : public RecyclingGridDataSource {
public:
    DataSourceSearchPGCList(bilibili::VideoItemSearchListResult result)
        : list(result) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemSearchPGCVideoCard* item =
            (RecyclingGridItemSearchPGCVideoCard*)recycler->dequeueReusableCell(
                "Cell");

        bilibili::VideoItemSearchResult& r = this->list[index];

        std::string score = "", score_count = "", cv = "", subtitle = "";
        if (r.media_score.score > 0) {
            score_count = fmt::format("{}人评分", r.media_score.user_count);
            score       = fmt::format("{}分", r.media_score.score);
        }
        if (!r.cv.empty()) {
            cv = "演员: " + r.cv;
        }
        subtitle =
            fmt::format("{} · {}", r.styles, wiliwili::sec2TimeDate(r.pubdate));
        if (!r.index_show.empty()) subtitle += " · " + r.index_show;

        item->setCard(r.cover + "@312w_420h_1c.jpg", r.title, subtitle, cv,
                      "简介: " + r.desc, r.badge.text, r.badge.bg_color,
                      score_count, score, r.season_type_name, r.areas);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto video = list[index];
        brls::Application::pushActivity(
            new PlayerSeasonActivity(list[index].season_id));
    }

    void appendData(const bilibili::VideoItemSearchListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::VideoItemSearchListResult list;
};

class SearchTab : public brls::Box {
public:
    SearchTab();

    ~SearchTab();

    static View* create();

    void requestData(const std::string& key);

    inline static std::string keyWord = "";

    void passEventToSearchHots(UpdateSearchEvent* updateSearchEvent);

    void focusNthTab(int i);

private:
    BRLS_BIND(SearchVideo, searchVideoTab, "search/tab/video");
    BRLS_BIND(SearchBangumi, searchBangumiTab, "search/tab/bangumi");
    BRLS_BIND(SearchCinema, searchCinemaTab, "search/tab/cinema");
    BRLS_BIND(SearchHots, searchHotsTab, "search/tab/hots");
    BRLS_BIND(AutoTabFrame, tabFrame, "search/tab/frame");
};