//
// Created by fang on 2022/7/7.
//

#include <borealis/views/label.hpp>
#include "activity/player_activity.hpp"
#include "fragment/home_hots_weekly.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"

class DataSourceHotsWeeklyVideoList
        : public RecyclingGridDataSource
{
public:
    DataSourceHotsWeeklyVideoList(bilibili::HotsWeeklyVideoListResult result):videoList(result){

    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override{
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::HotsWeeklyVideoResult& r = this->videoList[index];
        item->setCard(r.pic+"@672w_378h_1c.jpg",r.title,r.owner.name,r.pubdate, r.stat.view, r.stat.danmaku, r.duration);
        item->setRCMDReason(r.rcmd_reason);
        return item;
    }

    size_t getItemCount() override{
        return videoList.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override{
        brls::Application::pushActivity(new PlayerActivity(videoList[index].bvid));
    }

    void appendData(const bilibili::HotsWeeklyVideoListResult& data){
        brls::Logger::error("DataSourceRecommendVideoList: append data: {}", data.size());
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override{
        this->videoList.clear();
    }

private:
    bilibili::HotsWeeklyVideoListResult videoList;
};

HomeHotsWeekly::HomeHotsWeekly() {
    this->inflateFromXMLRes("xml/fragment/home_hots_weekly.xml");
    brls::Logger::debug("Fragment HomeHotsWeekly: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    this->requestData();
}

void HomeHotsWeekly::onCreate() {
    this->registerTabAction("切换", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        static int selected = 1;
        brls::Application::pushActivity(new brls::Activity(new brls::Dropdown(
                "每周必看", this->getWeeklyList(),
                [this](int _selected) {
                    if(_selected == 0){
                        selected = 1;
                        this->requestData();
                    } else {
                        selected = _selected;
                        this->requestHotsWeeklyVideoListByIndex(selected - 1);
                    }
                },
                selected)));
        return true;
    });
}

brls::View* HomeHotsWeekly::create() {
    return new HomeHotsWeekly();
}

HomeHotsWeekly::~HomeHotsWeekly() {
    brls::Logger::debug("Fragment HomeHotsWeeklyActivity: delete");
}

void HomeHotsWeekly::onHotsWeeklyList(const bilibili::HotsWeeklyListResult &result) {

}

void HomeHotsWeekly::onHotsWeeklyVideoList(const bilibili::HotsWeeklyVideoListResult &result,
                           const string& label, const string& reminder) {
    brls::Threading::sync([this, result, label, reminder](){
        AutoTabFrame::focus2Sidebar(this);
       this->weekly_reminder->setText(reminder);
       this->weekly_label->setText(label);
       recyclingGrid->setDataSource(new DataSourceHotsWeeklyVideoList(result));
    });

}