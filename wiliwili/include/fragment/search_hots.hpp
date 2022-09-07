//
// Created by 贾海峰 on 2022/8/20.
//

#pragma once

#include <borealis.hpp>
#include <view/auto_tab_frame.hpp>
#include "activity/search_activity.hpp"

typedef brls::Event<std::string> UpdateSearchEvent;

class RecyclingGrid;
class SearchActivity;

class SearchHots : public AttachedView {
public:
    SearchHots();

    ~SearchHots();

    static View *create();

    void requestSearch();

    SearchActivity *searchActivity;

    UpdateSearchEvent *updateSearchEvent;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "search/hots/recyclingGrid");
};
