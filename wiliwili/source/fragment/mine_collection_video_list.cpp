//
// Created by fang on 2022/7/31.
//

#include "fragment/mine_collection_video_list.hpp"
#include "activity/player_activity.hpp"
#include "view/video_card.hpp"
#include "utils/image_helper.hpp"
#include "bilibili.h"

class DataSourceCollectionVideoList
        : public RecyclingGridDataSource
{
public:
    DataSourceCollectionVideoList(bilibili::CollectionVideoListResult result):list(result){

    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index){
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::CollectionVideoResult& r = this->list[index];
        item->setCard(r.cover+"@672w_378h_1c.jpg",r.title,r.upper.name,r.pubtime, r.cnt_info.play, r.cnt_info.danmaku, r.duration);
        return item;
    }

    size_t getItemCount() {
        return list.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) {
        brls::Application::pushActivity(new PlayerActivity(list[index].bvid));
    }

    void appendData(const bilibili::CollectionVideoListResult& data){
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

private:
    bilibili::CollectionVideoListResult list;
};


MineCollectionVideoList::MineCollectionVideoList(const bilibili::CollectionResult& data):collectionData(data) {
    this->inflateFromXMLRes("xml/fragment/mine_collection_video_list.xml");
    brls::Logger::debug("Fragment MineCollectionVideoList: create");

    this->labelTitle->setText(collectionData.title);

    auto badge = std::to_string(collectionData.media_count) + "个内容";
    if(collectionData.attr & 1){
        badge += " · 私密";
    } else {
        badge += " · 公开";
    }

    this->labelSubtitle->setText(badge);
    ImageHelper::with(this->imageCover)->load(collectionData.cover)->into(this->imageCover);

    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    recyclingGrid->onNextPage([this](){
        this->requestCollectionList();
    });
    this->requestCollectionList();

    this->registerAction("刷新", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
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

brls::View *MineCollectionVideoList::create(const bilibili::CollectionResult& data) {
    return new MineCollectionVideoList(data);
}

void MineCollectionVideoList::requestCollectionList(){
    bilibili::BilibiliClient::get_collection_video_list(collectionData.id, requestIndex++, 20, [this](const bilibili::CollectionVideoListResultWrapper &result){
        this->onCollectionList(result);
    }, [](const std::string & error){
        brls::Logger::error(error);
    });
}

void MineCollectionVideoList::onCollectionList(const bilibili::CollectionVideoListResultWrapper &result){
    ASYNC_RETAIN
    brls::Threading::sync([ASYNC_TOKEN, result]() {
        ASYNC_RELEASE
        DataSourceCollectionVideoList* datasource = (DataSourceCollectionVideoList *)recyclingGrid->getDataSource();
        if(datasource && result.index != 1){
            datasource->appendData(result.medias);
            recyclingGrid->notifyDataChanged();
        } else{
            recyclingGrid->setDataSource(new DataSourceCollectionVideoList(result.medias));
        }
    });
}