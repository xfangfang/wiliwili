//
// Created by fang on 2022/7/6.
//

#include "activity/player_activity.hpp"
#include "fragment/home_hots_all.hpp"
#include "view/video_card.hpp"
#include "view/recycling_grid.hpp"


class DataSourceHotsAllVideoList
        : public RecyclingGridDataSource
{
public:
    DataSourceHotsAllVideoList(bilibili::HotsAllVideoListResult result):videoList(result){

    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override{
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::HotsAllVideoResult& r = this->videoList[index];
        brls::Logger::debug("title: {}", r.title);
        item->setCard(r.pic+"@672w_378h_1c.jpg",r.title,r.owner.name,r.pubdate, r.stat.view, r.stat.danmaku, r.duration);
        return item;
    }

    size_t getItemCount() override{
        return videoList.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override{
        brls::Application::pushActivity(new VideoDetailActivity(videoList[index].bvid));
    }

    void appendData(const bilibili::HotsAllVideoListResult& data){
        brls::Logger::error("DataSourceRecommendVideoList: append data: {}", data.size());
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override{
        this->videoList.clear();
    }

private:
    bilibili::HotsAllVideoListResult videoList;
};


HomeHotsAll::HomeHotsAll() {
    this->inflateFromXMLRes("xml/fragment/home_hots_all.xml");
    brls::Logger::debug("Fragment HomeHotsAll: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });

    recyclingGrid->onNextPage([this](){
        this->requestData();
    });

    this->requestData();
}

void HomeHotsAll::onHotsAllVideoList(const bilibili::HotsAllVideoListResult &result, int index) {
    brls::Threading::sync([this, index, result]() {
        DataSourceHotsAllVideoList* datasource = (DataSourceHotsAllVideoList *)recyclingGrid->getDataSource();
        if(datasource && index != 1){
            datasource->appendData(result);
            recyclingGrid->notifyDataChanged();
        } else{
            AutoTabFrame::focus2Sidebar(this);
            recyclingGrid->setDataSource(new DataSourceHotsAllVideoList(result));
        }
    });
}

void HomeHotsAll::onCreate() {
    this->registerTabAction("刷新", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        this->requestData(true);
        return true;
    });
}

brls::View* HomeHotsAll::create() {
    return new HomeHotsAll();
}


HomeHotsAll::~HomeHotsAll() {
    brls::Logger::debug("Fragment HomeHotsAllActivity: delete");
}