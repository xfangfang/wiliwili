//
// Created by fang on 2022/8/3.
//

#include "fragment/search_bangumi.hpp"
#include "bilibili.h"
#include "view/recycling_grid.hpp"
#include "view/video_card.hpp"
#include "activity/player_activity.hpp"
#include "activity/search_activity.hpp"
#include "fragment/search_tab.hpp"

SearchBangumi::SearchBangumi() {
    this->inflateFromXMLRes("xml/fragment/search_bangumi.xml");
    brls::Logger::debug("Fragment SearchBangumi: create");
    recyclingGrid->registerCell("Cell", []() { return RecyclingGridItemVideoCard::create(); });
    recyclingGrid->onNextPage([this](){
        this->_requestSearch(SearchActivity::currentKey);
    });
    if(!SearchActivity::currentKey.empty()){
        this->_requestSearch(SearchActivity::currentKey);
    }
}

SearchBangumi::~SearchBangumi() {
    brls::Logger::debug("Fragment SearchBangumiActivity: delete");
}

brls::View *SearchBangumi::create() {
    return new SearchBangumi();
}

void SearchBangumi::requestSearch(const std::string& key){
    this->requestIndex = 1;
    this->_requestSearch(key);
}

void SearchBangumi::_requestSearch(const std::string& key){
    bilibili::BilibiliClient::search_video(key, "media_bangumi", requestIndex, "", [this](const bilibili::SearchResult& result){
        for(auto i: result.result){
            brls::Logger::debug("search: {}", i.title);
        }
        brls::sync([this, result](){
            DataSourceSearchVideoList* datasource = (DataSourceSearchVideoList *)recyclingGrid->getDataSource();
            brls::Logger::error("========search bangumi");
            if(result.page != this->requestIndex){
                // 请求的顺序和当前需要的顺序不符
                brls::Logger::error("请求的顺序和当前需要的顺序不符 {} /{}", result.page, this->requestIndex);
                return;
            }
            if(datasource && result.page != 1){
                if(result.result.empty()){
                    // 搜索到底啦
                    brls::Logger::debug("搜索到底啦 {}", result.page);
                    return;
                }
                datasource->appendData(result.result);
                recyclingGrid->notifyDataChanged();
            } else{
                AutoTabFrame::focus2Sidebar(this);
                recyclingGrid->setDataSource(new DataSourceSearchVideoList(result.result));
            }
            this->requestIndex = result.page + 1;

        });
    }, [](const std::string error){

    });
}