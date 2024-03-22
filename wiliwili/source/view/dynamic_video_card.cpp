//
// Created by fang on 2024/3/22.
//

#include <borealis/views/image.hpp>
#include <borealis/views/label.hpp>

#include "view/dynamic_video_card.hpp"
#include "utils/image_helper.hpp"

DynamicVideoCardView::DynamicVideoCardView() { this->inflateFromXMLRes("xml/views/video_card_dynamic.xml"); }

void DynamicVideoCardView::setCard(const std::string& cover, const std::string& title, const std::string& count,
                                   const std::string& danmaku, const std::string& duration) {
    ImageHelper::with(picture)->load(cover);
    this->labelTitle->setText(title);
    this->labelCount->setText(count);
    this->labelDanmaku->setText(danmaku);
    this->labelDuration->setText(duration);
}

brls::View* DynamicVideoCardView::create() { return new DynamicVideoCardView(); }