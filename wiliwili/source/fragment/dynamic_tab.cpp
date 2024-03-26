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
#include "view/dynamic_article.hpp"
#include "utils/image_helper.hpp"
#include "utils/activity_helper.hpp"

using namespace brls::literals;

class DynamicUserInfoView : public RecyclingGridItem {
public:
    explicit DynamicUserInfoView(const std::string& xml) {
        this->inflateFromXMLRes(xml);
        auto theme = brls::Application::getTheme();
        selectedColor = theme.getColor("color/bilibili");
        fontColor = theme.getColor("brls/text");
    }

    void setUserInfo(const std::string& avatar, const std::string& username, bool isUpdate = false) {
        this->labelUsername->setText(username);
        ImageHelper::with(this->avatarView)->load(avatar);
    }

    void setNotice(bool show) {
        this->notice->setVisibility(show ? brls::Visibility::VISIBLE : brls::Visibility::INVISIBLE);
    }

    void setSelected(bool selected) {
        this->labelUsername->setTextColor(selected ? selectedColor: fontColor);
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
    BRLS_BIND(brls::Rectangle, notice, "notice");
    BRLS_BIND(brls::Image, avatarView, "avatar");
    BRLS_BIND(brls::Label, labelUsername, "username");
    NVGcolor selectedColor{};
    NVGcolor fontColor{};
};

class DataSourceUpList : public RecyclingGridDataSource {
public:
    explicit DataSourceUpList(bilibili::DynamicUpListResult result) : list(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        if (index == 0) {
            DynamicUserInfoView* item = (DynamicUserInfoView*)recycler->dequeueReusableCell("CellAll");
            item->setSelected(selectedIndex == index);
            return item;
        }

        //从缓存列表中取出 或者 新生成一个表单项
        DynamicUserInfoView* item = (DynamicUserInfoView*)recycler->dequeueReusableCell("Cell");

        auto& r = this->list[index - 1];
        item->setUserInfo(r.face + ImageHelper::face_ext, r.uname);
        item->setNotice(r.has_update);
        item->setSelected(selectedIndex == index);
        return item;
    }

    size_t getItemCount() override { return list.size() + 1; }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        // 取消选中
        std::vector<RecyclingGridItem*>& items = recycler->getGridItems();
        for (auto& i : items) {
            auto* cell = dynamic_cast<DynamicUserInfoView*>(i);
            if (cell) cell->setSelected(false);
        }

        selectedIndex = index;
        if (index == 0) {
            // 选择全部
            userSelectedEvent.fire(0);
            auto* item = dynamic_cast<DynamicUserInfoView*>(recycler->getGridItemByIndex(index));
            if (!item) return;
            item->setSelected(true);
        } else {
            // 选择具体的某个up主
            userSelectedEvent.fire(list[index - 1].mid);
            // 移除小红点
            auto* item = dynamic_cast<DynamicUserInfoView*>(recycler->getGridItemByIndex(index));
            if (!item) return;
            this->list[index - 1].has_update = false;
            item->setNotice(false);
            item->setSelected(true);
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
    size_t selectedIndex = 0;
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

class DataSourceDynamicArticleList : public RecyclingGridDataSource {
public:
    explicit DataSourceDynamicArticleList(bilibili::DynamicArticleListResult result) : list(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        auto* item = (DynamicArticleView*)recycler->dequeueReusableCell("Cell");
        auto& r = this->list[index];
        item->setCard(r);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        auto* item = dynamic_cast<DynamicArticleView*>(recycler->getGridItemByIndex(index));
        if (!item) return;
        item->openDetail();
    }

    void appendData(const bilibili::DynamicArticleListResult& data) {
        bool skip = false;
        for (const auto& i : data) {
            skip = false;
            for (const auto& j : this->list) {
                if (j.id_str == i.id_str) {
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
    bilibili::DynamicArticleListResult list;
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
        this->DynamicVideoRequest::requestData(false, DynamicRequestMode::Video);
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
            this->DynamicVideoRequest::requestData(true, DynamicRequestMode::Article);
            this->DynamicVideoRequest::requestData(true, DynamicRequestMode::Video);
        }
    });

    // 初始化右侧图文列表
    articleRecyclingGrid->registerCell("Cell", []() { return DynamicArticleView::create(); });
    articleRecyclingGrid->onNextPage([this]() {
        //自动加载下一页
        this->DynamicVideoRequest::requestData(false, DynamicRequestMode::Article);
    });
    articleRecyclingGrid->setRefreshAction([this]() {
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
            this->articleRecyclingGrid->showSkeleton();
            //请求刷新数据
            this->DynamicVideoRequest::requestData(true, DynamicRequestMode::Article);
            this->DynamicVideoRequest::requestData(true, DynamicRequestMode::Video);
        }
    });
    // 如果显示为小屏模式，调整卡片尺寸
    if (brls::Application::ORIGINAL_WINDOW_HEIGHT == 544) {
        articleRecyclingGrid->setPaddingLeft(100);
        articleRecyclingGrid->setPaddingRight(100);
    }

    this->tabFrame->setTabChangedAction([this](size_t index) {
        if (index == 0) {
            if (!tempVideoList.empty()) {
                this->videoRecyclingGrid->showSkeleton();
                this->onDynamicVideoList(tempVideoList, 1);
                tempVideoList.clear();
            }
        } else {
            if (!tempArticleList.empty()) {
                this->articleRecyclingGrid->showSkeleton();
                this->onDynamicArticleList(tempArticleList, 1);
                tempArticleList.clear();
            }
        }
    });

    this->registerAction(
        "上一项", brls::ControllerButton::BUTTON_LB,
        [this](brls::View* view) -> bool {
            tabFrame->focus2LastTab();
            return true;
        },
        true);

    this->registerAction(
        "下一项", brls::ControllerButton::BUTTON_RB,
        [this](brls::View* view) -> bool {
            tabFrame->focus2NextTab();
            return true;
        },
        true);


    // 获取动态
    this->DynamicVideoRequest::requestData(true, DynamicRequestMode::Video);
    this->DynamicVideoRequest::requestData(true, DynamicRequestMode::Article);
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
    brls::sync(
        [this, error]() { upRecyclingGrid->setDataSource(new DataSourceUpList(bilibili::DynamicUpListResult())); });
}

void DynamicTab::onVideoError(const std::string& error) {
    brls::Logger::error("DynamicTab::onVideoError {}", error);
    brls::sync([this, error]() { videoRecyclingGrid->setError(error); });
}

void DynamicTab::onArticleError(const std::string& error) {
    brls::Logger::error("DynamicTab::onArticleError {}", error);
    brls::sync([this, error]() { articleRecyclingGrid->setError(error); });
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
    this->articleRecyclingGrid->registerAction("wiliwili/home/common/refresh"_i18n, brls::ControllerButton::BUTTON_X,
                                             [this](brls::View* view) -> bool {
                                                 this->articleRecyclingGrid->refresh();
                                                 return true;
                                             });
}

void DynamicTab::changeUser(int64_t mid) {
    this->setCurrentUser(mid);
    this->videoRecyclingGrid->showSkeleton(tabFrame->getActiveIndex() == 0 ? 30 : 0);
    this->DynamicVideoRequest::requestData(true, DynamicRequestMode::Video);
    this->articleRecyclingGrid->showSkeleton(tabFrame->getActiveIndex() == 0 ? 0 : 30);
    this->DynamicVideoRequest::requestData(true, DynamicRequestMode::Article);
}

// 获取到动态视频
void DynamicTab::onDynamicVideoList(const bilibili::DynamicVideoListResult& result, unsigned int index) {
    brls::Threading::sync([this, result, index]() {
        if (tabFrame->getActiveIndex() == 1) {
            // 当前页面被隐藏
            if(index == 1) tempVideoList = result;
            else brls::Logger::error("video list is hidden, but got data: {}", index);
            return;
        }
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

// 获取到动态图文
void DynamicTab::onDynamicArticleList(const bilibili::DynamicArticleListResult& result, unsigned int index) {
    brls::Threading::sync([this, result, index]() {
        if (tabFrame->getActiveIndex() == 0) {
            // 当前页面被隐藏
            if (index == 1) tempArticleList = result;
            else brls::Logger::error("article list is hidden, but got data: {}", index);
            return;
        }
        auto* datasource = dynamic_cast<DataSourceDynamicArticleList*>(articleRecyclingGrid->getDataSource());
        if (datasource && index != 1) {
            if (!result.empty()) {
                datasource->appendData(result);
                articleRecyclingGrid->notifyDataChanged();
            }
        } else {
            articleRecyclingGrid->setDataSource(new DataSourceDynamicArticleList(result));
        }
    });
}