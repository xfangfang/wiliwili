//
// Created by fang on 2022/6/9.
//

#include <borealis.hpp>
#include <borealis/platforms/switch/swkbd.hpp>
#include "fragment/home_tab.hpp"
#include "activity/search_activity.hpp"

HomeTab::HomeTab() {
    this->inflateFromXMLRes("xml/fragment/home_tab.xml");
    brls::Logger::debug("Fragment HomeTab: create");

}

void HomeTab::onCreate() {
    this->registerTabAction("搜索", brls::ControllerButton::BUTTON_Y,
        [](brls::View* view)-> bool {
            brls::Swkbd::openForText([](std::string text) {
                                   brls::Application::pushActivity(new SearchActivity(text));
                               }, "搜索你感兴趣的视频", "", 32, "", 0);

        return true;
     });

    this->registerTabAction("上一项", brls::ControllerButton::BUTTON_LB,
                            [this](brls::View* view)-> bool {
                                tabFrame->focus2LastTab();
                                return true;
                        }, true);

    this->registerTabAction("下一项", brls::ControllerButton::BUTTON_RB,
                            [this](brls::View* view)-> bool {
                                tabFrame->focus2NextTab();
                                return true;
                            }, true);

    this->search->addGestureRecognizer(new brls::TapGestureRecognizer(this->search, [](){
        brls::Swkbd::openForText([](std::string text) {
            brls::Application::pushActivity(new SearchActivity(text));
        }, "搜索你感兴趣的视频", "", 32, "", 0);
    }));
}

brls::View* HomeTab::create(){
    return new HomeTab();
}

HomeTab::~HomeTab() {
    brls::Logger::debug("Fragment HomeTab: delete");
}