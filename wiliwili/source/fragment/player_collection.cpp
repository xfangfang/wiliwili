//
// Created by fang on 2022/12/28.
//

#include <borealis/core/thread.hpp>

#include "fragment/player_collection.hpp"
#include "bilibili.h"
#include "bilibili/result/mine_collection_result.h"
#include "utils/config_helper.hpp"
#include "view/recycling_grid.hpp"
#include "view/check_box.hpp"
#include <pystring.h>

using namespace brls::literals;

typedef struct CollectionListRequest {
    std::string add;
    std::string del;
} CollectionListRequest;

class CollectionListCell : public RecyclingGridItem {
public:
    CollectionListCell() { this->inflateFromXMLRes("xml/views/collection_list_cell.xml"); }

    void setSelected(bool selected) { this->checkbox->setChecked(selected); }

    bool getSelected() { return this->checkbox->getChecked(); }

    static RecyclingGridItem* create() { return new CollectionListCell(); }

    BRLS_BIND(brls::Label, title, "fav/title");
    BRLS_BIND(brls::Label, subtitle, "fav/subtitle");
    BRLS_BIND(BiliCheckBox, checkbox, "fav/checkbox");
};

class CollectionDataSourceList : public RecyclingGridDataSource {
public:
    CollectionDataSourceList(bilibili::SimpleCollectionListResult result, size_t defaultIndex = 0)
        : data(result), currentIndex(defaultIndex) {
        for (auto& i : data) {
            brls::Logger::debug("{} {} {}", i.title, i.id, i.fav_state);
        }
        for (size_t i = 0; i < data.size(); i++) selectionData.emplace_back(data[i].fav_state);
    }

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        CollectionListCell* item = (CollectionListCell*)recycler->dequeueReusableCell("Cell");

        auto& r = this->data[index];
        item->title->setText(r.title);
        item->subtitle->setText(getSubtitle(index));
        item->setSelected(this->selectionData[index]);
        item->setLineRight((index + 1) % recycler->spanCount);
        return item;
    }

    size_t getItemCount() override { return data.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        currentIndex             = index;
        selectionData[index]     = !selectionData[index];
        CollectionListCell* item = dynamic_cast<CollectionListCell*>(recycler->getGridItemByIndex(index));
        if (!item) return;

        item->setSelected(selectionData[index]);
        this->data[index].media_count += selectionData[index] ? 1 : -1;
        item->subtitle->setText(getSubtitle(index));
    }

    std::string getAddCollectionList() {
        std::vector<std::string> res;
        for (size_t i = 0; i < data.size(); i++) {
            if (!data[i].fav_state && selectionData[i]) res.push_back(std::to_string(data[i].id));
        }
        return pystring::join(",", res);
    }

    std::string getDeleteCollectionList() {
        std::vector<std::string> res;
        for (size_t i = 0; i < data.size(); i++) {
            if (data[i].fav_state && !selectionData[i]) res.push_back(std::to_string(data[i].id));
        }
        return pystring::join(",", res);
    }

    bool isFavorite() {
        for (auto i : selectionData) {
            if (i) return true;
        }
        return false;
    }

    void clearData() override { this->data.clear(); }

private:
    bilibili::SimpleCollectionListResult data;
    std::vector<bool> selectionData;
    size_t currentIndex;

    std::string getSubtitle(size_t index) {
        auto& r = this->data[index];
        std::string badge;
        if (r.attr & 1) {
            badge = fmt::format("{} · {}{}", "wiliwili/mine/private"_i18n, r.media_count, "wiliwili/mine/num"_i18n);
        } else {
            badge = fmt::format("{} · {}{}", "wiliwili/mine/public"_i18n, r.media_count, "wiliwili/mine/num"_i18n);
        }
        return badge;
    }
};

PlayerCollection::PlayerCollection(int rid, int type) {
    this->inflateFromXMLRes("xml/fragment/player_collection.xml");
    brls::Logger::debug("Fragment PlayerCollection: create");
    this->recyclingGrid->showSkeleton();
    this->recyclingGrid->registerCell("Cell", []() { return CollectionListCell::create(); });
    this->getCollectionList(rid, type);
}

PlayerCollection::~PlayerCollection() { brls::Logger::debug("Fragment PlayerCollection: delete"); }

std::string PlayerCollection::getAddCollectionList() {
    CollectionDataSourceList* dataSource = dynamic_cast<CollectionDataSourceList*>(recyclingGrid->getDataSource());
    if (!dataSource) return "";
    return dataSource->getAddCollectionList();
}

std::string PlayerCollection::getDeleteCollectionList() {
    CollectionDataSourceList* dataSource = dynamic_cast<CollectionDataSourceList*>(recyclingGrid->getDataSource());
    if (!dataSource) return "";
    return dataSource->getDeleteCollectionList();
}

bool PlayerCollection::isFavorite() {
    CollectionDataSourceList* dataSource = dynamic_cast<CollectionDataSourceList*>(recyclingGrid->getDataSource());
    if (!dataSource) return false;
    return dataSource->isFavorite();
}

void PlayerCollection::onCollectionList(const bilibili::SimpleCollectionListResultWrapper& result) {
    recyclingGrid->setDataSource(new CollectionDataSourceList(result.list));
}

void PlayerCollection::getCollectionList(int rid, int type) {
    std::string mid = ProgramConfig::instance().getUserID();

    ASYNC_RETAIN
    bilibili::BilibiliClient::get_collection_list_all(
        rid, type, mid,
        [ASYNC_TOKEN](const bilibili::SimpleCollectionListResultWrapper& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                this->onCollectionList(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->recyclingGrid->setError("");
            });
        });
}