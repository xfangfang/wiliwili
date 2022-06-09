/**
 * Created by fang on 2022/6/9.
 */

#include "activity/search_activity.hpp"

SearchActivity::SearchActivity() {
    brls::Logger::debug("SearchActivity: create");
}

void SearchActivity::onContentAvailable() {
    brls::Logger::debug("SearchActivity: onContentAvailable");
}

SearchActivity::~SearchActivity() {
    brls::Logger::debug("SearchActivity: delete");
}