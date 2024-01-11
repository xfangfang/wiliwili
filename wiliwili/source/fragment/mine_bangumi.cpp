//
// Created by fang on 2023/1/18.
//

#include "fragment/mine_bangumi.hpp"

#include <utility>
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "utils/image_helper.hpp"
#include "utils/activity_helper.hpp"

using namespace brls::literals;

class DataSourceMineBangumiVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceMineBangumiVideoList(bilibili::PGCItemListResult result) : videoList(std::move(result)) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        RecyclingGridItemPGCVideoCard* item = (RecyclingGridItemPGCVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::PGCItemResult& r = this->videoList[index];

        item->setCard(r.cover + ImageHelper::v_ext, r.title, r.desc.empty() ? "尚未观看" : r.desc, r.badge_info, "",
                      r.bottom_right_badge);

        return item;
    }

    size_t getItemCount() override { return videoList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        Intent::openSeasonBySeasonId(videoList[index].season_id);
    }

    void appendData(const bilibili::PGCItemListResult& data) {
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override { this->videoList.clear(); }

private:
    bilibili::PGCItemListResult videoList;
};

MineBangumi::MineBangumi() {
    this->inflateFromXMLRes("xml/fragment/mine_bangumi.xml");
    brls::Logger::debug("Fragment MineBangumi: create");

    this->registerFloatXMLAttribute("type", [this](float value) {
        this->setRequestType(value);
        this->requestData(true);
    });

    this->registerFloatXMLAttribute("spanCount", [this](float value) { this->recyclingGrid->spanCount = value; });

    this->registerFloatXMLAttribute("itemSpace",
                                    [this](float value) { this->recyclingGrid->estimatedRowSpace = value; });

    this->registerFloatXMLAttribute("itemHeight",
                                    [this](float value) { this->recyclingGrid->estimatedRowHeight = value; });

    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemPGCVideoCard::create(true); });
    recyclingGrid->onNextPage([this]() { this->requestData(); });
}

MineBangumi::~MineBangumi() { brls::Logger::debug("Fragment MineBangumi: delete"); }

brls::View* MineBangumi::create() { return new MineBangumi(); }

void MineBangumi::onError(const std::string& error) { this->recyclingGrid->setError(error); }

void MineBangumi::onBangumiList(const bilibili::BangumiCollectionWrapper& result) {
    if (result.pn == 1) {
        // 第一页
        this->recyclingGrid->setDataSource(new DataSourceMineBangumiVideoList(result.list));
    } else {
        // 第N页
        auto* datasource = dynamic_cast<DataSourceMineBangumiVideoList*>(recyclingGrid->getDataSource());
        if (!datasource) return;
        if (!result.list.empty()) {
            datasource->appendData(result.list);
            recyclingGrid->notifyDataChanged();
        }
    }
}
void MineBangumi::onCreate() {
    recyclingGrid->setRefreshAction([this]() {
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        this->requestData(true);
    });
    this->registerTabAction(requestType == 1 ? "wiliwili/mine/refresh_anime"_i18n : "wiliwili/mine/refresh_series"_i18n,
                            brls::ControllerButton::BUTTON_X, [this](brls::View* view) -> bool {
                                this->recyclingGrid->refresh();
                                return true;
                            });
}
