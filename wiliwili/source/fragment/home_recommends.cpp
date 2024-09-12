//
// Created by fang on 2022/6/14.
//

#include <utility>
#include <borealis/core/thread.hpp>

#include "fragment/home_recommends.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "utils/number_helper.hpp"
#include "utils/config_helper.hpp"
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
        if (r.business_info.is_ad) {
            auto& card = r.business_info.archive;
            auto& info = r.business_info;
            auto& mark = r.business_info.business_mark;
            if (r.business_info.is_ad_video) {
                // 推广视频
                item->setCard(card.pic + ImageHelper::h_ext, info.title, card.owner.name, card.pubdate, card.stat.view,
                              card.stat.danmaku, card.duration);
                item->setExtraInfo(mark.img_url, (float)mark.img_width * 0.375f, (float)mark.img_height * 0.375f);
            } else {
                // 广告网页
                item->setCard(info.pic + ImageHelper::h_ext, info.title, info.adver_name, "", "", "", mark.text);
            }
        } else {
            item->setCard(r.pic + ImageHelper::h_ext, r.title, r.owner.name, r.pubdate, r.stat.view, r.stat.danmaku,
                          r.duration, r.rcmd_reason.content);
        }
        return item;
    }

    size_t getItemCount() override { return recommendList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        if (recommendList[index].business_info.is_ad) {
            if (recommendList[index].business_info.is_ad_video) {
                Intent::openBV(recommendList[index].business_info.archive.bvid);
            } else {
                brls::Application::getPlatform()->openBrowser(recommendList[index].business_info.url);
            }
        } else {
            Intent::openBV(recommendList[index].bvid);
        }
    }

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

void HomeRecommends::onRecommendVideoList(const bilibili::RecommendVideoListResultWrapper& originalResult) {
    // 过滤up主
    bilibili::RecommendVideoListResultWrapper result;
    result.requestIndex = originalResult.requestIndex;
    result.item.resize(originalResult.item.size());
    if (ProgramConfig::instance().upFilter.empty()) {
        std::copy(originalResult.item.begin(), originalResult.item.end(), result.item.begin());
    } else {
        auto it = std::copy_if(originalResult.item.begin(), originalResult.item.end(), result.item.begin(),
                               [](const bilibili::RecommendVideoResult& r) {
                                   return !ProgramConfig::instance().upFilter.count(r.owner.mid);
                               });
        result.item.resize(std::distance(result.item.begin(), it));
    }

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