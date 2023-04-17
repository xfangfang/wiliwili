//
// Created by fang on 2022/7/12.
//

#include <borealis.hpp>
#include <utility>
#include "fragment/home_live.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "utils/image_helper.hpp"
#include "utils/activity_helper.hpp"

using namespace brls::literals;

class DataSourceLiveVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceLiveVideoList(bilibili::LiveVideoListResult result)
        : videoList(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemLiveVideoCard* item =
            (RecyclingGridItemLiveVideoCard*)recycler->dequeueReusableCell(
                "Cell");

        bilibili::LiveVideoResult& r = this->videoList[index];
        item->setCard(r.cover + ImageHelper::h_ext, r.title, r.uname,
                      r.area_name, r.online, r.following);
        return item;
    }

    size_t getItemCount() override { return videoList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        Intent::openLive(videoList[index].roomid, videoList[index].title,
                         videoList[index].watched_show.text_large);
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
        auto* datasource = dynamic_cast<DataSourceLiveVideoList*>(
            recyclingGrid->getDataSource());
        if (datasource && index != 1) {
            datasource->appendData(result);
            recyclingGrid->notifyDataChanged();
        } else {
            recyclingGrid->setDataSource(new DataSourceLiveVideoList(result));
        }
    });
}

void HomeLive::onCreate() {
    this->live_box->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->live_box, [this]() {
            this->switchChannel();
            return true;
        }));
    this->registerTabAction("wiliwili/home/common/switch"_i18n,
                            brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                this->switchChannel();
                                return true;
                            });
}

void HomeLive::switchChannel() {
    static int selected = 0;
    AutoTabFrame::focus2Sidebar(this);
    brls::Application::pushActivity(new brls::Activity(new brls::Dropdown(
        "wiliwili/home/live/dialog"_i18n, this->getAreaList(),
        [this](int _selected) {
            this->recyclingGrid->showSkeleton();
            selected = _selected;
            this->requestData(selected);
            auto list = this->getAreaList();
            if (_selected == 0 && list.size() <= 1) {
                // 暂未加载出分区列表
                this->live_label->setText("wiliwili/home/live/default"_i18n);
            } else
                this->live_label->setText("wiliwili/home/common/part"_i18n +
                                          this->getAreaList()[_selected]);
        },
        selected)));
}

void HomeLive::onError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}

HomeLive::~HomeLive() {
    brls::Logger::debug("Fragment HomeLiveActivity: delete");
}

brls::View* HomeLive::create() { return new HomeLive(); }