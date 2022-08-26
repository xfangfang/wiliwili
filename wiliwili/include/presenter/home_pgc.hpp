//
// Created by fang on 2022/7/14.
//

#pragma once

#include "bilibili/result/home_pgc_result.h"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "activity/player_activity.hpp"
#include "activity/pgc_index_activity.hpp"

class HomeBangumiRequest {
public:
    virtual void onBangumiList(const bilibili::PGCModuleListResult &result, int has_next);

    virtual void onError();

    void requestData();

    void requestBangumiList(int is_refresh, int cursor);

};

class HomeCinemaRequest {
public:
    virtual void onCinemaList(const bilibili::PGCModuleListResult &result, int has_next);

    virtual void onError();

    void requestData();

    void requestCinemaList(int is_refresh, int cursor);

};



class DataSourcePGCVideoList
        : public RecyclingGridDataSource
{
public:
    DataSourcePGCVideoList(bilibili::PGCModuleResult result): videoList(result){
        if(!result.url.empty()){
            showMore = true;
        }
    }

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override{
        if(index == videoList.items.size()){
            // show more button
            RecyclingGridItemViewMoreCard* item = (RecyclingGridItemViewMoreCard*)recycler->dequeueReusableCell("CellMore");
            return item;
        }


        RecyclingGridItemPGCVideoCard* item = (RecyclingGridItemPGCVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::PGCItemResult& r = this->videoList.items[index];
        if(item->isVertical()){
            item->setCard(r.cover+"@312w_420h_1c.jpg",r.title, r.desc,
                          r.badge_info, r.bottom_left_badge, r.bottom_right_badge);
        }else{
            item->setCard(r.cover+"@672w_378h_1c.jpg",r.title, r.desc,
                          r.badge_info, r.bottom_left_badge, r.bottom_right_badge);
        }
        return item;
    }

    size_t getItemCount() override{
        if(this->showMore){
            return videoList.items.size() + 1;
        }
        return videoList.items.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override{
        if(index == videoList.items.size()){
            if(this->videoList.module_id == 1741){
                // 我的追番
            } else if(this->videoList.module_id == 1745){
                // 我的追剧
            } else {
                brls::Application::pushActivity(new PGCIndexActivity(this->videoList.url));
            }
        } else {
            brls::Application::pushActivity(new PlayerSeasonActivity(videoList.items[index].season_id));
        }

    }

    void appendData(const bilibili::PGCItemListResult& data){
        this->videoList.items.insert(this->videoList.items.end(), data.begin(), data.end());
    }

    void clearData() override{
        this->videoList.items.clear();
    }

private:
    bilibili::PGCModuleResult videoList;
    bool showMore = false;
};