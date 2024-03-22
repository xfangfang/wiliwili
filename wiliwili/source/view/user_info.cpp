//
// Created by fang on 2023/5/18.
//

#include <borealis/core/box.hpp>
#include <borealis/core/application.hpp>
#include <borealis/views/label.hpp>

#include "view/user_info.hpp"
#include "view/svg_image.hpp"
#include "utils/image_helper.hpp"

UserInfoView::UserInfoView() {
    this->inflateFromXMLRes("xml/views/user_info.xml");
    this->registerColorXMLAttribute("mainTextColor", [this](NVGcolor value) { this->setMainTextColor(value); });
}

UserInfoView::~UserInfoView() { ImageHelper::clear(this->avatarView); }

void UserInfoView::setUserInfo(const std::string& avatar, const std::string& username, const std::string& misc) {
    this->labeMisc->setText(misc);

    if (username.empty()) {
        this->labelUsername->setHeight(30);
    } else {
        this->labelUsername->setHeight(brls::View::AUTO);
        this->labelUsername->setText(username);
    }

    if (avatar.empty()) {
        this->avatarView->getParent()->setVisibility(brls::Visibility::GONE);
    } else {
        this->avatarView->getParent()->setVisibility(brls::Visibility::VISIBLE);
        ImageHelper::with(this->avatarView)->load(avatar);
    }
}

void UserInfoView::setMainTextColor(NVGcolor color) { this->labelUsername->setTextColor(color); }

brls::View* UserInfoView::create() { return new UserInfoView(); }

brls::Image* UserInfoView::getAvatar() { return this->avatarView; }

brls::Label* UserInfoView::getLabelName() { return this->labelUsername; }

brls::Label* UserInfoView::getLabelMisc() { return this->labeMisc; }

void UserInfoView::setHintType(InfoHintType type) {
    if (type == InfoHintType::NONE) {
        this->boxHint->setVisibility(brls::Visibility::GONE);
        return;
    }

    this->boxHint->setVisibility(brls::Visibility::VISIBLE);
    auto theme = brls::Application::getTheme();

    switch (type) {
        case InfoHintType::UP_FOLLOWING:
            this->boxHint->setBackgroundColor(theme.getColor("color/grey_1"));
            this->svgHint->setImageFromSVGRes("svg/bpx-svg-sprite-sort.svg");
            this->labeHint->setText("已关注");
            this->labeHint->setTextColor(theme.getColor("font/grey"));
            break;
        case InfoHintType::UP_NOT_FOLLOWED:
            this->boxHint->setBackgroundColor(theme.getColor("color/bilibili"));
            this->svgHint->setImageFromSVGRes("svg/bpx-svg-sprite-add.svg");
            this->labeHint->setText("关注");
            this->labeHint->setTextColor(theme.getColor("color/white"));
            break;
        case InfoHintType::BANGUMI_FOLLOWING:
            this->boxHint->setBackgroundColor(theme.getColor("color/grey_1"));
            this->svgHint->setImageFromSVGRes("svg/bpx-svg-sprite-sort.svg");
            this->labeHint->setText("已追番");
            this->labeHint->setTextColor(theme.getColor("font/grey"));
            break;
        case InfoHintType::BANGUMI_NOT_FOLLOWED:
            this->boxHint->setBackgroundColor(theme.getColor("color/bilibili"));
            this->svgHint->setImageFromSVGRes("svg/bpx-svg-sprite-add.svg");
            this->labeHint->setText("追番");
            this->labeHint->setTextColor(theme.getColor("color/white"));
            break;
        case InfoHintType::CINEMA_FOLLOWING:
            this->boxHint->setBackgroundColor(theme.getColor("color/grey_1"));
            this->svgHint->setImageFromSVGRes("svg/bpx-svg-sprite-sort.svg");
            this->labeHint->setText("已追剧");
            this->labeHint->setTextColor(theme.getColor("font/grey"));
            break;
        case InfoHintType::CINEMA_NOT_FOLLOWED:
            this->boxHint->setBackgroundColor(theme.getColor("color/bilibili"));
            this->svgHint->setImageFromSVGRes("svg/bpx-svg-sprite-add.svg");
            this->labeHint->setText("追剧");
            this->labeHint->setTextColor(theme.getColor("color/white"));
            break;

        default:
            break;
    }
}