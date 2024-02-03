//
// Created by fang on 2022/8/2.
//

// register this fragment in main.cpp
//#include "fragment/search_tab.hpp"
//    brls::Application::registerXMLView("SearchTab", SearchTab::create);
// <brls:View xml=@res/xml/fragment/search_tab.xml

#pragma once

#include <utility>

#include "bilibili.h"
#include "bilibili/result/search_result.h"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"
#include "utils/number_helper.hpp"

class SearchOrder;
class SearchBangumi;
class SearchCinema;
class SearchHots;
class SearchHistory;
class AutoTabFrame;
typedef brls::Event<std::string> UpdateSearchEvent;

class DataSourceSearchVideoList : public RecyclingGridDataSource {
public:
    DataSourceSearchVideoList(bilibili::VideoItemSearchListResult result) : list(std::move(result)) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::VideoItemSearchResult& r = this->list[index];
        item->setCard(r.cover + ImageHelper::h_ext, r.title, r.subtitle, r.pubdate, r.play, r.danmaku, "");
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto video = list[index];
        if (!video.bvid.empty()) {
            Intent::openBV(list[index].bvid);
        } else if (video.season_id != 0) {
            Intent::openSeasonBySeasonId(list[index].season_id);
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
    DataSourceSearchPGCList(bilibili::VideoItemSearchListResult result) : list(std::move(result)) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemSearchPGCVideoCard* item =
            (RecyclingGridItemSearchPGCVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::VideoItemSearchResult& r = this->list[index];

        std::string score, score_count, cv, subtitle;
        if (r.media_score.score > 0) {
            score_count = fmt::format("{}人评分", wiliwili::num2w(r.media_score.user_count));
            score       = fmt::format("{}分", r.media_score.score);
        }
        if (!r.cv.empty()) {
            cv = "演员: " + r.cv;
        }

        std::vector<std::string> subtitles;
        if (!r.styles.empty()) subtitles.emplace_back(r.styles);
        if (r.pubdate > 0) subtitles.emplace_back(wiliwili::sec2dateV2(r.pubdate));
        if (!r.index_show.empty()) subtitles.emplace_back(r.index_show);
        subtitle = pystring::join(" · ", subtitles);

        item->setCard(r.cover + ImageHelper::v_ext, r.title, subtitle, cv, "简介: " + r.desc, r.badge.text,
                      r.badge.bg_color, score_count, score, r.season_type_name, r.areas);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto video = list[index];
        Intent::openSeasonBySeasonId(list[index].season_id);
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

    ~SearchTab() override;

    static View* create();

    void focusNthTab(int i);

    SearchHistory* getSearchHistoryTab();

    SearchHots* getSearchHotsTab();

    SearchOrder* getSearchVideoTab();

    SearchBangumi* getSearchBangumiTab();

    SearchCinema* getSearchCinemaTab();

private:
    BRLS_BIND(SearchOrder, searchVideoTab, "search/tab/video");
    BRLS_BIND(SearchBangumi, searchBangumiTab, "search/tab/bangumi");
    BRLS_BIND(SearchCinema, searchCinemaTab, "search/tab/cinema");
    BRLS_BIND(SearchHots, searchHotsTab, "search/tab/hots");
    BRLS_BIND(SearchHistory, searchHistoryTab, "search/tab/history");
    BRLS_BIND(AutoTabFrame, tabFrame, "search/tab/frame");
};