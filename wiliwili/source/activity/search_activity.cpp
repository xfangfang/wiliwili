/**
 * Created by fang on 2022/6/9.
 */

#include <borealis/core/touch/tap_gesture.hpp>

#include "activity/search_activity.hpp"
#include "fragment/search_tab.hpp"
#include "fragment/search_hots.hpp"
#include "fragment/search_history.hpp"
#include "fragment/search_order.hpp"
#include "utils/config_helper.hpp"
#include "utils/event_helper.hpp"
#include "analytics.h"

using namespace brls::literals;

SearchActivity::SearchActivity(const std::string& key) {
    SearchActivity::currentKey = key;
    brls::Logger::debug("SearchActivity: create {}", key);
    GA("open_search", {{"key", key}})
}

void SearchActivity::onContentAvailable() {
    brls::Logger::debug("SearchActivity: onContentAvailable");
    this->requestSearch(SearchActivity::currentKey);

    this->registerAction(
        "wiliwili/search/tab"_i18n, brls::ControllerButton::BUTTON_Y, [this](brls::View* view) -> bool {
            brls::Application::getImeManager()->openForText([&](const std::string& text) { this->search(text); },
                                                            "wiliwili/home/common/search"_i18n, "", 32,
                                                            SearchActivity::currentKey, 0);
            return true;
        });

    this->searchBox->addGestureRecognizer(new brls::TapGestureRecognizer(this->searchBox, [this]() {
        brls::Application::getImeManager()->openForText([&](const std::string& text) { this->search(text); },
                                                        "wiliwili/home/common/search"_i18n, "", 32,
                                                        SearchActivity::currentKey, 0);
    }));

    this->getUpdateSearchEvent()->subscribe([this](const std::string& s) { this->search(s); });
    this->searchTab->getSearchHotsTab()->setSearchCallback(&updateSearchEvent);
    this->searchTab->getSearchHistoryTab()->setSearchCallback(&updateSearchEvent);
}

void SearchActivity::search(const std::string& key) {
    if (key.empty()) return;
    SEARCH_E->fire(SEARCH_KEY, (void*)key.c_str());
}

void SearchActivity::requestSearch(const std::string& key) {
    if (key.empty()) return;
    ProgramConfig::instance().addHistory(key);
    SearchActivity::currentKey = key;
    this->labelSearchKey->setText(key);
    // SearchActivity 会最先触发搜索事件，在这里调整页面到默认的搜索页
    // 搜索事件在 SearchActivity 下的其他页面触发时，会根据他当前的显示状态来决定是否立刻进行搜索
    // 由此实现，重新搜索时只加载默认搜索页的结果，减少不必要的网络请求
    this->searchTab->focusNthTab(2);
    this->searchTab->getSearchVideoTab()->focusNthTab(0);
    this->searchTab->getSearchHistoryTab()->requestHistory();
}

SearchActivity::~SearchActivity() { brls::Logger::debug("SearchActivity: delete"); }

UpdateSearchEvent* SearchActivity::getUpdateSearchEvent() { return &this->updateSearchEvent; }
