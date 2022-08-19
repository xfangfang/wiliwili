//
// Created by fang on 2022/8/18.
//

#include "fragment/dynamic_video.hpp"
#include "activity/player_activity.hpp"
#include "utils/number_helper.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"

class DataSourceDynamicVideoList
        : public RecyclingGridDataSource
{
public:
    DataSourceDynamicVideoList(bilibili::DynamicVideoListResult result):list(result){

    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index){
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        auto& r = this->list[index];
        item->setCard(r.pic+"@672w_378h_1c.jpg",r.title,r.owner.name,r.pubdate, r.stat.view, r.stat.danmaku, r.duration);
        return item;
    }

    size_t getItemCount() {
        return list.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) {
        brls::Application::pushActivity(new PlayerActivity(list[index].bvid));
    }

    void appendData(const bilibili::DynamicVideoListResult& data){
        bool skip = false;
        for(auto i: data){
            skip = false;
            for(auto j: this->list){
                if(j.aid == i.aid){
                    skip = true;
                    break;
                }
            }
            if(!skip){
                this->list.push_back(i);
            }
        }
    }

private:
    bilibili::DynamicVideoListResult list;
};


DynamicVideo::DynamicVideo() {
    this->inflateFromXMLRes("xml/fragment/dynamic_video.xml");
    brls::Logger::debug("Fragment DynamicVideo: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    recyclingGrid->onNextPage([this](){
        this->requestData();
    });
    this->requestData();
    this->registerAction("刷新", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        this->requestData(true);
        return true;
    });
}

DynamicVideo::~DynamicVideo() {
    brls::Logger::debug("Fragment DynamicVideoActivity: delete");
}

brls::View *DynamicVideo::create() {
    return new DynamicVideo();
}

void DynamicVideo::onCreate(){
    this->registerTabAction("刷新", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        this->requestData(true);
        return true;
    });
}

void DynamicVideo::changeUser(uint mid){
    this->setCurrentUser(mid);
    this->requestData(true);
}

void DynamicVideo::onDynamicVideoList(const bilibili::DynamicVideoListResult &result, uint index){
    brls::Threading::sync([this, result, index]() {
        DataSourceDynamicVideoList* datasource = (DataSourceDynamicVideoList *)recyclingGrid->getDataSource();
        if(datasource && index != 1){
            datasource->appendData(result);
            recyclingGrid->notifyDataChanged();
        } else{
            recyclingGrid->setDataSource(new DataSourceDynamicVideoList(result));
            brls::sync([this](){
                brls::Application::giveFocus(this);
            });
        }
    });
}

void DynamicVideo::onError(const std::string& error){

}