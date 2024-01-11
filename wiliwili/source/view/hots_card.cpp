//
// Created by 贾海峰 on 2022/8/20.
//

#include "view/hots_card.hpp"
#include "utils/image_helper.hpp"

RecyclingGridItemHotsCard::RecyclingGridItemHotsCard() { this->inflateFromXMLRes("xml/views/hots_card.xml"); }

RecyclingGridItemHotsCard::~RecyclingGridItemHotsCard() = default;

void RecyclingGridItemHotsCard::setCard(const std::string& prefix, const std::string& name, const std::string& image) {
    this->order->setText(prefix);
    this->content->setText(name);
    if (image.empty()) {
        this->icon->setVisibility(brls::Visibility::GONE);
    } else {
        this->icon->setVisibility(brls::Visibility::VISIBLE);
        ImageHelper::with(this->icon)->load(image);
    }
}

void RecyclingGridItemHotsCard::cacheForReuse() { ImageHelper::clear(this->icon); }

RecyclingGridItemHotsCard* RecyclingGridItemHotsCard::create() { return new RecyclingGridItemHotsCard(); }
