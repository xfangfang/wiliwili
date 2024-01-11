//
// Created by fang on 2022/7/6.
//

#include <utility>
#include <borealis/core/thread.hpp>

#include "fragment/home_hots_all.hpp"
#include "view/video_card.hpp"
#include "view/recycling_grid.hpp"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"

using namespace brls::literals;

class DataSourceHotsAllVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceHotsAllVideoList(bilibili::HotsAllVideoListResult result) : videoList(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::HotsAllVideoResult& r = this->videoList[index];
        brls::Logger::debug("title: {}", r.title);
        item->setCard(r.pic + ImageHelper::h_ext, r.title, r.owner.name, r.pubdate, r.stat.view, r.stat.danmaku,
                      r.duration);
        return item;
    }

    size_t getItemCount() override { return videoList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override { Intent::openBV(videoList[index].bvid); }

    void appendData(const bilibili::HotsAllVideoListResult& data) {
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override { this->videoList.clear(); }

private:
    bilibili::HotsAllVideoListResult videoList;
};

HomeHotsAll::HomeHotsAll() {
    this->inflateFromXMLRes("xml/fragment/home_hots_all.xml");
    brls::Logger::debug("Fragment HomeHotsAll: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });

    recyclingGrid->onNextPage([this]() { this->requestData(); });
    recyclingGrid->setRefreshAction([this]() {
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        this->requestData(true);
    });

    this->requestData();
}

void HomeHotsAll::onHotsAllVideoList(const bilibili::HotsAllVideoListResult& result, int index) {
    brls::Threading::sync([this, index, result]() {
        auto* datasource = dynamic_cast<DataSourceHotsAllVideoList*>(recyclingGrid->getDataSource());
        if (datasource && index != 1) {
            if (!result.empty()) {
                datasource->appendData(result);
                recyclingGrid->notifyDataChanged();
            }
        } else {
            recyclingGrid->setDataSource(new DataSourceHotsAllVideoList(result));
        }
    });
}

void HomeHotsAll::onCreate() {
    this->registerTabAction("wiliwili/home/common/refresh"_i18n, brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                this->recyclingGrid->refresh();
                                return true;
                            });
}

void HomeHotsAll::onError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}

brls::View* HomeHotsAll::create() { return new HomeHotsAll(); }

HomeHotsAll::~HomeHotsAll() { brls::Logger::debug("Fragment HomeHotsAllActivity: delete"); }