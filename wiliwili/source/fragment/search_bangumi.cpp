//
// Created by fang on 2022/8/3.
//

#include <borealis/core/thread.hpp>

#include "fragment/search_bangumi.hpp"
#include "bilibili.h"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "activity/search_activity.hpp"
#include "fragment/search_tab.hpp"

SearchBangumi::SearchBangumi() {
    this->inflateFromXMLRes("xml/fragment/search_bangumi.xml");
    brls::Logger::debug("Fragment SearchBangumi: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemSearchPGCVideoCard::create(); });
    recyclingGrid->onNextPage([this]() { this->_requestSearch(SearchActivity::currentKey); });
}

SearchBangumi::~SearchBangumi() {
    brls::Logger::debug("Fragment SearchBangumiActivity: delete");
    this->recyclingGrid->clearData();
}

brls::View* SearchBangumi::create() { return new SearchBangumi(); }

void SearchBangumi::requestSearch(const std::string& key) {
    this->recyclingGrid->showSkeleton();
    this->requestIndex = 1;
    this->_requestSearch(key);
}

void SearchBangumi::_requestSearch(const std::string& key) {
    ASYNC_RETAIN
    BILI::search_video(
        key, "media_bangumi", requestIndex, "",
        [ASYNC_TOKEN](const bilibili::SearchResult& result) {
            for (const auto& i : result.result) {
                brls::Logger::verbose("search: {}", i.title);
            }
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                auto* datasource = dynamic_cast<DataSourceSearchPGCList*>(recyclingGrid->getDataSource());
                if (result.page != this->requestIndex) {
                    // 请求的顺序和当前需要的顺序不符
                    brls::Logger::error("请求的顺序和当前需要的顺序不符 {} /{}", result.page, this->requestIndex);
                    return;
                }
                this->requestIndex = result.page + 1;
                if (datasource && result.page != 1) {
                    if (result.result.empty()) {
                        // 搜索到底啦
                        brls::Logger::debug("搜索到底啦 {}", result.page);
                        return;
                    }
                    datasource->appendData(result.result);
                    recyclingGrid->notifyDataChanged();
                } else {
                    // 搜索加载的第一页
                    recyclingGrid->setDataSource(new DataSourceSearchPGCList(result.result));
                }
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("SearchBangumi: {}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->recyclingGrid->setError(error);
            });
        });
}