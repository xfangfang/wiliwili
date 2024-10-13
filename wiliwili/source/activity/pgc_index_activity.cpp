//
// Created by fang on 2022/8/24.
//

#include <pystring.h>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>
#include <borealis/views/applet_frame.hpp>

#include "activity/pgc_index_activity.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "view/auto_tab_frame.hpp"
#include "utils/image_helper.hpp"
#include "utils/activity_helper.hpp"
#include "analytics.h"

using namespace brls::literals;

/// 辅助映射 页面出现的顺序
const std::map<std::string, int> INDEX_TYPE_MAP  = {{"1", 0}, {"2", 1}, {"5", 2}, {"3", 3}, {"7", 4}, {"102", 5}};
const std::vector<std::string> INDEX_TYPE_VECTOR = {"1", "2", "5", "3", "7", "102"};

/// 辅助映射：index_type -> index name
const std::map<std::string, std::string> INDEX_TYPE_NAME_MAP = {{"1", "追番"},   {"2", "电影"}, {"5", "电视剧"},
                                                                {"3", "纪录片"}, {"7", "综艺"}, {"102", "影视综合"}};

/// 检索行中的一项

class IndexItem : public brls::Box {
public:
    IndexItem() {
        content = new brls::Label();
        if (brls::Application::ORIGINAL_WINDOW_HEIGHT < 720) {
            content->setFontSize(16);
            this->setHeight(28);
        } else {
            this->setHeight(36);
        }
        this->setHideHighlightBackground(true);
        this->setHighlightCornerRadius(6);
        this->setMargins(2, 6, 2, 6);
        this->setCornerRadius(4);
        this->setFocusable(true);
        this->content->setHorizontalAlign(brls::HorizontalAlign::CENTER);
        this->content->setMargins(0, 8, 0, 8);
        this->addView(this->content);
        this->addGestureRecognizer(new brls::TapGestureRecognizer(this));
    }

    void onFocusGained() override {
        // 不需要文字动画
        brls::View::onFocusGained();

        for (auto& c : this->getParent()->getChildren()) {
            if (c != this) {
                ((IndexItem*)c)->setActive(false);
            }
        }
        this->setActive(true);
    }

    void setActive(bool status = true) {
        auto theme = brls::Application::getTheme();
        if (status) {
            this->setBackgroundColor(theme.getColor("color/pink_1"));
            this->content->setTextColor(theme.getColor("color/bilibili"));
        } else {
            this->setBackgroundColor(nvgRGBA(0, 0, 0, 0));
            this->content->setTextColor(theme.getColor("brls/text"));
        }
    }

    void setText(const std::string& value) {
        if (this->content) this->content->setText(value);
    }

    brls::Label* content = nullptr;
};

/// 检索列表中的一行

class IndexRow : public brls::Box {
public:
    IndexRow(bilibili::PGCIndexFilter data) : data(data) {
        YGNodeStyleSetFlexWrap(this->ygNode, YGWrap::YGWrapWrap);
        if (brls::Application::ORIGINAL_WINDOW_HEIGHT < 720) {
            this->setMargins(2, 0, 2, 0);
        } else {
            this->setMargins(6, 0, 6, 0);
        }
        this->key = data.field;
        for (size_t i = 0; i < data.values.size(); i++) {
            auto item = new IndexItem();
            if (i == 0) {
                // 默认选中第一项，第一项与右侧诸项距离大
                item->setActive(true);
                item->setMarginRight(16);
            } else {
                item->setActive(false);
            }
            item->setText(data.values[i].name);
            this->addView(item);
        }
    }

    void onChildFocusGained(View* directChild, View* focusedView) override {
        auto& children = this->getChildren();
        for (size_t i = 0; i < children.size(); i++) {
            if (children[i] == focusedView) {
                this->selectedIndex = i;
            }
        }
        brls::Box::onChildFocusGained(directChild, focusedView);
    }

    size_t getSelectedIndex() { return this->selectedIndex; }

    IndexItemSinglePairData getData() { return std::make_pair(data.field, data.values[selectedIndex].keyword); }

    void setSelectedIndex(std::string value) {
        auto& children = this->getChildren();

        for (size_t i = 0; i < data.values.size(); i++) {
            if (data.values[i].keyword == value) {
                // 找到匹配的数据
                auto view = children[i];
                view->onFocusGained();
                return;
            }
        }
        if (children.size() > 0) children[0]->onFocusGained();
    }

    void setSelectedIndex(size_t value) {
        auto& children = this->getChildren();
        if (value < 0 || value >= children.size()) return;
        children[value]->onFocusGained();
    }

    size_t selectedIndex = 0;
    std::string key;
    bilibili::PGCIndexFilter data;
};

/// 检索列表

class IndexView : public brls::Box {
public:
    IndexView() {
        this->setAxis(brls::Axis::COLUMN);
        this->setMargins(10, 30, 0, 30);

        tabFrame = new AutoTabFrame();
        tabFrame->setGrow(1);
        tabFrame->setSideBarPosition(AutoTabBarPosition::TOP);
        auto theme = brls::Application::getTheme();
        tabFrame->setItemActiveTextColor(theme.getColor("color/bilibili"));
        tabFrame->setItemActiveBackgroundColor(theme.getColor("color/pink_1"));
        tabFrame->setItemDefaultBackgroundColor(theme.getColor("color/grey_1"));

        for (const auto& indexType : INDEX_TYPE_VECTOR) {
            auto& tab  = PGCIndexRequest::INDEX_FILTERS.at(indexType);
            auto* item = new AutoSidebarItem();
            item->setTabStyle(AutoTabBarStyle::PLAIN);
            item->setFontSize(20);
            item->setLabel(tab.index_name);
            item->setHeight(35);
            this->tabFrame->addTab(item, [tab]() {
                auto container = new AttachedView();
                container->setMargins(0, 20, 0, 20);
                container->setAxis(brls::Axis::COLUMN);
                for (auto& v : tab.filter) {
                    container->addView(new IndexRow(v));
                }
                container->setDefaultFocusedIndex(0);
                return container;
            });
        }

        tabFrame->setHideClickAnimation(true);
        tabFrame->registerAction("hints/back"_i18n, brls::BUTTON_B, [this](View* view) {
            this->close([this]() { this->dismiss(); });
            return true;
        });
        tabFrame->registerClickAction([this](...) {
            auto data = this->getData();
            for (const auto& i : data) {
                brls::Logger::debug("Got request params: {} / {}", i.first, i.second);
            }
            this->event.fire(data);
            this->close([this]() { this->dismiss(); });
            return true;
        });
        tabFrame->addGestureRecognizer(new brls::TapGestureRecognizer(tabFrame));

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

        this->addView(tabFrame);
        this->open();
    }

    ~IndexView() override { this->contentOffsetY.stop(); }

    void open(std::function<void()> cb = nullptr) {
        contentOffsetY.reset(-720.0f);
        this->startScrolling(0.0, 300, brls::EasingFunction::quadraticOut, cb);
    }

    void close(std::function<void()> cb = nullptr) {
        contentOffsetY.reset(0.0f);
        this->startScrolling(-720.0, 200, brls::EasingFunction::quadraticOut, cb);
    }

    UserRequestData getData() {
        UserRequestData res = {{"type", "2"}};

        for (brls::View* row : getIndexRows()) {
            auto* indexRow = dynamic_cast<IndexRow*>(row);
            if (!indexRow) continue;
            const auto& data = indexRow->getData();
            res[data.first]  = data.second;
        }
        res["index_type"] = INDEX_TYPE_VECTOR[tabFrame->getActiveIndex()];
        return res;
    }

    std::vector<View*>& getIndexRows() const { return ((AttachedView*)this->tabFrame->getActiveTab())->getChildren(); }

    // 打开检索表单时设置默认的tab与选项
    void setDefault(UserRequestData data) {
        if (data.count("index_type") == 0) return;  // 未包含 index_type
        std::string index_type = data["index_type"];
        if (PGCIndexRequest::INDEX_FILTERS.count(index_type) == 0) return;  //包含了不支持的 index_type

        int index = INDEX_TYPE_MAP.at(index_type);

        // 如果立刻设置焦点，会当成是当前页面的焦点，这样在返回时会导致焦点空指针
        brls::sync([this, index, index_type, data]() {
            this->tabFrame->focusTab(index);

            auto& items = PGCIndexRequest::INDEX_FILTERS[index_type].filter;  // index_type 分类下的数据

            // 设置默认数据，从后向前遍历保证最后一个选中的是最上面的一行
            auto& rows = this->getIndexRows();
            if (rows.size() != items.size()) {
                brls::Logger::error("错误的检索数据，页面行数: {} 数据行数: {}", rows.size(), items.size());
                return;
            }
            // 这里一定用int，不要再改回size_t了
            for (int j = items.size() - 1; j >= 0; j--) {
                auto& key      = items[j].field;
                auto* indexRow = dynamic_cast<IndexRow*>(rows[j]);
                if (!indexRow) continue;
                if (data.count(key) != 0) {
                    indexRow->setSelectedIndex(data.at(key));
                } else {
                    indexRow->setSelectedIndex(0);
                }
            }
        });
    }

    View* getDefaultFocus() override { return this->tabFrame->getSidebar(); }

    IndexChangeEvent* getIndexChangeEvent() { return &this->event; }

    void startScrolling(float newScroll, float time = 300,
                        brls::EasingFunction func = brls::EasingFunction::quadraticOut,
                        std::function<void()> cb  = nullptr) {
        if (newScroll == this->contentOffsetY) return;

        brls::Application::blockInputs();
        this->contentOffsetY.stop();
        this->contentOffsetY.reset();
        this->contentOffsetY.addStep(newScroll, time, func);
        this->contentOffsetY.setTickCallback([this] { this->tabFrame->setTranslationY(this->contentOffsetY); });

        this->contentOffsetY.setEndCallback([cb](bool finished) {
            if (cb) cb();
            brls::Application::unblockInputs();
        });
        this->contentOffsetY.start();
        this->invalidate();
    }

    AutoTabFrame* tabFrame;
    IndexChangeEvent event;
    brls::Animatable contentOffsetY = 0.0f;
};

class DataSourcePGCIndexVideoList : public RecyclingGridDataSource {
public:
    DataSourcePGCIndexVideoList(bilibili::PGCIndexListResult result) : list(std::move(result)) {}
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        //从缓存列表中取出 或者 新生成一个表单项
        RecyclingGridItemPGCVideoCard* item = (RecyclingGridItemPGCVideoCard*)recycler->dequeueReusableCell("Cell");

        auto& r = this->list[index];
        item->setCard(r.cover + ImageHelper::v_ext, r.title, r.index_show, r.badge_info, "", r.order);
        return item;
    }

    size_t getItemCount() override { return list.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        Intent::openSeasonBySeasonId(list[index].season_id);
    }

    void appendData(const bilibili::PGCIndexListResult& data) {
        this->list.insert(this->list.end(), data.begin(), data.end());
    }

    void clearData() override { this->list.clear(); }

private:
    bilibili::PGCIndexListResult list;
};

/// PGCIndexActivity

PGCIndexActivity::PGCIndexActivity(const std::string& url) {
    brls::Logger::debug("PGCIndexActivity: create");
    this->parseParam(url);
    GA("open_pgc_filter")
}

void PGCIndexActivity::onContentAvailable() {
    brls::Logger::debug("PGCIndexActivity: onContentAvailable");

    this->updateTitleBox();

    // 顶栏被点击 打开检索页面
    this->titleBox->registerClickAction([this](brls::View*) {
        this->openIndexActivity();
        return true;
    });

    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemPGCVideoCard::create(); });
    recyclingGrid->onNextPage([this]() { this->requestData(this->requestParam, false); });

    // 首次加载 请求过滤表单
    if (PGCIndexRequest::INDEX_FILTERS.empty()) this->requestPGCFilter();

    // 使用传入的参数直接加载
    brls::Logger::debug("PGCIndexActivity requestPGCIndex: {}", this->originParam);
    this->requestPGCIndex(this->originParam);

    titleBox->addGestureRecognizer(new brls::TapGestureRecognizer([this](brls::TapGestureStatus status, brls::Sound*) {
        if (status.state == brls::GestureState::END) {
            this->openIndexActivity();
        }
    }));

    this->registerAction("wiliwili/home/common/filter"_i18n, brls::ControllerButton::BUTTON_X,
                         [this](brls::View* view) {
                             this->openIndexActivity();
                             return true;
                         });
}

PGCIndexActivity::~PGCIndexActivity() {
    brls::Logger::debug("PGCIndexActivity: delete");
    this->alpha.stop();
}

void PGCIndexActivity::onPGCIndex(const bilibili::PGCIndexResultWrapper& result) {
    brls::sync([this, result]() {
        auto* datasource = dynamic_cast<DataSourcePGCIndexVideoList*>(recyclingGrid->getDataSource());
        if (datasource && result.num != 1) {
            if (!result.list.empty()) {
                datasource->appendData(result.list);
                recyclingGrid->notifyDataChanged();
            }
        } else {
            recyclingGrid->setDataSource(new DataSourcePGCIndexVideoList(result.list));
        }
    });
}

void PGCIndexActivity::onPGCFilter(const bilibili::PGCIndexFilters& result) {
    brls::sync([this]() { this->updateTitleBox(); });
}

void PGCIndexActivity::onError(const std::string& error) {
    brls::Logger::error("PGCIndexActivity::onError: {}", error);
    brls::sync([this]() { this->recyclingGrid->clearData(); });
}

// 解析用户选择的检索数据，返回一个 human-readable 字符串列表
std::vector<std::string> PGCIndexActivity::parseData(const UserRequestData& query) {
    if (query.count("index_type") == 0) return {"参数错误"};  // 未包含 index_type
    std::string type = query.at("index_type");
    if (INDEX_TYPE_NAME_MAP.count(type) == 0) return {"未知参数"};  //包含了不支持的 index_type

    if (PGCIndexRequest::INDEX_FILTERS.empty()) {
        // 从表中查询默认分类
        return {INDEX_TYPE_NAME_MAP.at(type), "skeleton", "skeleton", "skeleton", "skeleton", "skeleton", "skeleton"};
    }

    if (PGCIndexRequest::INDEX_FILTERS.count(type) == 0) return {"未知参数"};  //包含了不支持的 index_type

    bilibili::PGCIndexFilterWrapper& data = PGCIndexRequest::INDEX_FILTERS.at(type);

    std::vector<std::string> res = {data.index_name};

    for (auto& q : query) {
        for (auto& filter : data.filter) {
            if (q.first == filter.field) {
                for (size_t k = 0; k < filter.values.size(); k++) {
                    if (q.second == filter.values[k].keyword && k != 0) {
                        // 不添加选项
                        res.push_back(filter.values[k].name);
                        break;
                    }
                }
                break;
            }
        }
    }
    return res;
}

void PGCIndexActivity::parseParam(const std::string& url) {
    // parse request params
    try {
        std::vector<std::string> data;
        pystring::split(url, data, "?");  // url eg: "/page/home/pgc/more?type=2&index_type=2"
        this->originParam = data[1];
        std::vector<std::string> params;
        pystring::split(data[1], params, "&");
        for (auto p : params) {
            std::vector<std::string> d;
            pystring::split(p, d, "=");
            brls::Logger::debug("PGCIndexActivity url: {}/{}", d[0], d[1]);
            this->requestParam[d[0]] = d[1];
        }
        // 不设置排序方式(order)时，默认以综合排序进行请求
        if (this->requestParam.count("order") == 0) {
            this->originParam += "&order=8";
        }
    } catch (...) {
        brls::Logger::error("Cannot decode url: {}", url);
    }
}

void PGCIndexActivity::updateTitleBox() {
    auto list = this->parseData(requestParam);
    this->titleBox->clearViews();
    for (const auto& i : list) {
        if (i == "skeleton") {
            auto item = new SkeletonCell();
            item->setWidth(80);
            item->setMargins(4, 6, 4, 6);
            this->titleBox->addView(item);
        } else {
            auto item = new IndexItem();
            item->setText(i);
            item->setFocusable(false);
            item->setActive(true);
            this->titleBox->addView(item);
        }
    }
}

void PGCIndexActivity::openIndexActivity() {
    if (PGCIndexRequest::INDEX_FILTERS.empty()) return;
    brls::Application::giveFocus(this->titleBox);

    this->alpha.reset(1.0f);
    this->startAnimation(0.1f);

    auto indexView = new IndexView();
    indexView->setDefault(this->requestParam);
    indexView->getIndexChangeEvent()->subscribe([this](UserRequestData data) {
        this->recyclingGrid->showSkeleton(10);
        this->requestParam = data;
        this->requestData(this->requestParam, true);
        this->updateTitleBox();
    });
    auto container = new brls::AppletFrame(indexView);
    container->setHeaderVisibility(brls::Visibility::GONE);
    container->setFooterVisibility(brls::Visibility::GONE);
    container->setInFadeAnimation(true);
    brls::Application::pushActivity(new brls::Activity(container));
}

void PGCIndexActivity::startAnimation(float a) {
    if (a == this->alpha) return;
    brls::Application::blockInputs();
    this->alpha.stop();
    this->alpha.reset();
    this->alpha.addStep(a, 300, brls::EasingFunction::quadraticOut);
    this->alpha.setTickCallback([this] { this->setAlpha(this->alpha); });
    this->alpha.setEndCallback([](bool finished) { brls::Application::unblockInputs(); });
    this->alpha.start();
}