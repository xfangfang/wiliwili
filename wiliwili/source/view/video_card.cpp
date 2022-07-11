//
// Created by fang on 2022/7/10.
//

#include "view/video_card.hpp"
#include "view/net_image.hpp"
#include "view/svg_image.hpp"
#include "utils/number_helper.hpp"


RecyclingGridItemVideoCard::RecyclingGridItemVideoCard(){
    this->inflateFromXMLRes("xml/views/video_card.xml");
}

RecyclingGridItemVideoCard::~RecyclingGridItemVideoCard() {

}

void RecyclingGridItemVideoCard::setRCMDReason(std::string reason){
    this->boxRCMD->setVisibility(brls::Visibility::VISIBLE);
    this->labelRCMD->setText(reason);
    this->boxPic->setHeightPercentage(57);
}

void RecyclingGridItemVideoCard::setAchievement(std::string explain){
    this->boxAchievement->setVisibility(brls::Visibility::VISIBLE);
    this->labelAchievement->setText(explain);
    this->boxPic->setHeightPercentage(57);
}

void RecyclingGridItemVideoCard::prepareForReuse(){
    //准备显示该项
}

void RecyclingGridItemVideoCard::cacheForReuse(){
    //准备回收该项
    this->picture->clear();
}

RecyclingGridItemVideoCard* RecyclingGridItemVideoCard::create(){
    return new RecyclingGridItemVideoCard();
}

void RecyclingGridItemVideoCard::setCard(std::string pic, std::string title, std::string username, int pubdate,
                                         int view_count, int danmaku, int duration){
    if(pubdate)
        this->labelUsername->setText(username + "·" +wiliwili::sec2date(pubdate));
    else
        this->labelUsername->setText(username);

    this->labelTitle->setIsWrapping(true);
    this->labelTitle->setText(title);
    this->picture->setImageFromNet(pic);
    this->labelCount->setText(wiliwili::num2w(view_count));
    this->labelDanmaku->setText(wiliwili::num2w(danmaku));

    if(duration)
        this->labelDuration->setText(wiliwili::sec2Time(duration));
    else
        this->labelDuration->setText("");
}


/// RecyclingGridItemRankVideoCard
/// 左上角有角标图案，从1开始自动添加序号


RecyclingGridItemRankVideoCard::RecyclingGridItemRankVideoCard(std::string res){
    this->inflateFromXMLRes(res);
}

RecyclingGridItemRankVideoCard::~RecyclingGridItemRankVideoCard(){
}

void RecyclingGridItemRankVideoCard::setCard(std::string pic, std::string title, std::string username, int pubdate,
             int view_count, int danmaku, int duration, int index){
    if(pubdate)
        this->labelUsername->setText(username + "·" +wiliwili::sec2date(pubdate));
    else
        this->labelUsername->setText(username);

    this->labelTitle->setIsWrapping(true);
    this->labelTitle->setText(title);
    this->picture->setImageFromNet(pic);
    this->labelCount->setText(wiliwili::num2w(view_count));
    this->labelDanmaku->setText(wiliwili::num2w(danmaku));

    if(duration)
        this->labelDuration->setText(wiliwili::sec2Time(duration));
    else
        this->labelDuration->setText("");

    if(index < 10){
        this->labelIndex->setMarginLeft(4);
    } else{
        this->labelIndex->setMarginLeft(0);
    }

    this->labelIndex->setText(std::to_string(index));

    switch (index) {
        case 1:
            this->svgIndex->setImageFromSVGRes("svg/rate-crown-gold.svg");
            break;
        case 2:
            this->svgIndex->setImageFromSVGRes("svg/rate-crown-sliver.svg");
            break;
        case 3:
            this->svgIndex->setImageFromSVGRes("svg/rate-crown-copper.svg");
            break;
        default:
            this->svgIndex->setImageFromSVGRes("svg/rate-crown-iron.svg");
    }
}


void RecyclingGridItemRankVideoCard::prepareForReuse(){
    //准备显示该项
}

void RecyclingGridItemRankVideoCard::cacheForReuse(){
    //准备回收该项
    this->picture->clear();
}

RecyclingGridItemRankVideoCard* RecyclingGridItemRankVideoCard::create(std::string res){
    return new RecyclingGridItemRankVideoCard(res);
}
