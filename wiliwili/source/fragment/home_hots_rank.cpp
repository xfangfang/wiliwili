//
// Created by fang on 2022/7/7.
//

#include <borealis.hpp>
#include "activity/player_activity.hpp"
#include "fragment/home_hots_rank.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "view/grid_dropdown.hpp"

class DataSourceHotsRankVideoList
        : public RecyclingGridDataSource
{
public:
    DataSourceHotsRankVideoList(bilibili::HotsRankVideoListResult result):videoList(result){
    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override{
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemRankVideoCard* item = (RecyclingGridItemRankVideoCard*)recycler->dequeueReusableCell("Cell");

        auto r = this->videoList[index];
        brls::Logger::debug("title: {}", r.title);
        item->setCard(r.pic+"@672w_378h_1c.jpg",r.title,r.owner.name,r.pubdate, r.stat.view, r.stat.danmaku, r.duration, index + 1);
        return item;
    }

    size_t getItemCount() override{
        return videoList.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override{
        brls::Application::pushActivity(new VideoDetailActivity(videoList[index].bvid));
    }

    void appendData(const bilibili::HotsRankVideoListResult& data){
        brls::Logger::error("DataSourceRecommendVideoList: append data: {}", data.size());
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override{
        this->videoList.clear();
    }

private:
    bilibili::HotsRankVideoListResult videoList;
};


class DataSourceHotsRankPGCVideoList
        : public RecyclingGridDataSource
{
public:
    DataSourceHotsRankPGCVideoList(bilibili::HotsRankPGCVideoListResult result):videoList(result){

    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override{
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemRankVideoCard* item = (RecyclingGridItemRankVideoCard*)recycler->dequeueReusableCell("CellPGC");

        auto r = this->videoList[index];
        brls::Logger::debug("title: {}", r.title);
        item->setCard(r.ss_horizontal_cover+"@672w_378h_1c.jpg", r.title,
                      r.new_ep.index_show, 0, r.stat.view, r.stat.danmaku, 0, index + 1);
        return item;
    }

    size_t getItemCount() override{
        return videoList.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override{
//        brls::Application::pushActivity(new VideoDetailActivity(videoList[index].bvid));
    }

    void appendData(const bilibili::HotsRankPGCVideoListResult& data){
        brls::Logger::error("DataSourceRecommendVideoList: append data: {}", data.size());
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override{
        this->videoList.clear();
    }

private:
    bilibili::HotsRankPGCVideoListResult videoList;
};


HomeHotsRank::HomeHotsRank() {
    this->inflateFromXMLRes("xml/fragment/home_hots_rank.xml");
    brls::Logger::debug("Fragment HomeHotsRank: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemRankVideoCard::create(); });
    recyclingGrid->registerCell("CellPGC", []() { return RecyclingGridItemRankVideoCard::create("xml/views/video_card_rank_pgc.xml"); });
    this->requestData();

}

void HomeHotsRank::onCreate() {
    this->registerTabAction("切换", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        static int selected = 0;
        brls::Application::pushActivity(new brls::Activity(new GridDropdown(
                "排行榜", this->getRankList(), [this](int _selected) {
                    selected = _selected;
                    this->rank_label->setText("榜单：" + this->getRankList()[_selected]);
                    this->requestData(_selected);
                },
                selected)));

        return true;
    });
}

void HomeHotsRank::onHotsRankList(const bilibili::HotsRankVideoListResult &result, const string& note){
    brls::Threading::sync([this, result, note](){
        AutoTabFrame::focus2Sidebar(this);
        this->rank_note->setText(note);
        recyclingGrid->estimatedRowHeight = 257.5;
        recyclingGrid->setDataSource(new DataSourceHotsRankVideoList(result));
    });
}

void HomeHotsRank::onHotsRankPGCList(const bilibili::HotsRankPGCVideoListResult &result, const string& note){
    brls::Threading::sync([this, result, note](){
        AutoTabFrame::focus2Sidebar(this);
        this->rank_note->setText(note);
        recyclingGrid->estimatedRowHeight = 220;
        recyclingGrid->setDataSource(new DataSourceHotsRankPGCVideoList(result));
    });
}

HomeHotsRank::~HomeHotsRank() {
    brls::Logger::debug("Fragment HomeHotsRankActivity: delete");
}