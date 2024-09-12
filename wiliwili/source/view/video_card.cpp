//
// Created by fang on 2022/7/10.
//

#include "view/video_card.hpp"
#include "view/svg_image.hpp"
#include "view/text_box.hpp"
#include "utils/number_helper.hpp"
#include "utils/image_helper.hpp"
#include <pystring.h>

using namespace brls::literals;

/// 视频卡片基类

void BaseVideoCard::prepareForReuse() {
    //准备显示该项
    this->picture->setImageFromRes("pictures/video-card-bg.png");
}

void BaseVideoCard::cacheForReuse() {
    //准备回收该项
    ImageHelper::clear(this->picture);
}

/// 普通视频封面

RecyclingGridItemVideoCard::RecyclingGridItemVideoCard() { this->inflateFromXMLRes("xml/views/video_card.xml"); }

RecyclingGridItemVideoCard::~RecyclingGridItemVideoCard() {
    // 优先清空正在进行的图片请求
    ImageHelper::clear(this->picture);
}

void RecyclingGridItemVideoCard::setRCMDReason(const std::string& reason) {
    this->boxRCMD->setVisibility(brls::Visibility::VISIBLE);
    this->labelRCMD->setText(reason);
    this->boxPic->setHeightPercentage(54.8);
}

void RecyclingGridItemVideoCard::setAchievement(const std::string& explain) {
    this->boxAchievement->setVisibility(brls::Visibility::VISIBLE);
    this->labelAchievement->setText(explain);
    this->boxPic->setHeightPercentage(54.8);
}

RecyclingGridItemVideoCard* RecyclingGridItemVideoCard::create() { return new RecyclingGridItemVideoCard(); }

void RecyclingGridItemVideoCard::cacheForReuse() {
    //准备回收该项
    ImageHelper::clear(this->picture);
    ImageHelper::clear(this->pictureHint);
}

void RecyclingGridItemVideoCard::setExtraInfo(const std::string& extra, float width, float height){
    if (extra.empty()) {
        this->svgUp->setVisibility(brls::Visibility::VISIBLE);
        this->boxHint->setVisibility(brls::Visibility::GONE);
        this->pictureHint->setVisibility(brls::Visibility::GONE);
    } else if (pystring::startswith(extra, "http")) {
        this->svgUp->setVisibility(brls::Visibility::GONE);
        this->boxHint->setVisibility(brls::Visibility::GONE);
        this->pictureHint->setVisibility(brls::Visibility::VISIBLE);
        this->pictureHint->setDimensions(width, height);
        ImageHelper::with(this->pictureHint)->load(extra);
    } else {
        this->svgUp->setVisibility(brls::Visibility::GONE);
        this->boxHint->setVisibility(brls::Visibility::VISIBLE);
        this->labelHint->setText(extra);
        this->pictureHint->setVisibility(brls::Visibility::GONE);
    }
}

void RecyclingGridItemVideoCard::setBasicInfo(const std::string& title, const std::string& pic, const std::string& username) {
    this->labelTitle->setIsWrapping(true);
    this->labelTitle->setText(title);

    this->labelUsername->setText(username);

    ImageHelper::with(this->picture)->load(pic);
}

void RecyclingGridItemVideoCard::setCard(const std::string& pic, const std::string& title, const std::string& username,
                                         int pubdate, int view_count, int danmaku, int duration,
                                         const std::string& extra) {
    std::string rightBottomBadge;
    if (duration)
        rightBottomBadge = wiliwili::sec2Time(duration);

    this->setCard(pic, title, username, pubdate, view_count, danmaku, rightBottomBadge, extra);
}

void RecyclingGridItemVideoCard::setCard(const std::string& pic, const std::string& title, const std::string& username,
                                         int pubdate, int view_count, int danmaku, const std::string& rightBottomBadge,
                                         const std::string& extra) {
    std::string author;
    if (pubdate)
        author = username + "·" + wiliwili::sec2date(pubdate);
    else
        author = username;

    this->setCard(pic, title, author, wiliwili::num2w(view_count), wiliwili::num2w(danmaku), rightBottomBadge, extra);
}

void RecyclingGridItemVideoCard::setCard(const std::string& pic, const std::string& title, const std::string& username,
                                         const std::string& viewCount, const std::string& danmakuCount,
                                         const std::string& rightBottomBadge, const std::string& extra) {
    this->setBasicInfo(title, pic, username);
    this->setExtraInfo(extra);

    this->labelDuration->setText(rightBottomBadge);

    if (viewCount.empty()) {
        this->svgView->setVisibility(brls::Visibility::GONE);
        this->labelCount->setVisibility(brls::Visibility::GONE);
    } else {
        this->svgView->setVisibility(brls::Visibility::VISIBLE);
        this->labelCount->setVisibility(brls::Visibility::VISIBLE);
        this->labelCount->setText(viewCount);
    }

    if (danmakuCount.empty()) {
        this->svgDanmaku->setVisibility(brls::Visibility::GONE);
        this->labelDanmaku->setVisibility(brls::Visibility::GONE);
    } else {
        this->svgDanmaku->setVisibility(brls::Visibility::VISIBLE);
        this->labelDanmaku->setVisibility(brls::Visibility::VISIBLE);
        this->labelDanmaku->setText(danmakuCount);
    }
}

/// 排行榜视频封面
/// 左上角有角标图案，从1开始自动添加序号

RecyclingGridItemRankVideoCard::RecyclingGridItemRankVideoCard(std::string res) { this->inflateFromXMLRes(res); }

RecyclingGridItemRankVideoCard::~RecyclingGridItemRankVideoCard() {
    // 优先清空正在进行的图片请求
    ImageHelper::clear(this->picture);
}

void RecyclingGridItemRankVideoCard::setCard(std::string pic, std::string title, std::string username, int pubdate,
                                             int view_count, int danmaku, int duration, int index) {
    if (pubdate)
        this->labelUsername->setText(username + "·" + wiliwili::sec2date(pubdate));
    else
        this->labelUsername->setText(username);

    this->labelTitle->setIsWrapping(true);
    this->labelTitle->setText(title);
    ImageHelper::with(this->picture)->load(pic);
    this->labelCount->setText(wiliwili::num2w(view_count));
    this->labelDanmaku->setText(wiliwili::num2w(danmaku));

    if (duration)
        this->labelDuration->setText(wiliwili::sec2Time(duration));
    else
        this->labelDuration->setText("");

    if (index < 10) {
        this->labelIndex->setMarginLeft(4);
    } else {
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

RecyclingGridItemRankVideoCard* RecyclingGridItemRankVideoCard::create(std::string res) {
    return new RecyclingGridItemRankVideoCard(res);
}

/// 直播视频封面

RecyclingGridItemLiveVideoCard::RecyclingGridItemLiveVideoCard() {
    this->inflateFromXMLRes("xml/views/video_card_live.xml");
}

RecyclingGridItemLiveVideoCard::~RecyclingGridItemLiveVideoCard() {
    // 优先清空正在进行的图片请求
    ImageHelper::clear(this->picture);
}

void RecyclingGridItemLiveVideoCard::setCard(std::string pic, std::string title, std::string username, std::string area,
                                             int view_count, bool following) {
    this->labelUsername->setText(username);
    this->labelTitle->setIsWrapping(false);
    this->labelTitle->setText(title);
    ImageHelper::with(this->picture)->load(pic);
    this->labelCount->setText(wiliwili::num2w(view_count));
    this->labelDuration->setText(area);
    if (following) {
        this->svgUp->setVisibility(brls::Visibility::GONE);
        this->boxHint->setVisibility(brls::Visibility::VISIBLE);
    } else {
        this->svgUp->setVisibility(brls::Visibility::VISIBLE);
        this->boxHint->setVisibility(brls::Visibility::GONE);
    }
}

RecyclingGridItemLiveVideoCard* RecyclingGridItemLiveVideoCard::create() {
    return new RecyclingGridItemLiveVideoCard();
}

/// pgc video card
/// 支持预览图横竖切换的视频封面

RecyclingGridItemPGCVideoCard::RecyclingGridItemPGCVideoCard(bool vertical_cover) : vertical_cover(vertical_cover) {
    this->inflateFromXMLRes("xml/views/video_card_pgc.xml");
    if (!vertical_cover) {
        this->boxPic->setHeightPercentage(70);
        this->boxBadgeBottom->setHeightPercentage(20);
    }
}

RecyclingGridItemPGCVideoCard::~RecyclingGridItemPGCVideoCard() {
    // 优先清空正在进行的图片请求
    ImageHelper::clear(this->picture);
}

bool RecyclingGridItemPGCVideoCard::isVertical() { return this->vertical_cover; }

void RecyclingGridItemPGCVideoCard::setCard(std::string pic, std::string title, std::string username,
                                            std::string badge_top, std::string badge_bottom_left,
                                            std::string badge_bottom_right) {
    this->labelUsername->setText(username);
    this->labelTitle->setIsWrapping(false);
    this->labelTitle->setText(title);
    ImageHelper::with(this->picture)->load(pic);
    this->labelDuration->setText(badge_bottom_right);

    if (!badge_top.empty()) {
        ImageHelper::with(this->badgeTop)->load(badge_top);
    }

    if (!badge_bottom_left.empty()) {
        ImageHelper::with(this->badgeBottomLeft)->load(badge_bottom_left);
    }
}

void RecyclingGridItemPGCVideoCard::cacheForReuse() {
    //准备回收该项
    ImageHelper::clear(this->picture);
    ImageHelper::clear(this->badgeTop);
    ImageHelper::clear(this->badgeBottomLeft);
}

RecyclingGridItemPGCVideoCard* RecyclingGridItemPGCVideoCard::create(bool vertical_cover) {
    return new RecyclingGridItemPGCVideoCard(vertical_cover);
}

/// 搜索 番剧 和 影视 卡片
RecyclingGridItemSearchPGCVideoCard::RecyclingGridItemSearchPGCVideoCard() {
    this->inflateFromXMLRes("xml/views/video_card_search_pgc.xml");
}

RecyclingGridItemSearchPGCVideoCard::~RecyclingGridItemSearchPGCVideoCard() {
    // 优先清空正在进行的图片请求
    ImageHelper::clear(this->picture);
}

void RecyclingGridItemSearchPGCVideoCard::setCard(std::string pic, std::string title, std::string subtitle,
                                                  std::string actor, std::string desc, std::string badge_top,
                                                  std::string badge_color, std::string scoreCount, std::string score,
                                                  std::string type, std::string bottom) {
    this->labelType->setText(type);
    this->labelTitle->setText(title);
    this->labelSubtitle->setText(subtitle);
    this->labelScore->setText(score);
    this->labelScoreCount->setText(scoreCount);
    this->labelDesc->setText(desc);
    this->labelBottom->setText(bottom);
    this->badgeTop->setText(badge_top);

    if (actor.empty()) {
        this->labelActor->setVisibility(brls::Visibility::GONE);
    } else {
        this->labelActor->setVisibility(brls::Visibility::VISIBLE);
        this->labelActor->setText(actor);
    }

    unsigned char r, g, b;
    int result = sscanf(badge_color.c_str(), "#%02hhx%02hhx%02hhx", &r, &g, &b);
    if (result == 3) {
        this->boxTop->setVisibility(brls::Visibility::VISIBLE);
        this->boxTop->setBackgroundColor(nvgRGB(r, g, b));
    } else {
        this->boxTop->setVisibility(brls::Visibility::GONE);
    }

    ImageHelper::with(this->picture)->load(pic);
}

RecyclingGridItem* RecyclingGridItemSearchPGCVideoCard::create() { return new RecyclingGridItemSearchPGCVideoCard(); }

/// PGC 查看更多卡片

RecyclingGridItemViewMoreCard::RecyclingGridItemViewMoreCard(bool vertical_cover) : vertical_cover(vertical_cover) {
    this->inflateFromXMLRes("xml/views/video_card_pgc_more.xml");
}

RecyclingGridItemViewMoreCard::~RecyclingGridItemViewMoreCard() {}

void RecyclingGridItemViewMoreCard::prepareForReuse() {}

void RecyclingGridItemViewMoreCard::cacheForReuse() {}

bool RecyclingGridItemViewMoreCard::isVertical() { return this->vertical_cover; }

RecyclingGridItem* RecyclingGridItemViewMoreCard::create(bool vertical_cover) {
    return new RecyclingGridItemViewMoreCard(vertical_cover);
}

/// 历史记录 视频卡片

RecyclingGridItemHistoryVideoCard::RecyclingGridItemHistoryVideoCard() {
    this->inflateFromXMLRes("xml/views/video_card_history.xml");
}

RecyclingGridItemHistoryVideoCard::~RecyclingGridItemHistoryVideoCard() {
    // 优先清空正在进行的图片请求
    ImageHelper::clear(this->picture);
}

RecyclingGridItemHistoryVideoCard* RecyclingGridItemHistoryVideoCard::create() {
    return new RecyclingGridItemHistoryVideoCard();
}

void RecyclingGridItemHistoryVideoCard::setCard(std::string pic, std::string title, std::string username,
                                                std::string leftBottomBadge, std::string rightBottomBadge,
                                                std::string rightTopBadge, int deviceType, float progress,
                                                bool showName) {
    this->labelUsername->setText(username);
    this->labelTitle->setIsWrapping(true);
    this->labelTitle->setText(title);
    ImageHelper::with(this->picture)->load(pic);
    this->labelCount->setText(leftBottomBadge);
    this->labelDuration->setText(rightBottomBadge);
    this->labelRightTop->setText(rightTopBadge);

    if (showName) {
        svgUp->setVisibility(brls::Visibility::VISIBLE);
    } else {
        svgUp->setVisibility(brls::Visibility::GONE);
    }

    if (rightTopBadge.empty()) {
        boxBadge->setVisibility(brls::Visibility::INVISIBLE);
    } else {
        boxBadge->setVisibility(brls::Visibility::VISIBLE);
        auto theme = brls::Application::getTheme();
        if (rightTopBadge == "wiliwili/mine/done"_i18n) {
            boxBadge->setBackgroundColor(theme.getColor("color/grey_4"));
        } else {
            boxBadge->setBackgroundColor(theme.getColor("color/bilibili"));
        }
    }

    if (progress < 0) {
        rectProgress->getParent()->setVisibility(brls::Visibility::INVISIBLE);
    } else {
        rectProgress->getParent()->setVisibility(brls::Visibility::VISIBLE);
        if (progress > 1.0) progress = 1.0;
        rectProgress->setWidthPercentage(progress * 100);
    }

    switch (deviceType) {
        case 1:  // ios
        case 3:  // android
        case 5:
            svgDT->setImageFromSVGRes("svg/history-phone.svg");
            break;
        case 9:  // voice
            svgDT->setImageFromSVGRes("svg/history-voice.svg");
            break;
        case 2:  // pc
            svgDT->setImageFromSVGRes("svg/history-pc.svg");
            break;
        case 4:  // pad
        case 6:
            svgDT->setImageFromSVGRes("svg/history-ipad.svg");
            break;
        case 8:  // car
            svgDT->setImageFromSVGRes("svg/history-carplay.svg");
            break;
        case 33:  // tv
            svgDT->setImageFromSVGRes("svg/history-tv.svg");
            break;
        default:
            svgDT->setImageFromSVGRes("svg/widget-video-play-count.svg");
    }
}

/// 收藏夹 卡片

RecyclingGridItemCollectionVideoCard::RecyclingGridItemCollectionVideoCard() {
    this->inflateFromXMLRes("xml/views/video_card_collection.xml");
}

RecyclingGridItemCollectionVideoCard::~RecyclingGridItemCollectionVideoCard() {
    // 优先清空正在进行的图片请求
    ImageHelper::clear(this->picture);
}

RecyclingGridItemCollectionVideoCard* RecyclingGridItemCollectionVideoCard::create() {
    return new RecyclingGridItemCollectionVideoCard();
}

void RecyclingGridItemCollectionVideoCard::setCard(std::string pic, std::string title, std::string username,
                                                   std::string leftBottomBadge, std::string rightBottomBadge) {
    this->labelUsername->setText(username);
    this->labelTitle->setIsWrapping(true);
    this->labelTitle->setText(title);
    if (!pic.empty()) {
        ImageHelper::with(this->picture)->load(pic);
    }
    this->labelCount->setText(leftBottomBadge);
    this->labelDuration->setText(rightBottomBadge);
}

/// 播放页推荐 卡片

RecyclingGridItemRelatedVideoCard::RecyclingGridItemRelatedVideoCard() {
    this->inflateFromXMLRes("xml/views/video_card_related.xml");
}

RecyclingGridItemRelatedVideoCard::~RecyclingGridItemRelatedVideoCard() {
    // 优先清空正在进行的图片请求
    ImageHelper::clear(this->picture);
}

RecyclingGridItemRelatedVideoCard* RecyclingGridItemRelatedVideoCard::create() {
    return new RecyclingGridItemRelatedVideoCard();
}

void RecyclingGridItemRelatedVideoCard::setCard(std::string pic, std::string title, std::string username,
                                                std::string playCount, std::string danmakuCount,
                                                std::string rightBottomBadge) {
    this->labelUsername->setText(username);
    this->labelTitle->setIsWrapping(true);
    this->labelTitle->setText(title);
    ImageHelper::with(this->picture)->load(pic);
    this->labelCount->setText(playCount);
    this->labelDanmaku->setText(danmakuCount);
    this->labelDuration->setText(rightBottomBadge);
}

void RecyclingGridItemRelatedVideoCard::setCharging(bool value) {
    boxCharging->setVisibility(value ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
}

/// 相关番剧卡片

RecyclingGridItemSeasonSeriesVideoCard::RecyclingGridItemSeasonSeriesVideoCard() {
    this->inflateFromXMLRes("xml/views/video_card_series.xml");
}

RecyclingGridItemSeasonSeriesVideoCard::~RecyclingGridItemSeasonSeriesVideoCard() {
    // 优先清空正在进行的图片请求
    ImageHelper::clear(this->picture);
}

RecyclingGridItem* RecyclingGridItemSeasonSeriesVideoCard::create() {
    return new RecyclingGridItemSeasonSeriesVideoCard();
}

void RecyclingGridItemSeasonSeriesVideoCard::setCard(const std::string& pic, const std::string& title,
                                                     const std::string& username, const std::string& playCount,
                                                     const std::string& likeCount, const std::string& badge,
                                                     const std::string& badge_color) {
    this->labelUsername->setText(username);
    this->labelTitle->setIsWrapping(true);
    this->labelTitle->setText(title);
    ImageHelper::with(this->picture)->load(pic);
    this->labelCount->setText(playCount);
    this->labelLike->setText(likeCount);
    this->badgeTop->setText(badge);

    unsigned char r, g, b;
    int result = sscanf(badge_color.c_str(), "#%02hhx%02hhx%02hhx", &r, &g, &b);
    if (result == 3 && !badge.empty()) {
        this->boxTop->setVisibility(brls::Visibility::VISIBLE);
        this->boxTop->setBackgroundColor(nvgRGB(r, g, b));
    } else {
        this->boxTop->setVisibility(brls::Visibility::GONE);
    }
}
