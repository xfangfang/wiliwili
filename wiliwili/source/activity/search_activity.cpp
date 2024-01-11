/**
 * Created by fang on 2022/6/9.
 */

#include <borealis/core/touch/tap_gesture.hpp>

#include "activity/search_activity.hpp"
#include "fragment/search_tab.hpp"
#include "fragment/search_hots.hpp"
#include "fragment/search_history.hpp"
#include "utils/config_helper.hpp"
#include "analytics.h"

using namespace brls::literals;

SearchActivity::SearchActivity(const std::string& key) {
    SearchActivity::currentKey = key;
    brls::Logger::debug("SearchActivity: create {}", key);
    GA("open_search", {{"key", key}})
}

void SearchActivity::onContentAvailable() {
    brls::Logger::debug("SearchActivity: onContentAvailable");
    if (!currentKey.empty()) {
        this->search(currentKey);
    }

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
    ProgramConfig::instance().addHistory(key);
    SearchActivity::currentKey = key;
    this->labelSearchKey->setText(key);
    this->searchTab->requestData(key);
}

SearchActivity::~SearchActivity() { brls::Logger::debug("SearchActivity: delete"); }

UpdateSearchEvent* SearchActivity::getUpdateSearchEvent() { return &this->updateSearchEvent; }
