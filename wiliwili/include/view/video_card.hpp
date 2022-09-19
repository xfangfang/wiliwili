//
// Created by fang on 2022/5/26.
//

#pragma once

#include <borealis.hpp>
#include "view/recycling_grid.hpp"

class NetImage;
class SVGImage;

class RecyclingGridItemVideoCard : public RecyclingGridItem {
public:
    RecyclingGridItemVideoCard();

    ~RecyclingGridItemVideoCard();

    void setCard(std::string pic, std::string title, std::string username,
                 int pubdate = 0, int view_count = 0, int danmaku = 0,
                 int duration = 0);

    void setCard(std::string pic, std::string title, std::string username,
                 int pubdate = 0, int view_count = 0, int danmaku = 0,
                 std::string rightBottomBadge = "");

    /** 设置视频下方的推荐原因
     * 热门 每周必看 视频下方都包含推荐原因
     */
    void setRCMDReason(std::string reason);

    /** 设置视频下方的推荐原因（黄色字体）
     * 热门 入站必刷 视频下方都包含此种样式的推荐原因
     */
    void setAchievement(std::string explain);

    void prepareForReuse();

    void cacheForReuse();

    static RecyclingGridItemVideoCard* create();

private:
    BRLS_BIND(brls::Image, picture, "video/card/picture");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDanmaku, "video/card/label/danmaku");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
    BRLS_BIND(brls::Box, boxRCMD, "video/card/rcmd_box");
    BRLS_BIND(brls::Label, labelRCMD, "video/card/label/rcmd");
    BRLS_BIND(brls::Box, boxAchievement, "video/card/achievement_box");
    BRLS_BIND(brls::Label, labelAchievement, "video/card/label/achievement");
};

class RecyclingGridItemRankVideoCard : public RecyclingGridItem {
public:
    RecyclingGridItemRankVideoCard(
        std::string res = "xml/views/video_card_rank.xml");

    ~RecyclingGridItemRankVideoCard();

    void setCard(std::string pic, std::string title, std::string username,
                 int pubdate = 0, int view_count = 0, int danmaku = 0,
                 int duration = 0, int index = 0);

    void prepareForReuse();

    void cacheForReuse();

    static RecyclingGridItemRankVideoCard* create(
        std::string res = "xml/views/video_card_rank.xml");

private:
    BRLS_BIND(brls::Image, picture, "video/card/picture");
    BRLS_BIND(SVGImage, svgIndex, "video/card/svg/index");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDanmaku, "video/card/label/danmaku");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Label, labelIndex, "video/card/label/index");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
};

class RecyclingGridItemLiveVideoCard : public RecyclingGridItem {
public:
    RecyclingGridItemLiveVideoCard();

    ~RecyclingGridItemLiveVideoCard();

    void setCard(std::string pic, std::string title, std::string username,
                 std::string area, int view_count = 0);

    void prepareForReuse();

    void cacheForReuse();

    static RecyclingGridItemLiveVideoCard* create();

private:
    BRLS_BIND(brls::Image, picture, "video/card/picture");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
};

class RecyclingGridItemPGCVideoCard : public RecyclingGridItem {
public:
    RecyclingGridItemPGCVideoCard(bool vertical_cover = true);

    ~RecyclingGridItemPGCVideoCard();

    bool isVertical();

    void setCard(std::string pic, std::string title, std::string username,
                 std::string badge_top, std::string badge_bottom_left,
                 std::string badge_bottom_right);

    void prepareForReuse();

    void cacheForReuse();

    static RecyclingGridItemPGCVideoCard* create(bool vertical_cover = true);

private:
    BRLS_BIND(brls::Image, picture, "video/card/picture");
    BRLS_BIND(brls::Image, badgeTop, "video/card/badge/top");
    BRLS_BIND(brls::Image, badgeBottomLeft, "video/card/badge/bottom/left");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
    BRLS_BIND(brls::Box, boxBadgeBottom, "video/card/badge/box/bottom");

    bool vertical_cover = true;
};

class RecyclingGridItemViewMoreCard : public RecyclingGridItem {
public:
    RecyclingGridItemViewMoreCard(bool vertical_cover = true);

    ~RecyclingGridItemViewMoreCard();

    bool isVertical();

    void prepareForReuse();

    void cacheForReuse();

    static RecyclingGridItem* create(bool vertical_cover = true);

private:
    BRLS_BIND(brls::Image, picture, "video/card/picture");
    BRLS_BIND(brls::Image, badgeTop, "video/card/badge/top");
    BRLS_BIND(brls::Image, badgeBottomLeft, "video/card/badge/bottom/left");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
    BRLS_BIND(brls::Box, boxBadgeBottom, "video/card/badge/box/bottom");

    bool vertical_cover = true;
};

class RecyclingGridItemHistoryVideoCard : public RecyclingGridItem {
public:
    RecyclingGridItemHistoryVideoCard();

    ~RecyclingGridItemHistoryVideoCard();

    void setCard(std::string pic, std::string title, std::string username,
                 std::string leftBottomBadge  = "",
                 std::string rightBottomBadge = "",
                 std::string rightTopBadge = "", int deviceType = 0,
                 float progress = -1, bool showName = true);

    void prepareForReuse();

    void cacheForReuse();

    static RecyclingGridItemHistoryVideoCard* create();

private:
    BRLS_BIND(brls::Image, picture, "video/card/picture");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Label, labelRightTop, "video/card/label/badge/right/top");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
    BRLS_BIND(SVGImage, svgDT, "video/card/deviceType");
    BRLS_BIND(SVGImage, svgUp, "video/card/up");
    BRLS_BIND(brls::Box, boxBadge, "video/card/badgeBox");
    BRLS_BIND(brls::Rectangle, rectProgress, "video/card/progress");
};

class RecyclingGridItemCollectionVideoCard : public RecyclingGridItem {
public:
    RecyclingGridItemCollectionVideoCard();

    ~RecyclingGridItemCollectionVideoCard();

    void setCard(std::string pic, std::string title, std::string username,
                 std::string leftBottomBadge  = "",
                 std::string rightBottomBadge = "");

    void prepareForReuse();

    void cacheForReuse();

    static RecyclingGridItemCollectionVideoCard* create();

private:
    BRLS_BIND(brls::Image, picture, "video/card/picture");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
};

/// 播放页推荐卡片
class RecyclingGridItemRelatedVideoCard : public RecyclingGridItem {
public:
    RecyclingGridItemRelatedVideoCard();

    ~RecyclingGridItemRelatedVideoCard();

    void setCard(std::string pic, std::string title, std::string username,
                 std::string playCount, std::string danmakuCount,
                 std::string rightBottomBadge = "");

    void prepareForReuse();

    void cacheForReuse();

    static RecyclingGridItemRelatedVideoCard* create();

private:
    BRLS_BIND(brls::Image, picture, "video/card/picture");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDanmaku, "video/card/label/danmaku");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
};
