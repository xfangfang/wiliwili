//
// Created by fang on 2022/6/9.
//

#include "activity/player_activity.hpp"
#include "fragment/dynamic_tab.hpp"
#include "view/auto_tab_frame.hpp"
#include "view/recycling_grid.hpp"
#include "view/svg_image.hpp"
#include "view/video_card.hpp"
#include "utils/image_helper.hpp"

class DynamicUserInfoView : public RecyclingGridItem {
public:
    DynamicUserInfoView(std::string xml) { this->inflateFromXMLRes(xml); }

    void setUserInfo(std::string avatar, std::string username,
                     bool isUpdate = false) {
        this->labelUsername->setText(username);
        ImageHelper::with(this)->load(avatar)->into(this->avatarView);
    }

    brls::Image* getAvatar() { return this->avatarView; }

    void prepareForReuse() {}

    void cacheForReuse() {
        if (!dynamic_cast<SVGImage*>(this->avatarView.getView()))
            ImageHelper::clear(this->avatarView);
    }

    static RecyclingGridItem* create(
        std::string xml = "xml/views/user_info_dynamic.xml") {
        return new DynamicUserInfoView(xml);
    }

private:
    BRLS_BIND(brls::Image, avatarView, "avatar");
    BRLS_BIND(brls::Label, labelUsername, "username");
};

class DataSourceUpList : public RecyclingGridDataSource {
public:
    DataSourceUpList(bilibili::DynamicUpListResult result) : list(result) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) {
        if (index == 0) {
            DynamicUserInfoView* item =
                (DynamicUserInfoView*)recycler->dequeueReusableCell("CellAll");
            return item;
        }

        //从缓存列表中取出 或者 新生成一个表单项
        DynamicUserInfoView* item =
            (DynamicUserInfoView*)recycler->dequeueReusableCell("Cell");

        auto& r = this->list[index - 1];
        item->setUserInfo(r.user_profile.info.face + "@96w_96h_1c.jpg",
                          r.user_profile.info.uname);
        return item;
    }

    size_t getItemCount() { return list.size() + 1; }

    void onItemSelected(RecyclingGrid* recycler, size_t index) {
        if (index == 0) {
            // 选择全部
            userSelectedEvent.fire(0);
        } else {
            // 选择具体的某个up主
            userSelectedEvent.fire(list[index - 1].user_profile.info.uid);
        }
    }

    void appendData(const bilibili::DynamicUpListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    UserSelectedEvent* getSelectedEvent() { return &this->userSelectedEvent; }

private:
    bilibili::DynamicUpListResult list;
    UserSelectedEvent userSelectedEvent;
};

class DataSourceDynamicVideoList : public RecyclingGridDataSource {
public:
    DataSourceDynamicVideoList(bilibili::DynamicVideoListResult result)
        : list(result) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item =
            (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        auto& r = this->list[index];
        item->setCard(r.pic + "@672w_378h_1c.jpg", r.title, r.owner.name,
                      r.pubdate, r.stat.view, r.stat.danmaku, r.duration);
        return item;
    }

    size_t getItemCount() { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) {
        brls::Application::pushActivity(new PlayerActivity(list[index].bvid));
    }

    void appendData(const bilibili::DynamicVideoListResult& data) {
        bool skip = false;
        for (auto i : data) {
            skip = false;
            for (auto j : this->list) {
                if (j.aid == i.aid) {
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                this->list.push_back(i);
            }
        }
    }

private:
    bilibili::DynamicVideoListResult list;
};

DynamicTab::DynamicTab() {
    this->inflateFromXMLRes("xml/fragment/dynamic_tab.xml");
    brls::Logger::debug("Fragment DynamicTab: create");

    // 初始化左侧Up主列表
    upRecyclingGrid->registerCell(
        "Cell", []() { return DynamicUserInfoView::create(); });
    upRecyclingGrid->registerCell("CellAll", []() {
        return DynamicUserInfoView::create(
            "xml/views/user_info_dynamic_all.xml");
    });
    upRecyclingGrid->setDataSource(
        new DataSourceUpList(bilibili::DynamicUpListResult()));
    this->DynamicTabRequest::requestData();

    // 初始化右侧视频列表
    videoRecyclingGrid->registerCell(
        "Cell", []() { return RecyclingGridItemVideoCard::create(); });
    videoRecyclingGrid->onNextPage([this]() {
        //自动加载下一页
        this->DynamicVideoRequest::requestData();
    });
    this->DynamicVideoRequest::requestData();
}

DynamicTab::~DynamicTab() {
    brls::Logger::debug("Fragment DynamicTabActivity: delete");
}

brls::View* DynamicTab::create() { return new DynamicTab(); }

void DynamicTab::onUpList(const bilibili::DynamicUpListResultWrapper& result) {
    brls::Threading::sync([this, result]() {
        auto dataSource = new DataSourceUpList(result.items);
        upRecyclingGrid->setDataSource(dataSource);
        dataSource->getSelectedEvent()->subscribe(
            [this](unsigned int mid) { this->changeUser(mid); });
    });
}

void DynamicTab::onError(const std::string& error) {
    brls::Logger::error("DynamicTab::onError {}", error);
    brls::sync([this, error]() {
        videoRecyclingGrid->setError(error);
        upRecyclingGrid->setDataSource(
            new DataSourceUpList(bilibili::DynamicUpListResult()));
    });
}

void DynamicTab::onCreate() {
    this->registerTabAction("刷新列表", brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                AutoTabFrame::focus2Sidebar(this);
                                this->upRecyclingGrid->showSkeleton();
                                this->DynamicTabRequest::requestData();
                                this->changeUser(0);
                                return true;
                            });
    this->videoRecyclingGrid->registerAction(
        "刷新", brls::ControllerButton::BUTTON_X,
        [this](brls::View* view) -> bool {
            //焦点转移到UP主列表
            brls::Application::giveFocus(this->upRecyclingGrid);
            //展示骨架屏
            this->videoRecyclingGrid->showSkeleton();
            //请求刷新数据
            this->DynamicVideoRequest::requestData(true);
            return true;
        });
}

void DynamicTab::changeUser(unsigned int mid) {
    this->setCurrentUser(mid);
    this->videoRecyclingGrid->showSkeleton();
    this->DynamicVideoRequest::requestData(true);
}

// 获取到动态视频
void DynamicTab::onDynamicVideoList(
    const bilibili::DynamicVideoListResult& result, unsigned int index) {
    brls::Threading::sync([this, result, index]() {
        DataSourceDynamicVideoList* datasource =
            (DataSourceDynamicVideoList*)videoRecyclingGrid->getDataSource();
        if (datasource && index != 1) {
            datasource->appendData(result);
            videoRecyclingGrid->notifyDataChanged();
        } else {
            videoRecyclingGrid->setDataSource(
                new DataSourceDynamicVideoList(result));
        }
    });
}