//
// Created by fang on 2022/7/14.
//

#include "fragment/home_bangumi.hpp"
#include "view/auto_tab_frame.hpp"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"


HomeBangumi::HomeBangumi() {
    this->inflateFromXMLRes("xml/fragment/home_bangumi.xml");
    brls::Logger::debug("Fragment HomeBangumi: create");

    this->requestData();
}

void HomeBangumi::onCreate() {
    this->registerTabAction("刷新", brls::ControllerButton::BUTTON_X, [this](brls::View* view)-> bool {
        this->requestData();
        return true;
    });
}

void HomeBangumi::onBangumiList(const bilibili::PGCModuleListResult &result, int has_next){

    brls::sync([this, result](){

        //避免刷新时焦点在tabFrame的tab栏上因为销毁组件导致出错，所以刷新前将焦点向上移动一级
        AutoTabFrame::focus2Sidebar(this);
        this->tabFrame->clearTabs();

        for(auto i: result){
            if(i.items.size() > 0){
                AutoSidebarItem* item = new AutoSidebarItem();
                item->setTabStyle(AutoTabBarStyle::PLAIN);
                item->setLabel(i.title);
                item->setFontSize(14);
                this->tabFrame->addTab(item, [i](){
                    auto container = new AttachedView();
                    container->setMarginTop(12);
                    auto grid = new RecyclingGrid();
                    grid->setPadding(0, 10, 0, 20);
                    grid->setGrow(1);
                    if(i.style.compare("double_feed") == 0 || i.style.compare("follow") == 0){
                        // 封面横图
                        grid->applyXMLAttribute("itemSpace", "20");
                        grid->applyXMLAttribute("spanCount", "4");
                        grid->applyXMLAttribute("itemHeight", "200");
                        grid->registerCell("Cell", []() { return RecyclingGridItemPGCVideoCard::create(false); });

                        // todo: 猜你喜欢页面加载下一页
                    } else {
                        // 封面竖图
                        grid->applyXMLAttribute("itemSpace", "31.4");
                        grid->applyXMLAttribute("spanCount", "5");
                        grid->applyXMLAttribute("itemHeight", "320");
                        grid->registerCell("Cell", []() { return RecyclingGridItemPGCVideoCard::create(true); });

                        // todo: 最后一项替换为"查看全部"，点击后进入番剧筛选页面
                    }

                    container->addView(grid);
                    grid->setDataSource(new DataSourcePGCVideoList(i.items));
                    return container;
                });
            }
        }
    });
}

HomeBangumi::~HomeBangumi() {
    brls::Logger::debug("Fragment HomeBangumiActivity: delete");
}

brls::View *HomeBangumi::create() {
    return new HomeBangumi();
}