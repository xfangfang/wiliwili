//
// Created by 贾海峰 on 2022/8/20.
//

#pragma once

#include "view/recycling_grid.hpp"

class RecyclingGridItemHotsCard : public RecyclingGridItem {
public:
    RecyclingGridItemHotsCard();

    ~RecyclingGridItemHotsCard() override;

    void setCard(const std::string& prefix, const std::string& name, const std::string& image);

    void cacheForReuse() override;

    static RecyclingGridItemHotsCard* create();

private:
    BRLS_BIND(brls::Label, order, "hots/order");
    BRLS_BIND(brls::Label, content, "hots/content");
    BRLS_BIND(brls::Image, icon, "hots/icon");
};