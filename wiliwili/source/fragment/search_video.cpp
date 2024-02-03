//
// Created by fang on 2022/8/2.
//

#include <borealis/core/thread.hpp>

#include "fragment/search_video.hpp"
#include "bilibili.h"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "activity/search_activity.hpp"
#include "fragment/search_tab.hpp"

SearchVideo::SearchVideo() {
    this->inflateFromXMLRes("xml/fragment/search_video.xml");
    brls::Logger::debug("Fragment SearchVideo: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    recyclingGrid->onNextPage([this]() { this->_requestSearch(SearchActivity::currentKey); });

    this->registerStringXMLAttribute("order", [this](const std::string& value) { this->requestOrder = value; });
}

SearchVideo::~SearchVideo() {
    brls::Logger::debug("Fragment SearchVideoActivity: delete");
    this->recyclingGrid->clearData();
}

brls::View* SearchVideo::create() { return new SearchVideo(); }

void SearchVideo::requestSearch(const std::string& key) {
    this->recyclingGrid->showSkeleton();
    this->requestIndex = 1;
    this->_requestSearch(key);
}

void SearchVideo::_requestSearch(const std::string& key) {
    ASYNC_RETAIN
    BILI::search_video(
        key, "video", requestIndex, this->requestOrder,
        [ASYNC_TOKEN](const bilibili::SearchResult& result) {
            for (auto i : result.result) {
                brls::Logger::verbose("search: {}", i.title);
            }
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                auto* datasource = dynamic_cast<DataSourceSearchVideoList*>(recyclingGrid->getDataSource());
                if (result.page != this->requestIndex) {
                    // 请求的顺序和当前需要的顺序不符
                    brls::Logger::error("SearchVideo 顺序不符 {} /{}", result.page, this->requestIndex);
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
                    recyclingGrid->setDataSource(new DataSourceSearchVideoList(result.result));
                }
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("SearchVideo: {}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->recyclingGrid->setError(error);
            });
        });
}