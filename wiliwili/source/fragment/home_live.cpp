//
// Created by fang on 2022/7/12.
//

#include <borealis.hpp>
#include "fragment/home_live.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "activity/live_player_activity.hpp"

class DataSourceLiveVideoList : public RecyclingGridDataSource {
public:
    DataSourceLiveVideoList(bilibili::LiveVideoListResult result)
        : videoList(result) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemLiveVideoCard* item =
            (RecyclingGridItemLiveVideoCard*)recycler->dequeueReusableCell(
                "Cell");

        bilibili::LiveVideoResult& r = this->videoList[index];
        item->setCard(r.cover + "@672w_378h_1c.jpg", r.title, r.uname,
                      r.area_name, r.online, r.following);
        return item;
    }

    size_t getItemCount() override { return videoList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        brls::Application::pushActivity(new LiveActivity(videoList[index]));
    }

    void appendData(const bilibili::LiveVideoListResult& data) {
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override { this->videoList.clear(); }

private:
    bilibili::LiveVideoListResult videoList;
};

HomeLive::HomeLive() {
    this->inflateFromXMLRes("xml/fragment/home_live.xml");
    brls::Logger::debug("Fragment HomeLive: create");
    recyclingGrid->registerCell(
        "Cell", []() { return RecyclingGridItemLiveVideoCard::create(); });
    recyclingGrid->onNextPage([this]() { this->requestData(); });
    this->requestData();
}

void HomeLive::onLiveList(const bilibili::LiveVideoListResult& result,
                          int index, bool no_more) {
    brls::Threading::sync([this, result, index]() {
        DataSourceLiveVideoList* datasource =
            (DataSourceLiveVideoList*)recyclingGrid->getDataSource();
        if (datasource && index != 1) {
            datasource->appendData(result);
            recyclingGrid->notifyDataChanged();
        } else {
            recyclingGrid->setDataSource(new DataSourceLiveVideoList(result));
        }
    });
}

void HomeLive::onCreate() {
    this->registerTabAction(
        "切换", brls::ControllerButton::BUTTON_X,
        [this](brls::View* view) -> bool {
            static int selected = 0;
            AutoTabFrame::focus2Sidebar(this);
            brls::Application::pushActivity(
                new brls::Activity(new brls::Dropdown(
                    "直播分区", this->getAreaList(),
                    [this](int _selected) {
                        this->recyclingGrid->showSkeleton();
                        selected = _selected;
                        this->requestData(selected);
                        auto list = this->getAreaList();
                        if (_selected == 0 && list.size() <= 1) {
                            // 暂未加载出分区列表
                            this->live_label->setText("分区：推荐");
                        } else
                            this->live_label->setText(
                                "分区：" + this->getAreaList()[_selected]);
                    },
                    selected)));
            return true;
        });
}

HomeLive::~HomeLive() {
    brls::Logger::debug("Fragment HomeLiveActivity: delete");
}

brls::View* HomeLive::create() { return new HomeLive(); }