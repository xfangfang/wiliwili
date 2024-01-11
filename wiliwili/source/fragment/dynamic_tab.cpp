//
// Created by fang on 2022/6/9.
//

#include <utility>
#include <borealis/core/thread.hpp>

#include "fragment/dynamic_tab.hpp"
#include "view/auto_tab_frame.hpp"
#include "view/recycling_grid.hpp"
#include "view/svg_image.hpp"
#include "view/video_card.hpp"
#include "utils/image_helper.hpp"
#include "utils/activity_helper.hpp"

using namespace brls::literals;

class DynamicUserInfoView : public RecyclingGridItem {
public:
    explicit DynamicUserInfoView(const std::string& xml) { this->inflateFromXMLRes(xml); }

    void setUserInfo(const std::string& avatar, const std::string& username, bool isUpdate = false) {
        this->labelUsername->setText(username);
        ImageHelper::with(this->avatarView)->load(avatar);
    }

    brls::Image* getAvatar() { return this->avatarView; }

    void prepareForReuse() override {}

    void cacheForReuse() override {
        if (!dynamic_cast<SVGImage*>(this->avatarView.getView())) ImageHelper::clear(this->avatarView);
    }

    static RecyclingGridItem* create(const std::string& xml = "xml/views/user_info_dynamic.xml") {
        return new DynamicUserInfoView(xml);
    }

private:
    BRLS_BIND(brls::Image, avatarView, "avatar");
    BRLS_BIND(brls::Label, labelUsername, "username");
};

class DataSourceUpList : public RecyclingGridDataSource {
public:
    explicit DataSourceUpList(bilibili::DynamicUpListResult result) : list(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        if (index == 0) {
            DynamicUserInfoView* item = (DynamicUserInfoView*)recycler->dequeueReusableCell("CellAll");
            return item;
        }

        //从缓存列表中取出 或者 新生成一个表单项
        DynamicUserInfoView* item = (DynamicUserInfoView*)recycler->dequeueReusableCell("Cell");

        auto& r = this->list[index - 1];
        item->setUserInfo(r.user_profile.info.face + ImageHelper::face_ext, r.user_profile.info.uname);
        return item;
    }

    size_t getItemCount() override { return list.size() + 1; }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
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

    void clearData() override { this->list.clear(); }

private:
    bilibili::DynamicUpListResult list;
    UserSelectedEvent userSelectedEvent;
};

class DataSourceDynamicVideoList : public RecyclingGridDataSource {
public:
    explicit DataSourceDynamicVideoList(bilibili::DynamicVideoListResult result) : list(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemVideoCard* item = (RecyclingGridItemVideoCard*)recycler->dequeueReusableCell("Cell");

        auto& r = this->list[index];
        item->setCard(r.pic + ImageHelper::h_ext, r.title, r.owner.name, r.pubdate, r.stat.view, r.stat.danmaku,
                      r.duration);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override { Intent::openBV(list[index].bvid); }

    void appendData(const bilibili::DynamicVideoListResult& data) {
        bool skip = false;
        for (const auto& i : data) {
            skip = false;
            for (const auto& j : this->list) {
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

    void clearData() override { this->list.clear(); }

private:
    bilibili::DynamicVideoListResult list;
};

DynamicTab::DynamicTab() {
    this->inflateFromXMLRes("xml/fragment/dynamic_tab.xml");
    brls::Logger::debug("Fragment DynamicTab: create");

    // 初始化左侧Up主列表
    upRecyclingGrid->registerCell("Cell", []() { return DynamicUserInfoView::create(); });
    upRecyclingGrid->registerCell("CellAll",
                                  []() { return DynamicUserInfoView::create("xml/views/user_info_dynamic_all.xml"); });
    upRecyclingGrid->setDataSource(new DataSourceUpList(bilibili::DynamicUpListResult()));
    this->DynamicTabRequest::requestData();

    // 初始化右侧视频列表
    videoRecyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    videoRecyclingGrid->onNextPage([this]() {
        //自动加载下一页
        this->DynamicVideoRequest::requestData();
    });
    videoRecyclingGrid->setRefreshAction([this]() {
        if (currentUser == 0) {
            // 全部动态页，同时刷新up主列表和视频内容
            AutoTabFrame::focus2Sidebar(this);
            this->upRecyclingGrid->showSkeleton();
            this->DynamicTabRequest::requestData();
            this->changeUser(0);
        } else {
            //焦点转移到UP主列表
            brls::Application::giveFocus(this->upRecyclingGrid);
            //展示骨架屏
            this->videoRecyclingGrid->showSkeleton();
            //请求刷新数据
            this->DynamicVideoRequest::requestData(true);
        }
    });
    this->DynamicVideoRequest::requestData();
}

DynamicTab::~DynamicTab() { brls::Logger::debug("Fragment DynamicTabActivity: delete"); }

brls::View* DynamicTab::create() { return new DynamicTab(); }

void DynamicTab::onUpList(const bilibili::DynamicUpListResultWrapper& result) {
    brls::Threading::sync([this, result]() {
        auto dataSource = new DataSourceUpList(result.items);
        upRecyclingGrid->setDataSource(dataSource);
        dataSource->getSelectedEvent()->subscribe([this](int64_t mid) { this->changeUser(mid); });
    });
}

void DynamicTab::onError(const std::string& error) {
    brls::Logger::error("DynamicTab::onError {}", error);
    brls::sync([this, error]() {
        videoRecyclingGrid->setError(error);
        upRecyclingGrid->setDataSource(new DataSourceUpList(bilibili::DynamicUpListResult()));
    });
}

void DynamicTab::onCreate() {
    this->registerTabAction("wiliwili/activity/refresh"_i18n, brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                this->setCurrentUser(0);
                                this->videoRecyclingGrid->refresh();
                                return true;
                            });
    this->videoRecyclingGrid->registerAction("wiliwili/home/common/refresh"_i18n, brls::ControllerButton::BUTTON_X,
                                             [this](brls::View* view) -> bool {
                                                 this->videoRecyclingGrid->refresh();
                                                 return true;
                                             });
}

void DynamicTab::changeUser(int64_t mid) {
    this->setCurrentUser(mid);
    this->videoRecyclingGrid->showSkeleton();
    this->DynamicVideoRequest::requestData(true);
}

// 获取到动态视频
void DynamicTab::onDynamicVideoList(const bilibili::DynamicVideoListResult& result, unsigned int index) {
    brls::Threading::sync([this, result, index]() {
        auto* datasource = dynamic_cast<DataSourceDynamicVideoList*>(videoRecyclingGrid->getDataSource());
        if (datasource && index != 1) {
            if (!result.empty()) {
                datasource->appendData(result);
                videoRecyclingGrid->notifyDataChanged();
            }
        } else {
            videoRecyclingGrid->setDataSource(new DataSourceDynamicVideoList(result));
        }
    });
}