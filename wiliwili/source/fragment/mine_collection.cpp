//
// Created by fang on 2022/7/30.
//

#include <utility>
#include <borealis/core/thread.hpp>
#include <borealis/views/image.hpp>

#include "fragment/mine_collection.hpp"
#include "fragment/mine_collection_video_list.hpp"
#include "view/video_card.hpp"
#include "utils/number_helper.hpp"
#include "utils/image_helper.hpp"

using namespace brls::literals;

class DataSourceMineCollectionList : public RecyclingGridDataSource {
public:
    /**
     * 收藏或订阅列表
     * @param result 列表数据
     * @param type 1 为收藏列表, 2 为订阅列表
     */
    DataSourceMineCollectionList(bilibili::CollectionListResult result, int type)
        : list(std::move(result)), dataType(type) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemCollectionVideoCard* item =
            (RecyclingGridItemCollectionVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::CollectionResult& r = this->list[index];

        // 封面左下角文字
        auto badge = std::to_string(r.media_count) + "wiliwili/mine/num"_i18n;
        if (dataType == COLLECTION_UI_TYPE_1) {
            if (r.attr & 1) {
                badge += " " + "wiliwili/mine/private"_i18n;
            } else {
                badge += " " + "wiliwili/mine/public"_i18n;
            }
        } else {
            badge += " · ";
            if (r.type == SUBSCRIPTION_TYPE_1) {
                // 订阅的收藏
                badge += "wiliwili/player/collection/type"_i18n;
            } else if (r.type == SUBSCRIPTION_TYPE_2) {
                // 订阅的合集
                badge += "wiliwili/player/ugc_season"_i18n;
            } else {
                badge += r.upper.name;
            }
        }

        // 标题下方的副标题
        std::string subtitle;
        if (dataType == COLLECTION_UI_TYPE_2 && r.type == SUBSCRIPTION_TYPE_2) {
            subtitle = wiliwili::num2w(r.view_count) + "播放";
        } else {
            subtitle = "wiliwili/mine/pub"_i18n + wiliwili::sec2date(r.ctime);
        }

        // 封面
        auto cover = r.cover.empty() ? "" : r.cover + ImageHelper::h_ext;

        item->setCard(cover, r.title, subtitle, badge);

        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        bilibili::CollectionResult& r = this->list[index];

        if (this->dataType == COLLECTION_UI_TYPE_2 && r.type == SUBSCRIPTION_TYPE_2) {
            brls::Application::pushActivity(
                new brls::Activity(new MineCollectionVideoList(list[index], COLLECTION_UI_TYPE_2)),
                brls::TransitionAnimation::NONE);
        } else {
            brls::Application::pushActivity(
                new brls::Activity(new MineCollectionVideoList(list[index], COLLECTION_UI_TYPE_1)),
                brls::TransitionAnimation::NONE);
        }
    }

    void appendData(const bilibili::CollectionListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::CollectionListResult list;
    int dataType;
};

MineCollection::MineCollection() {
    this->inflateFromXMLRes("xml/fragment/mine_collection.xml");
    brls::Logger::debug("Fragment MineCollection: create");

    this->registerFloatXMLAttribute("type", [this](float value) {
        this->setRequestType((int)value);
        this->requestData(true);
    });

    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemCollectionVideoCard::create(); });

    recyclingGrid->onNextPage([this]() { this->requestData(); });

    recyclingGrid->setRefreshAction([this]() {
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        this->requestData(true);
    });
}

MineCollection::~MineCollection() { brls::Logger::debug("Fragment MineCollectionActivity: delete"); }

brls::View* MineCollection::create() { return new MineCollection(); }

void MineCollection::onCreate() {
    this->registerTabAction(getRequestType() == COLLECTION_UI_TYPE_1 ? "wiliwili/mine/refresh_collection"_i18n
                                                                     : "wiliwili/mine/refresh_subscription"_i18n,
                            brls::ControllerButton::BUTTON_X, [this](brls::View* view) -> bool {
                                this->recyclingGrid->refresh();
                                return true;
                            });
}

void MineCollection::onCollectionList(const bilibili::CollectionListResultWrapper& result) {
    brls::Logger::debug("collection: {} ", result.count);
    for (auto& i : result.list) {
        brls::Logger::debug("{}", i.title);
    }
    brls::Threading::sync([this, result]() {
        auto* datasource = dynamic_cast<DataSourceMineCollectionList*>(recyclingGrid->getDataSource());
        if (datasource && result.index != 1) {
            if (!result.list.empty()) {
                datasource->appendData(result.list);
                recyclingGrid->notifyDataChanged();
            }
        } else {
            recyclingGrid->setDataSource(new DataSourceMineCollectionList(result.list, getRequestType()));
        }
    });
}

void MineCollection::onError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}