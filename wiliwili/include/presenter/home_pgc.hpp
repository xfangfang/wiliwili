//
// Created by fang on 2022/7/14.
//

#pragma once

#include "bilibili/result/home_pgc_result.h"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "activity/pgc_index_activity.hpp"
#include "presenter.h"

class HomeBangumiRequest : public Presenter {
public:
    virtual void onBangumiList(const bilibili::PGCResultWrapper& result);

    virtual void onError(const std::string& error);

    void requestData(bool refresh = true);

    void requestBangumiList(int is_refresh = 0, const std::string& cursor = "0");

protected:
    std::string next_cursor;
    int refresh_flag = 0;
};

class HomeCinemaRequest : public Presenter {
public:
    virtual void onCinemaList(const bilibili::PGCResultWrapper& result);

    virtual void onError(const std::string& error);

    void requestData(bool refresh = true);

    void requestCinemaList(int is_refresh = 0, const std::string& cursor = "0");

protected:
    std::string next_cursor;
    int refresh_flag = 0;
};

class DataSourcePGCVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourcePGCVideoList(const bilibili::PGCModuleResult& result);

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override;

    size_t getItemCount() override;

    void onItemSelected(RecyclingGrid* recycler, size_t index) override;

    void appendData(const bilibili::PGCItemListResult& data);

    void clearData() override;

private:
    bilibili::PGCModuleResult videoList;
    bool showMore = false;
};