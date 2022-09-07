//
// Created by fang on 2022/5/13.
//

#pragma once

#include <borealis.hpp>
#include "view/svg_image.hpp"
#include "utils/image_helper.hpp"


enum class InfoHintType
{
    UP_FOLLOWING,           // 已关注
    UP_NOT_FOLLOWED,        //关注
    BANGUMI_FOLLOWING,      //已追番
    BANGUMI_NOT_FOLLOWED,   //追番
    CINEMA_FOLLOWING,       //已追剧
    CINEMA_NOT_FOLLOWED,    //追剧
    NONE                    // 看自己的视频不显示
};

class UserInfoView : public brls::Box {

public:
    UserInfoView(){
        this->inflateFromXMLRes("xml/views/user_info.xml");
    }

    void setUserInfo(std::string avatar, std::string username, std::string misc){
        this->labelUsername->setText(username);
        this->labeMisc->setText(misc);

        if(avatar.empty()){
            this->avatarView->getParent()->setVisibility(brls::Visibility::GONE);
        } else {
            this->avatarView->getParent()->setVisibility(brls::Visibility::VISIBLE);
            ImageHelper::with(this)->load(avatar)->into(this->avatarView);
        }
    }

    static brls::View* create(){
        return new UserInfoView();
    }

    brls::Image* getAvatar(){
        return this->avatarView;
    }

    void setHintType(InfoHintType type){
        if(type == InfoHintType::NONE){
            this->boxHint->setVisibility(brls::Visibility::GONE);
            return;
        }

        this->boxHint->setVisibility(brls::Visibility::VISIBLE);
        auto theme = brls::Application::getTheme();

        switch(type){
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

private:
    BRLS_BIND(brls::Image, avatarView, "avatar");
    BRLS_BIND(brls::Label, labelUsername, "username");
    BRLS_BIND(brls::Label, labeMisc, "misc");
    BRLS_BIND(brls::Label, labeHint, "label/hint");
    BRLS_BIND(SVGImage, svgHint, "svg/hint");
    BRLS_BIND(brls::Box, boxHint, "box/hint");

};