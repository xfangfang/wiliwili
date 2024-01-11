//
// Created by fang on 2022/7/7.
//

#include <utility>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>

#include "fragment/home_hots_rank.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "view/grid_dropdown.hpp"
#include "view/svg_image.hpp"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"

class DataSourceHotsRankVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceHotsRankVideoList(bilibili::HotsRankVideoListResult result) : videoList(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemRankVideoCard* item = (RecyclingGridItemRankVideoCard*)recycler->dequeueReusableCell("Cell");

        auto r = this->videoList[index];
        brls::Logger::debug("title: {}", r.title);
        item->setCard(r.pic + ImageHelper::h_ext, r.title, r.owner.name, r.pubdate, r.stat.view, r.stat.danmaku,
                      r.duration, index + 1);
        return item;
    }

    size_t getItemCount() override { return videoList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override { Intent::openBV(videoList[index].bvid); }

    void appendData(const bilibili::HotsRankVideoListResult& data) {
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override { this->videoList.clear(); }

private:
    bilibili::HotsRankVideoListResult videoList;
};

class DataSourceHotsRankPGCVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceHotsRankPGCVideoList(bilibili::HotsRankPGCVideoListResult result)
        : videoList(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemRankVideoCard* item =
            (RecyclingGridItemRankVideoCard*)recycler->dequeueReusableCell("CellPGC");

        auto r = this->videoList[index];
        brls::Logger::debug("title: {}", r.title);
        item->setCard(r.ss_horizontal_cover + ImageHelper::h_ext, r.title, r.new_ep.index_show, 0, r.stat.view,
                      r.stat.danmaku, 0, index + 1);
        return item;
    }

    size_t getItemCount() override { return videoList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        Intent::openSeasonBySeasonId(videoList[index].season_id);
    }

    void appendData(const bilibili::HotsRankPGCVideoListResult& data) {
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override { this->videoList.clear(); }

private:
    bilibili::HotsRankPGCVideoListResult videoList;
};

HomeHotsRank::HomeHotsRank() {
    this->inflateFromXMLRes("xml/fragment/home_hots_rank.xml");
    brls::Logger::debug("Fragment HomeHotsRank: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemRankVideoCard::create(); });
    recyclingGrid->registerCell(
        "CellPGC", []() { return RecyclingGridItemRankVideoCard::create("xml/views/video_card_rank_pgc.xml"); });
    recyclingGrid->setRefreshAction([this]() {
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        this->requestData(currentChannel);
    });
    this->requestData();
}

void HomeHotsRank::onCreate() {
    this->registerTabAction("切换", brls::ControllerButton::BUTTON_X, [this](brls::View* view) -> bool {
        this->switchChannel();
        return true;
    });

    this->rank_box->addGestureRecognizer(new brls::TapGestureRecognizer(this->rank_box, [this]() {
        this->switchChannel();
        return true;
    }));

    if (brls::Application::ORIGINAL_WINDOW_HEIGHT < 720) {
        this->rank_note->setVisibility(brls::Visibility::GONE);
        this->rank_note_icon->setVisibility(brls::Visibility::GONE);
    }
}

void HomeHotsRank::switchChannel() {
    AutoTabFrame::focus2Sidebar(this);
    BaseDropdown::text(
        "排行榜", this->getRankList(),
        [this](int selected) {
            this->currentChannel = selected;
            this->rank_label->setText("榜单：" + this->getRankList()[selected]);
            this->recyclingGrid->refresh();
        },
        currentChannel);
}

void HomeHotsRank::onHotsRankList(const bilibili::HotsRankVideoListResult& result, const std::string& note) {
    brls::Threading::sync([this, result, note]() {
        this->rank_note->setText(note);
        recyclingGrid->estimatedRowHeight = 257.5;
        recyclingGrid->setDataSource(new DataSourceHotsRankVideoList(result));
    });
}

void HomeHotsRank::onHotsRankPGCList(const bilibili::HotsRankPGCVideoListResult& result, const std::string& note) {
    brls::Threading::sync([this, result, note]() {
        this->rank_note->setText(note);
        recyclingGrid->estimatedRowHeight = 220;
        recyclingGrid->setDataSource(new DataSourceHotsRankPGCVideoList(result));
    });
}

brls::View* HomeHotsRank::hitTest(brls::Point point) {
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

HomeHotsRank::~HomeHotsRank() { brls::Logger::debug("Fragment HomeHotsRankActivity: delete"); }

void HomeHotsRank::onError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}