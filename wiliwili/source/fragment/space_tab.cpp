//
// Created by fang on 2022/6/14.
//

#include <utility>
#include <borealis/core/thread.hpp>

#include "fragment/space_tab.hpp"
#include "utils/number_helper.hpp"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "view/mpv_core.hpp"

using namespace brls::literals;

/// SpaceVideoCard
//class SpaceVideoCard : public BaseVideoCard {
//public:
//    SpaceVideoCard() {
//        this->inflateFromXMLRes("xml/views/video_card.xml");
//    }
//
//    ~SpaceVideoCard() override;
//
//    void setCard(std::string pic, std::string title, std::string username,
//                 int pubdate = 0, int view_count = 0, int danmaku = 0,
//                 std::string rightBottomBadge = "", std::string extra = "");
//
//    static SpaceVideoCard* create();
//
//private:
//    BRLS_BIND(TextBox, labelTitle, "video/card/label/title");
//    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
//    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
//    BRLS_BIND(brls::Label, labelDanmaku, "video/card/label/danmaku");
//    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
//    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
//    BRLS_BIND(brls::Box, boxHint, "video/card/hint");
//    BRLS_BIND(brls::Label, labelHint, "video/card/label/hint");
//    BRLS_BIND(SVGImage, svgUp, "video/svg/up");
//    BRLS_BIND(brls::Box, boxRCMD, "video/card/rcmd_box");
//    BRLS_BIND(brls::Label, labelRCMD, "video/card/label/rcmd");
//    BRLS_BIND(brls::Box, boxAchievement, "video/card/achievement_box");
//    BRLS_BIND(brls::Label, labelAchievement, "video/card/label/achievement");
//};

/// DataSourceSpaceVideoList

class DataSourceSpaceVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceSpaceVideoList(bilibili::RecommendVideoListResult result) : recommendList(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::RecommendVideoResult& r = this->recommendList[index];
        item->setHideHighlightBorder(true);
        item->setHideHighlightBackground(true);
        item->setCard(r.pic + ImageHelper::h_ext, r.title, r.owner.name, r.pubdate, r.stat.view, r.stat.danmaku,
                      r.duration, r.rcmd_reason.content);
        return item;
    }

    size_t getItemCount() override { return recommendList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override { Intent::openBV(recommendList[index].bvid); }

    void appendData(const bilibili::RecommendVideoListResult& data) {
        //todo: 研究一下多线程条件下的问题
        //todo: 性能更强地去重
        brls::Logger::debug("DataSourceRecommendVideoList: append data");
        bool skip = false;
        for (const auto& i : data) {
            skip = false;
            for (const auto& j : this->recommendList) {
                if (j.cid == i.cid) {
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                this->recommendList.push_back(i);
            }
        }
    }

    void clearData() override { this->recommendList.clear(); }

private:
    bilibili::RecommendVideoListResult recommendList;
};

/// HomeRecommends

SpaceTab::SpaceTab() {
    this->inflateFromXMLRes("xml/fragment/space_tab.xml");
    //    recyclingGrid->estimatedRowHeight = brls::Application::contentHeight;
    //    recyclingGrid->registerCell(
    //        "Cell", []() { return RecyclingGridItemVideoCard::create(); });
    //    recyclingGrid->onNextPage([this]() { this->requestData(); });
    //    recyclingGrid->setRefreshAction([this]() {
    //        brls::Logger::debug("refresh home recommends");
    //        AutoTabFrame::focus2Sidebar(this);
    //        this->recyclingGrid->showSkeleton();
    //        this->requestData(true);
    //    });
    this->requestData();

    //    brls::Application::getWindowSizeChangedEvent()->subscribe([this](){
    //        recyclingGrid->estimatedRowHeight = brls::Application::contentHeight;
    //        recyclingGrid->reloadData();
    //    });
}

void SpaceTab::onCreate() {}

void SpaceTab::onLayout() {
    //    brls::Rect rect = getFrame();
    //    if (!(rect == oldRect)) MPVCore::instance().setFrameSize(rect);
    //    oldRect = rect;
}

void SpaceTab::onRecommendVideoList(const bilibili::RecommendVideoListResultWrapper& result) {
    brls::Threading::sync([this, result]() {
        MPVCore::instance().setUrl("https://www.w3schools.com/html/movie.mp4");
        //        auto* datasource = dynamic_cast<DataSourceSpaceVideoList*>(
        //            recyclingGrid->getDataSource());
        //        if (datasource && result.requestIndex != 1) {
        //            brls::Logger::debug("refresh home recommends: auto load {}",
        //                                result.requestIndex);
        //            datasource->appendData(result.item);
        //            recyclingGrid->notifyDataChanged();
        //        } else {
        //            brls::Logger::verbose("refresh home recommends: first page");
        //            recyclingGrid->setDataSource(
        //                new DataSourceSpaceVideoList(result.item));
        //        }
    });
}

brls::View* SpaceTab::create() { return new SpaceTab(); }

void SpaceTab::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                    brls::FrameContext* ctx) {
    MPVCore::instance().draw(brls::Rect(x, y, width, height), this->getAlpha());
}

SpaceTab::~SpaceTab() = default;

void SpaceTab::onError(const std::string& error) {
    brls::sync([this, error]() {
        //        this->recyclingGrid->setError(error);
    });
}

void SpaceTab::willDisappear(bool resetState) {
    Box::willDisappear(resetState);
    brls::Logger::error("SpaceTab::willDisappear");
    //    MPVCore::instance().setSkipRender(true);
}

void SpaceTab::willAppear(bool resetState) {
    // todo：从其他页面返回时重新加载进度
    Box::willAppear(resetState);
    //    MPVCore::instance().setSkipRender(false);
    //    MPVCore::instance().setFrameSize(getFrame());
}