//
// Created by 贾海峰 on 2022/8/20.
//

#pragma once

#include <borealis.hpp>
#include "view/recycling_grid.hpp"

class RecyclingGridItemHotsCard: public RecyclingGridItem {
public:
    RecyclingGridItemHotsCard();

    ~RecyclingGridItemHotsCard();

    void setCard(int order, std::string showName, std::string icon);

    static RecyclingGridItemHotsCard* create();

private:
    BRLS_BIND(brls::Label, order, "hots/order");
    BRLS_BIND(brls::Label, content, "hots/content");
    BRLS_BIND(brls::Image, icon, "hots/icon");

};