//
// Created by fang on 2022/7/31.
//

#include <utility>
#include <borealis/views/applet_frame.hpp>
#include <borealis/core/thread.hpp>

#include "fragment/mine_collection_video_list.hpp"
#include "view/video_card.hpp"
#include "view/text_box.hpp"
#include "utils/image_helper.hpp"
#include "utils/activity_helper.hpp"
#include "bilibili.h"

using namespace brls::literals;

class DataSourceCollectionVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceCollectionVideoList(bilibili::CollectionVideoListResult result) : list(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::CollectionVideoResult& r = this->list[index];

        std::string author = r.upper.name;
        if (r.type == 24) author = r.intro;

        item->setCard(r.cover + ImageHelper::h_ext, r.title, author, r.pubtime, r.cnt_info.play, r.cnt_info.danmaku,
                      r.duration);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto& data = list[index];
        switch (data.type) {
            case 2:  // 普通视频
                Intent::openBV(data.bvid);
                break;
            case 12:  // 音频
            case 21:  // 视频合集
                break;
            case 24:  // 番剧
                Intent::openSeasonByEpId(data.id);
                break;
        }
    }

    void appendData(const bilibili::CollectionVideoListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::CollectionVideoListResult list;
};

MineCollectionVideoList::MineCollectionVideoList() {
    brls::Logger::debug("Fragment MineCollectionVideoList: create");
    this->inflateFromXMLRes("xml/fragment/mine_collection_video_list.xml");
    registerFloatXMLAttribute("type", [this](float value) {
        if (this->collectionData.id != 0) brls::fatal("You must set type before collection id.");
        this->requestType = (int)value;
    });
    registerStringXMLAttribute("collection", [this](const std::string& value) {
        this->collectionData.id = std::stoll(value);
        this->requestCollectionList();
    });
    registerBoolXMLAttribute("footerHidden", [this](bool value) {
        this->appletFrame->setFooterVisibility(value ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
    });

    // 初始化列表
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    recyclingGrid->onNextPage([this]() { this->requestCollectionList(); });
    recyclingGrid->setRefreshAction([this]() {
        brls::Application::giveFocus(this->imageCover);
        this->recyclingGrid->showSkeleton();
        this->requestIndex = 1;
        this->hasMore      = true;
        this->requestCollectionList();
    });
    this->registerAction("wiliwili/home/common/refresh"_i18n, brls::ControllerButton::BUTTON_X,
                         [this](brls::View* view) -> bool {
                             this->recyclingGrid->refresh();
                             return true;
                         });
}

MineCollectionVideoList::MineCollectionVideoList(const bilibili::CollectionResult& data, int type)
    : MineCollectionVideoList() {
    // 从 data 中获取数据，设置UI
    collectionData = data;
    requestType    = type;
    this->labelTitle->setText(data.title);
    this->labelSubtitle->setText(fmt::format("{}{}", data.media_count, "wiliwili/mine/num"_i18n));
    ImageHelper::with(this->imageCover)->load(data.cover);

    this->requestCollectionList();
}

MineCollectionVideoList::~MineCollectionVideoList() {
    brls::Logger::debug("Fragment MineCollectionVideoListActivity: delete");
}

brls::View* MineCollectionVideoList::create() { return new MineCollectionVideoList(); }

void MineCollectionVideoList::requestCollectionList() {
    if (!hasMore) return;

    brls::Logger::debug("requestCollectionList id: {}, type: {}, index: {}", collectionData.id, requestType,
                        requestIndex);

    ASYNC_RETAIN
    BILI::get_collection_video_list(
        collectionData.id, (int)requestIndex, 20, requestType,
        [ASYNC_TOKEN](const bilibili::CollectionVideoListResultWrapper& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                hasMore = result.has_more;
                if (requestIndex != result.index) {
                    brls::Logger::error("request index error: request {} got {}", requestIndex, result.index);
                }
                requestIndex++;

                auto* datasource = dynamic_cast<DataSourceCollectionVideoList*>(recyclingGrid->getDataSource());
                if (datasource && result.index != 1) {
                    if (!result.medias.empty()) {
                        datasource->appendData(result.medias);
                        recyclingGrid->notifyDataChanged();
                    }
                } else {
                    recyclingGrid->setDataSource(new DataSourceCollectionVideoList(result.medias));
                    labelTitle->setText(result.info.title);
                    labelSubtitle->setText(fmt::format("{}{} · {}: {}", result.info.media_count,
                                                       "wiliwili/mine/num"_i18n, "wiliwili/mine/creator"_i18n,
                                                       result.info.upper.name));
                    ImageHelper::with(this->imageCover)->load(result.info.cover);
                }
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->recyclingGrid->setError(error);
            });
        });
}