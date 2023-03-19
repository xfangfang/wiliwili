//
// Created by fang on 2022/7/31.
//

#include "fragment/mine_collection_video_list.hpp"
#include "activity/player_activity.hpp"
#include "view/video_card.hpp"
#include "utils/image_helper.hpp"
#include "bilibili.h"

class DataSourceCollectionVideoList : public RecyclingGridDataSource {
public:
    DataSourceCollectionVideoList(bilibili::CollectionVideoListResult result)
        : list(result) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item =
            (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::CollectionVideoResult& r = this->list[index];

        std::string author = r.upper.name;
        if (r.type == 24) author = r.intro;

        item->setCard(r.cover + ImageHelper::h_ext, r.title, author, r.pubtime,
                      r.cnt_info.play, r.cnt_info.danmaku, r.duration);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto& data = list[index];
        switch (data.type) {
            case 2:  // 普通视频
                brls::Application::pushActivity(new PlayerActivity(data.bvid));
                break;
            case 12:  // 音频
                break;
            case 21:  // 视频合集
                break;
            case 24:  // 番剧
                brls::Application::pushActivity(
                    new PlayerSeasonActivity(data.id, PGC_ID_TYPE::EP_ID));
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

MineCollectionVideoList::MineCollectionVideoList(
    const bilibili::CollectionResult& data)
    : collectionData(data) {
    this->inflateFromXMLRes("xml/fragment/mine_collection_video_list.xml");
    brls::Logger::debug("Fragment MineCollectionVideoList: create");

    this->labelTitle->setText(data.title);
    this->labelSubtitle->setText(
        fmt::format("{}{}", data.media_count, "wiliwili/mine/num"_i18n));
    ImageHelper::with(this->imageCover)->load(data.cover);

    recyclingGrid->registerCell(
        "Cell", []() { return RecyclingGridItemVideoCard::create(); });
    recyclingGrid->onNextPage([this]() { this->requestCollectionList(); });
    this->requestCollectionList();

    this->registerAction("wiliwili/home/common/refresh"_i18n,
                         brls::ControllerButton::BUTTON_X,
                         [this](brls::View* view) -> bool {
                             brls::Application::giveFocus(this->imageCover);
                             this->recyclingGrid->showSkeleton();
                             this->requestIndex = 1;
                             this->requestCollectionList();
                             return true;
                         });
}

MineCollectionVideoList::~MineCollectionVideoList() {
    brls::Logger::debug("Fragment MineCollectionVideoListActivity: delete");
}

brls::View* MineCollectionVideoList::create(
    const bilibili::CollectionResult& data) {
    return new MineCollectionVideoList(data);
}

void MineCollectionVideoList::requestCollectionList() {
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_collection_video_list(
        collectionData.id, requestIndex++, 20,
        [ASYNC_TOKEN](
            const bilibili::CollectionVideoListResultWrapper& result) {
            brls::Threading::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                DataSourceCollectionVideoList* datasource =
                    dynamic_cast<DataSourceCollectionVideoList*>(
                        recyclingGrid->getDataSource());
                if (datasource && result.index != 1) {
                    datasource->appendData(result.medias);
                    recyclingGrid->notifyDataChanged();
                } else {
                    recyclingGrid->setDataSource(
                        new DataSourceCollectionVideoList(result.medias));
                    labelSubtitle->setText(fmt::format(
                        "{}{} · {}: {}", result.info.media_count,
                        "wiliwili/mine/num"_i18n, "wiliwili/mine/creator"_i18n,
                        result.info.upper.name));
                    ImageHelper::with(this->imageCover)
                        ->load(result.info.cover);
                }
            });
        },
        [](const std::string& error) { brls::Logger::error(fmt::runtime(error)); });
}