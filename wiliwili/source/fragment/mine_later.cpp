//
// Created by 贾海峰 on 2023/7/7.
//

#include <utility>
#include <borealis/core/thread.hpp>

#include "fragment/mine_later.hpp"
#include "view/video_card.hpp"
#include "utils/number_helper.hpp"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"

using namespace brls::literals;

class DataSourceMineWatchLaterList : public RecyclingGridDataSource {
public:
    explicit DataSourceMineWatchLaterList(bilibili::WatchLaterList result) : list(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");
        bilibili::WatchLaterItem& r      = this->list[index];
        item->setCard(r.pic, r.title, r.owner.name, 0, r.stat.view, r.stat.danmaku, r.duration);  //todo
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto& data = list[index];

        Intent::openBV(data.bvid);
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::WatchLaterList list;
};

MineLater::MineLater() {
    this->inflateFromXMLRes("xml/fragment/mine_later.xml");
    brls::Logger::debug("frag mine later created");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    recyclingGrid->setRefreshAction([this]() {
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        this->requestData(true);
    });
    this->requestData();
}

MineLater::~MineLater() { brls::Logger::debug("frag mine later deleted"); }

brls::View* MineLater::create() { return new MineLater(); }

void MineLater::onCreate() {
    this->registerTabAction("wiliwili/mine/refresh_later"_i18n, brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                this->recyclingGrid->refresh();
                                return true;
                            });
}

void MineLater::onWatchLaterList(const bilibili::WatchLaterListWrapper& result) {
    brls::Threading::sync(
        [this, result]() { recyclingGrid->setDataSource(new DataSourceMineWatchLaterList(result.list)); });
}

void MineLater::onError(const std::string& error) {
    brls::Logger::error("MineLater::onError: {}", error);
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}