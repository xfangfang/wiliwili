//
// Created by 贾海峰 on 2022/8/20.
//

#include "view/hots_card.hpp"
#include "utils/image_helper.hpp"

RecyclingGridItemHotsCard::RecyclingGridItemHotsCard() {
    this->inflateFromXMLRes("xml/views/hots_card.xml");
}

RecyclingGridItemHotsCard::~RecyclingGridItemHotsCard() {

}

void RecyclingGridItemHotsCard::setCard(int order, std::string showName, std::string pic) {
    this->order->setVisibility(brls::Visibility::VISIBLE);
    this->order->setText(std::to_string(order));
    this->content->setVisibility(brls::Visibility::VISIBLE);
    this->content->setText(showName);
//    this->content->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    this->icon->setVisibility(brls::Visibility::VISIBLE);
    if (pic.size())
        ImageHelper::with(this)->load(pic)->into(this->icon);
//    this->icon->s
}

RecyclingGridItemHotsCard *RecyclingGridItemHotsCard::create() {
    return new RecyclingGridItemHotsCard;
}
