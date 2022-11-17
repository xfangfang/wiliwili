//
// Created by fang on 2022/7/30.
//

#include "fragment/mine_collection.hpp"
#include "fragment/mine_collection_video_list.hpp"
#include "view/video_card.hpp"
#include "utils/number_helper.hpp"

using namespace brls::literals;

class DataSourceMineCollectionList : public RecyclingGridDataSource {
public:
    DataSourceMineCollectionList(bilibili::CollectionListResult result)
        : list(result) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemCollectionVideoCard* item =
            (RecyclingGridItemCollectionVideoCard*)
                recycler->dequeueReusableCell("Cell");

        bilibili::CollectionResult& r = this->list[index];

        auto badge = std::to_string(r.media_count) + "wiliwili/mine/num"_i18n;
        if (r.attr & 1) {
            badge += " " + "wiliwili/mine/public"_i18n;
        } else {
            badge += " " + "wiliwili/mine/private"_i18n;
        }
        auto time = "wiliwili/mine/pub"_i18n + wiliwili::sec2date(r.ctime);

        auto cover = r.cover;
        if (!cover.empty()) {
            cover += "@672w_378h_1c.jpg";
        }
        item->setCard(cover, r.title, time, badge);

        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        brls::Application::pushActivity(
            new brls::Activity(MineCollectionVideoList::create(list[index])));
    }

    void appendData(const bilibili::CollectionListResult& data) {
        brls::Logger::error("DataSourceRecommendVideoList: append data");
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::CollectionListResult list;
};

MineCollection::MineCollection() {
    this->inflateFromXMLRes("xml/fragment/mine_collection.xml");
    brls::Logger::debug("Fragment MineCollection: create");
    recyclingGrid->registerCell("Cell", []() {
        return RecyclingGridItemCollectionVideoCard::create();
    });
    recyclingGrid->onNextPage([this]() { this->requestData(); });
    this->requestData();
}

MineCollection::~MineCollection() {
    brls::Logger::debug("Fragment MineCollectionActivity: delete");
}

brls::View* MineCollection::create() { return new MineCollection(); }

void MineCollection::onCreate() {
    this->registerTabAction("wiliwili/mine/refresh_collection"_i18n,
                            brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                AutoTabFrame::focus2Sidebar(this);
                                this->recyclingGrid->showSkeleton();
                                this->requestData(true);
                                return true;
                            });
}

void MineCollection::onCollectionList(
    const bilibili::CollectionListResultWrapper& result) {
    brls::Logger::debug("collection: {} ", result.count);
    for (auto i : result.list) {
        brls::Logger::debug("{}", i.title);
    }
    brls::Threading::sync([this, result]() {
        DataSourceMineCollectionList* datasource =
            dynamic_cast<DataSourceMineCollectionList*>(
                recyclingGrid->getDataSource());
        if (datasource && result.index != 1) {
            datasource->appendData(result.list);
            recyclingGrid->notifyDataChanged();
        } else {
            recyclingGrid->setDataSource(
                new DataSourceMineCollectionList(result.list));
        }
    });
}

void MineCollection::onError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}