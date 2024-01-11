//
// Created by fang on 2022/7/7.
//

#include <utility>
#include <borealis/views/label.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>

#include "fragment/home_hots_weekly.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "view/grid_dropdown.hpp"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"

using namespace brls::literals;

class DataSourceHotsWeeklyVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceHotsWeeklyVideoList(bilibili::HotsWeeklyVideoListResult result) : videoList(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::HotsWeeklyVideoResult& r = this->videoList[index];
        item->setCard(r.pic + ImageHelper::h_ext, r.title, r.owner.name, r.pubdate, r.stat.view, r.stat.danmaku,
                      r.duration);
        item->setRCMDReason(r.rcmd_reason);
        return item;
    }

    size_t getItemCount() override { return videoList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override { Intent::openBV(videoList[index].bvid); }

    void appendData(const bilibili::HotsWeeklyVideoListResult& data) {
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override { this->videoList.clear(); }

private:
    bilibili::HotsWeeklyVideoListResult videoList;
};

HomeHotsWeekly::HomeHotsWeekly() {
    this->inflateFromXMLRes("xml/fragment/home_hots_weekly.xml");
    brls::Logger::debug("Fragment HomeHotsWeekly: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    recyclingGrid->setRefreshAction([this]() {
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        if (this->weeklyList.empty()) {
            this->requestData();
        } else {
            this->requestHotsWeeklyVideoListByIndex(currentChannel);
        }
    });
    this->requestData();
}

void HomeHotsWeekly::onCreate() {
    this->registerTabAction("wiliwili/home/common/switch"_i18n, brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                this->switchChannel();
                                return true;
                            });

    this->weekly_box->addGestureRecognizer(new brls::TapGestureRecognizer(this->weekly_box, [this]() {
        this->switchChannel();
        return true;
    }));
}

void HomeHotsWeekly::switchChannel() {
    AutoTabFrame::focus2Sidebar(this);
    BaseDropdown::text(
        "wiliwili/home/hots/t3"_i18n, this->getWeeklyList(),
        [this](int selected) {
            currentChannel = selected;
            this->recyclingGrid->refresh();
        },
        currentChannel);
}

brls::View* HomeHotsWeekly::hitTest(brls::Point point) {
    // Check if can focus farther first
    if (alpha == 0.0f || getVisibility() != brls::Visibility::VISIBLE) return nullptr;

    // Check if touch fits in view frame
    brls::Rect area    = this->getFrame();
    brls::Rect topArea = brls::Rect(area.getMaxX() - 200, area.getMinY() - 62, 200, 62);
    if (area.pointInside(point) || topArea.pointInside(point)) {
        for (auto child = this->getChildren().rbegin(); child != this->getChildren().rend(); child++) {
            View* result = (*child)->hitTest(point);

            if (result) return result;
        }
        return this;
    }

    return nullptr;
}

brls::View* HomeHotsWeekly::create() { return new HomeHotsWeekly(); }

HomeHotsWeekly::~HomeHotsWeekly() { brls::Logger::debug("Fragment HomeHotsWeeklyActivity: delete"); }

void HomeHotsWeekly::onHotsWeeklyList(const bilibili::HotsWeeklyListResult& result) {}

void HomeHotsWeekly::onHotsWeeklyVideoList(const bilibili::HotsWeeklyVideoListResult& result, const std::string& label,
                                           const std::string& reminder) {
    brls::Threading::sync([this, result, label, reminder]() {
        this->weekly_reminder->setText(reminder);
        this->weekly_label->setText(label);
        recyclingGrid->setDataSource(new DataSourceHotsWeeklyVideoList(result));
    });
}

void HomeHotsWeekly::onError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}