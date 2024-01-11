//
// Created by fang on 2022/7/14.
//

#include <borealis/core/thread.hpp>

#include "fragment/home_bangumi.hpp"
#include "view/auto_tab_frame.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"

using namespace brls::literals;

HomeBangumi::HomeBangumi() {
    this->inflateFromXMLRes("xml/fragment/home_bangumi.xml");
    brls::Logger::debug("Fragment HomeBangumi: create");
    this->tabFrame->setRefreshAction([this]() {
        AutoTabFrame::focus2Sidebar(this);
        this->tabFrame->clearTabs();
        this->requestData();
    });
    this->requestData();
}

void HomeBangumi::onCreate() {
    this->registerTabAction("wiliwili/home/common/refresh"_i18n, brls::ControllerButton::BUTTON_X,
                            [this](brls::View* view) -> bool {
                                this->tabFrame->refresh();
                                return true;
                            });

    this->registerTabAction(
        "上一项", brls::ControllerButton::BUTTON_LT,
        [this](brls::View* view) -> bool {
            tabFrame->focus2LastTab();
            return true;
        },
        true);

    this->registerTabAction(
        "下一项", brls::ControllerButton::BUTTON_RT,
        [this](brls::View* view) -> bool {
            tabFrame->focus2NextTab();
            return true;
        },
        true);
}

void HomeBangumi::onBangumiList(const bilibili::PGCResultWrapper& result) {
    if (this->refresh_flag == 1 && this->tabFrame->getActiveTab() != nullptr && !result.modules.empty()) {
        // 加载的是 猜你喜欢的第N页
        auto tab  = (AttachedView*)this->tabFrame->getActiveTab();  // 猜你喜欢页面
        auto grid = (RecyclingGrid*)tab->getChildren()[0];

        auto* datasource = (DataSourcePGCVideoList*)grid->getDataSource();
        if (!result.modules[0].items.empty()) {
            datasource->appendData(result.modules[0].items);
            grid->notifyDataChanged();
        }

        return;
    }

    brls::sync([this, result]() {
        int span4 = brls::Application::getStyle().getMetric("wiliwili/grid/span/4");
        int span5 = brls::Application::getStyle().getMetric("wiliwili/grid/span/5");

        for (const auto& i : result.modules) {
            if (i.items.empty()) continue;
            auto* item = new AutoSidebarItem();
            item->setTabStyle(AutoTabBarStyle::PLAIN);
            item->setLabel(i.title);
            item->setFontSize(18);
            this->tabFrame->addTab(item, [this, i, span4, span5]() {
                auto container = new AttachedView();
                container->setMarginTop(12);
                auto grid = new RecyclingGrid();
                grid->setPadding(0, 10, 0, 20);
                grid->setGrow(1);
                if (i.style == "double_feed" || i.style == "follow") {
                    // 封面横图
                    grid->applyXMLAttribute("itemSpace", "20");
                    grid->spanCount = span4;
                    grid->applyXMLAttribute("itemHeight", "200");
                    grid->registerCell("Cell", []() { return RecyclingGridItemPGCVideoCard::create(false); });
                    grid->registerCell("CellMore", []() { return RecyclingGridItemViewMoreCard::create(false); });

                    // 猜你喜欢tab 监控下一页请求
                    if (i.style == "double_feed") {
                        grid->onNextPage([this]() { this->requestData(false); });
                    }
                } else {
                    // 封面竖图
                    grid->applyXMLAttribute("itemSpace", "31.4");
                    grid->spanCount = span5;
                    grid->applyXMLAttribute("itemHeight", "320");
                    grid->registerCell("Cell", []() { return RecyclingGridItemPGCVideoCard::create(true); });
                    grid->registerCell("CellMore", []() { return RecyclingGridItemViewMoreCard::create(true); });
                }

                container->addView(grid);
                grid->setDataSource(new DataSourcePGCVideoList(i));
                return container;
            });
        }
    });
}

HomeBangumi::~HomeBangumi() { brls::Logger::debug("Fragment HomeBangumiActivity: delete"); }

brls::View* HomeBangumi::create() { return new HomeBangumi(); }

void HomeBangumi::onError(const std::string& error) {}