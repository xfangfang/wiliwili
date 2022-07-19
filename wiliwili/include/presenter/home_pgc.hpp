//
// Created by fang on 2022/7/14.
//

#pragma once

#include "bilibili/result/home_pgc_result.h"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"

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
    DataSourcePGCVideoList(bilibili::PGCItemListResult result): videoList(result){

    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override{
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemPGCVideoCard* item = (RecyclingGridItemPGCVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::PGCItemResult& r = this->videoList[index];
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
        return videoList.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override{

    }

    void appendData(const bilibili::PGCItemListResult& data){
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override{
        this->videoList.clear();
    }

private:
    bilibili::PGCItemListResult videoList;
};