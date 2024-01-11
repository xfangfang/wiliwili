//
// Created by fang on 2022/6/14.
//

#include <utility>
#include <borealis/core/thread.hpp>

#include "fragment/home_recommends.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "utils/number_helper.hpp"
#include "utils/activity_helper.hpp"
#include "utils/image_helper.hpp"

using namespace brls::literals;

/// DataSourceRecommendVideoList

class DataSourceRecommendVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceRecommendVideoList(bilibili::RecommendVideoListResult result)
        : recommendList(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::RecommendVideoResult& r = this->recommendList[index];
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

HomeRecommends::HomeRecommends() {
    this->inflateFromXMLRes("xml/fragment/home_recommends.xml");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    recyclingGrid->onNextPage([this]() { this->requestData(); });
    recyclingGrid->setRefreshAction([this]() {
        brls::Logger::debug("refresh home recommends");
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        this->requestData(true);
    });
    this->requestData();
}

void HomeRecommends::onCreate() {
    this->registerTabAction("wiliwili/home/common/refresh"_i18n, brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                this->recyclingGrid->refresh();
                                return true;
                            });
}

void HomeRecommends::onRecommendVideoList(const bilibili::RecommendVideoListResultWrapper& result) {
    brls::Threading::sync([this, result]() {
        auto* datasource = dynamic_cast<DataSourceRecommendVideoList*>(recyclingGrid->getDataSource());
        if (datasource && result.requestIndex != 1) {
            brls::Logger::debug("refresh home recommends: auto load {}", result.requestIndex);
            if (!result.item.empty()) {
                datasource->appendData(result.item);
                recyclingGrid->notifyDataChanged();
            }
        } else {
            brls::Logger::verbose("refresh home recommends: first page");
            recyclingGrid->setDataSource(new DataSourceRecommendVideoList(result.item));
        }
    });
}

brls::View* HomeRecommends::create() { return new HomeRecommends(); }

HomeRecommends::~HomeRecommends() = default;

void HomeRecommends::onError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}