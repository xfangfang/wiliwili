//
// Created by fang on 2023/4/25.
//

#pragma once

#include <view/auto_tab_frame.hpp>
#include "activity/search_activity.hpp"

typedef brls::Event<std::string> UpdateSearchEvent;

class RecyclingGrid;

class SearchHistory : public AttachedView {
public:
    SearchHistory();

    ~SearchHistory() override;

    static View *create();

    void requestHistory();

    RecyclingGrid *getRecyclingGrid();

    void setSearchCallback(UpdateSearchEvent *event);

private:
    UpdateSearchEvent *updateSearchEvent = nullptr;
    brls::Event<> clearSearchEvent;
    BRLS_BIND(RecyclingGrid, recyclingGrid, "search/history/recyclingGrid");
};
