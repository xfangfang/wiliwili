//
// Created by 贾海峰 on 2022/8/20.
//

#pragma once

#include <view/auto_tab_frame.hpp>
#include "activity/search_activity.hpp"

typedef brls::Event<std::string> UpdateSearchEvent;

class RecyclingGrid;

class SearchHots : public AttachedView {
public:
    SearchHots();

    ~SearchHots() override;

    static View *create();

    void requestSearch();

    RecyclingGrid *getRecyclingGrid();

    void setSearchCallback(UpdateSearchEvent *event);

private:
    UpdateSearchEvent *updateSearchEvent = nullptr;
    BRLS_BIND(RecyclingGrid, recyclingGrid, "search/hots/recyclingGrid");
};
