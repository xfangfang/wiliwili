//
// Created by fang on 2022/7/12.
//

#include <utility>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>
#include <borealis/views/applet_frame.hpp>

#include "fragment/home_live.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "view/grid_dropdown.hpp"
#include "utils/image_helper.hpp"
#include "utils/activity_helper.hpp"

using namespace brls::literals;

const std::string GridSubAreaCellContentXML = R"xml(
<brls:Box
        width="auto"
        height="@style/brls/sidebar/item_height"
        focusable="true"
        paddingTop="12.5"
        paddingBottom="12.5"
        axis="column"
        alignItems="center">

    <brls:Image
        id="area/avatar"
        scalingType="fill"
        cornerRadius="4"
        width="50"
        height="50"/>

    <brls:Label
            id="area/title"
            width="auto"
            height="auto"
            grow="1"
            fontSize="14" />

</brls:Box>
)xml";

class GridSubAreaCell : public RecyclingGridItem {
public:
    GridSubAreaCell() { this->inflateFromXMLString(GridSubAreaCellContentXML); }

    void setData(const std::string& name, const std::string& pic) {
        title->setText(name);
        if (pic.empty()) {
            this->image->setImageFromRes("pictures/22_open.png");
        } else {
            ImageHelper::with(image)->load(pic + ImageHelper::face_ext);
        }
    }

    void prepareForReuse() override {
        //准备显示该项
        this->image->setImageFromRes("pictures/video-card-bg.png");
    }

    void cacheForReuse() override {
        //准备回收该项
        ImageHelper::clear(this->image);
    }

    static RecyclingGridItem* create() { return new GridSubAreaCell(); }

protected:
    BRLS_BIND(brls::Label, title, "area/title");
    BRLS_BIND(brls::Image, image, "area/avatar");
};

const std::string GridMainAreaCellContentXML = R"xml(
<brls:Box
        width="auto"
        height="@style/brls/sidebar/item_height"
        focusable="true"
        paddingTop="12.5"
        paddingBottom="12.5"
        alignItems="center">

    <brls:Image
        id="area/avatar"
        scalingType="fill"
        cornerRadius="4"
        marginLeft="10"
        marginRight="10"
        width="40"
        height="40"/>

    <brls:Label
            id="area/title"
            width="auto"
            height="auto"
            grow="1"
            fontSize="22" />

</brls:Box>
)xml";

class GridMainAreaCell : public RecyclingGridItem {
public:
    GridMainAreaCell() { this->inflateFromXMLString(GridMainAreaCellContentXML); }

    void setData(const std::string& name, const std::string& pic) {
        title->setText(name);
        if (pic.empty()) {
            this->image->setImageFromRes("pictures/22_open.png");
        } else {
            ImageHelper::with(image)->load(pic + ImageHelper::face_ext);
        }
    }

    void setSelected(bool value) { this->title->setTextColor(value ? selectedColor : fontColor); }

    void prepareForReuse() override {
        //准备显示该项
        this->image->setImageFromRes("pictures/video-card-bg.png");
    }

    void cacheForReuse() override {
        //准备回收该项
        ImageHelper::clear(this->image);
    }

    static RecyclingGridItem* create() { return new GridMainAreaCell(); }

protected:
    BRLS_BIND(brls::Label, title, "area/title");
    BRLS_BIND(brls::Image, image, "area/avatar");

    NVGcolor selectedColor = brls::Application::getTheme().getColor("color/bilibili");
    NVGcolor fontColor     = brls::Application::getTheme().getColor("brls/text");
};

class DataSourceLiveVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceLiveVideoList(bilibili::LiveVideoListResult result) : videoList(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemLiveVideoCard* item = (RecyclingGridItemLiveVideoCard*)recycler->dequeueReusableCell("Cell");

        bilibili::LiveVideoResult& r = this->videoList[index];
        item->setCard(r.cover + ImageHelper::h_ext, r.title, r.uname, r.area_name, r.online, r.following);
        return item;
    }

    size_t getItemCount() override { return videoList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        Intent::openLive(videoList[index].roomid, videoList[index].title, videoList[index].watched_show.text_large);
    }

    void appendData(const bilibili::LiveVideoListResult& data) {
        this->videoList.insert(this->videoList.end(), data.begin(), data.end());
    }

    void clearData() override { this->videoList.clear(); }

private:
    bilibili::LiveVideoListResult videoList;
};

typedef brls::Event<const bilibili::LiveFullAreaResult> AreaSelectedEvent;

class DataSourceLiveMainAreaList : public RecyclingGridDataSource {
public:
    DataSourceLiveMainAreaList(bilibili::LiveFullAreaListResult result, size_t mainIndex)
        : areaList(std::move(result)), defaultIndex(mainIndex) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        auto item = (GridMainAreaCell*)recycler->dequeueReusableCell("Cell");
        auto& r   = this->areaList[index];
        std::string pic;
        if (!r.area_list.empty()) {
            pic = r.area_list[0].pic;
        }
        item->setData(r.name, pic);
        item->setSelected(index == defaultIndex);
        return item;
    }

    size_t getItemCount() override { return areaList.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        this->areaSelectedEvent.fire(areaList[index]);
        auto* item                             = dynamic_cast<GridMainAreaCell*>(recycler->getGridItemByIndex(index));
        std::vector<RecyclingGridItem*>& items = recycler->getGridItems();
        for (auto& i : items) {
            auto* cell = dynamic_cast<GridMainAreaCell*>(i);
            if (cell) cell->setSelected(false);
        }
        if (item) item->setSelected(true);
        defaultIndex = index;
    }

    void clearData() override { this->areaList.clear(); }

    AreaSelectedEvent* getSelectedEvent() { return &this->areaSelectedEvent; }

private:
    bilibili::LiveFullAreaListResult areaList;
    AreaSelectedEvent areaSelectedEvent;
    size_t defaultIndex = -1;
};

// 主分区id，子分区id，主分区名，子分区名，进入的分区id
// 某个子分区可能位于全站推荐分区下，“进入的分区id” 这时为 0，其他情况同主分区id
typedef brls::Event<int, int, std::string, std::string, int> SubAreaSelectedEvent;

class DataSourceLiveSubAreaList : public RecyclingGridDataSource {
public:
    explicit DataSourceLiveSubAreaList(bilibili::LiveFullAreaResult result) : areaList(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        auto item = (GridSubAreaCell*)recycler->dequeueReusableCell("Cell");
        auto& r   = this->areaList.area_list[index];
        item->setData(r.name, r.pic);
        return item;
    }

    size_t getItemCount() override { return areaList.area_list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        this->subAreaSelectedEvent.fire(areaList.area_list[index].parent_id, areaList.area_list[index].id,
                                        areaList.name, areaList.area_list[index].name, areaList.id);
    }

    void clearData() override {}

    SubAreaSelectedEvent* getSelectedEvent() { return &this->subAreaSelectedEvent; }

private:
    bilibili::LiveFullAreaResult areaList;
    SubAreaSelectedEvent subAreaSelectedEvent;
};

class HomeLiveArea : public EmptyDropdown {
public:
    HomeLiveArea(const bilibili::LiveFullAreaListResult& result, int mainID, int subID) : areaList(result) {
        this->inflateFromXMLRes("xml/fragment/home_live_area.xml");

        mainGrid->registerCell("Cell", []() { return GridMainAreaCell::create(); });
        subGrid->registerCell("Cell", []() { return GridSubAreaCell::create(); });

        applet->addGestureRecognizer(new brls::TapGestureRecognizer([this](brls::TapGestureStatus status, ...) {
            if (status.position.y < this->content->getY()) this->applet->dismiss();
        }));

        if (result.empty()) return;

        // 获取默认选中的主分区序号
        size_t mainIndex = 0;
        for (size_t i = 0; i < result.size(); i++) {
            if (result[i].id == mainID) {
                mainIndex = i;
                break;
            }
        }

        // 生成主分区数据源
        auto mainDS = new DataSourceLiveMainAreaList(result, mainIndex);
        mainDS->getSelectedEvent()->subscribe([this](const auto& area) { this->selectMainArea(area, 0); });
        mainGrid->setDefaultCellFocus(mainIndex);
        mainGrid->setDataSource(mainDS);
        this->selectMainArea(result[mainIndex], subID);

        // 计算高度
        float height = (float)mainDS->getItemCount() * 70 + header->getHeight() + 150;  // bottom

        content->setHeight(fmin(height, brls::Application::contentHeight * 0.73f));
    }

    void selectMainArea(const bilibili::LiveFullAreaResult& data, int subID) {
        // 获取默认选中的子分区序号
        size_t subIndex = 0;
        for (size_t i = 0; i < data.area_list.size(); i++) {
            if (data.area_list[i].id == subID) {
                subIndex = i;
                break;
            }
        }

        // 生成子分区数据源
        auto subDS = new DataSourceLiveSubAreaList(data);
        subDS->getSelectedEvent()->subscribe(
            [this](int mainID, int subID, const std::string& mainName, const std::string& subName, int entryID) {
                brls::Logger::debug("live main/{}/{} sub/{}/{}", mainID, mainName, subID, subName);
                subAreaSelectedEvent.fire(mainID, subID, mainName, subName, entryID);
            });
        subGrid->setDefaultCellFocus(subIndex);
        subGrid->setDataSource(subDS);
    }

    SubAreaSelectedEvent* getSelectedEvent() { return &this->subAreaSelectedEvent; }

    View* getDefaultFocus() override { return this->subGrid; }

private:
    BRLS_BIND(brls::Box, header, "grid_dropdown/header");
    BRLS_BIND(brls::Label, title, "grid_dropdown/title_label");
    BRLS_BIND(brls::Box, content, "grid_dropdown/content");
    BRLS_BIND(brls::AppletFrame, applet, "grid_dropdown/applet");
    BRLS_BIND(RecyclingGrid, mainGrid, "area/mainList");
    BRLS_BIND(RecyclingGrid, subGrid, "area/subList");

    bilibili::LiveFullAreaListResult areaList;
    SubAreaSelectedEvent subAreaSelectedEvent;
};

HomeLive::HomeLive() {
    this->inflateFromXMLRes("xml/fragment/home_live.xml");
    brls::Logger::debug("Fragment HomeLive: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemLiveVideoCard::create(); });
    recyclingGrid->onNextPage([this]() { this->requestData(); });
    recyclingGrid->setRefreshAction([this]() {
        AutoTabFrame::focus2Sidebar(this);
        this->recyclingGrid->showSkeleton();
        this->requestData(-1, -1, 1);
        this->requestAreaList();
    });
    this->requestData();
    this->requestAreaList();
    live_label->setText("推荐 - 全部推荐");
}

void HomeLive::onLiveList(const bilibili::LiveVideoListResult& result, int index) {
    brls::Threading::sync([this, result, index]() {
        auto* datasource = dynamic_cast<DataSourceLiveVideoList*>(recyclingGrid->getDataSource());
        if (datasource && index != 1) {
            if (result.empty()) return;
            if (!result.empty()) {
                datasource->appendData(result);
                recyclingGrid->notifyDataChanged();
            }
        } else {
            if (result.empty())
                recyclingGrid->setEmpty();
            else
                recyclingGrid->setDataSource(new DataSourceLiveVideoList(result));
        }
    });
}

void HomeLive::onCreate() {
    this->live_box->addGestureRecognizer(new brls::TapGestureRecognizer(this->live_box, [this]() {
        this->switchChannel();
        return true;
    }));
    this->registerTabAction("wiliwili/home/common/switch"_i18n, brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                this->switchChannel();
                                return true;
                            });
}

void HomeLive::switchChannel() {
    if (this->fullAreaList.empty()) return;

    AutoTabFrame::focus2Sidebar(this);
    auto* area = new HomeLiveArea(fullAreaList, staticEntry, staticSub);
    area->getSelectedEvent()->subscribe(
        [this](int main, int sub, const std::string& mainName, const std::string& subName, int entryID) {
            this->staticEntry = entryID;
            this->requestData(main, sub, 1);
            live_label->setText(mainName + " - " + subName);
            brls::Application::popActivity();
            recyclingGrid->refresh();
        });
    brls::Application::pushActivity(new brls::Activity(area));
}

void HomeLive::onError(const std::string& error) {
    brls::sync([this, error]() { this->recyclingGrid->setError(error); });
}

HomeLive::~HomeLive() { brls::Logger::debug("Fragment HomeLiveActivity: delete"); }

brls::View* HomeLive::create() { return new HomeLive(); }