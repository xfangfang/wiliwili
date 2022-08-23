//
// Created by fang on 2022/6/14.
//

#include "fragment/home_recommends.hpp"
#include "activity/player_activity.hpp"
#include "utils/number_helper.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"


/// DataSourceRecommendVideoList


class DataSourceRecommendVideoList
        : public RecyclingGridDataSource
{
public:
    DataSourceRecommendVideoList(bilibili::RecommendVideoListResult result):recommendList(result){

    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index){
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::RecommendVideoResult& r = this->recommendList[index];
        item->setCard(r.pic+"@672w_378h_1c.jpg",r.title,r.owner.name,r.pubdate, r.stat.view, r.stat.danmaku, r.duration);
        return item;
    }

    size_t getItemCount() {
        return recommendList.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) {
        brls::Application::pushActivity(new PlayerActivity(recommendList[index].bvid));
    }

    void appendData(const bilibili::RecommendVideoListResult& data){
        //todo: 研究一下多线程条件下的问题
        //todo: 性能更强地去重
        brls::Logger::debug("DataSourceRecommendVideoList: append data");
        bool skip = false;
        for(auto i: data){
            skip = false;
            for(auto j: this->recommendList){
                if(j.cid == i.cid){
                    skip = true;
                    break;
                }
            }
            if(!skip){
                this->recommendList.push_back(i);
            }
        }
    }

private:
    bilibili::RecommendVideoListResult recommendList;
};

/// HomeRecommends

HomeRecommends::HomeRecommends() {
    this->inflateFromXMLRes("xml/fragment/home_recommends.xml");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    recyclingGrid->onNextPage([this](){
        this->requestData();
    });
    this->requestData();
}

void HomeRecommends::onCreate(){

    this->registerTabAction("刷新", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        this->requestData(true);
        return true;
    });
}

void HomeRecommends::onRecommendVideoList(const bilibili::RecommendVideoListResult &result, int index) {
    brls::Threading::sync([this, result, index]() {
        DataSourceRecommendVideoList* datasource = (DataSourceRecommendVideoList *)recyclingGrid->getDataSource();
        if(datasource && index != 1){
            datasource->appendData(result);
            recyclingGrid->notifyDataChanged();
        } else{
            recyclingGrid->setDataSource(new DataSourceRecommendVideoList(result));
        }
    });
}

brls::View* HomeRecommends::create() {
    return new HomeRecommends();
}

HomeRecommends::~HomeRecommends() {

}

void HomeRecommends::onError(const std::string &error) {
    brls::sync([error](){
        auto dialog = new brls::Dialog(error);
        dialog->setCancelable(false);
        dialog->addButton("OK", [](){});
        dialog->open();
    });
}