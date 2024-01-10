//
// Created by fang on 2022/5/26.
//

#pragma once

#include "view/recycling_grid.hpp"

class NetImage;
class SVGImage;
class TextBox;

class BaseVideoCard : public RecyclingGridItem {
public:
    void prepareForReuse() override;

    void cacheForReuse() override;

protected:
    BRLS_BIND(brls::Image, picture, "video/card/picture");
};

class RecyclingGridItemVideoCard : public BaseVideoCard {
public:
    RecyclingGridItemVideoCard();

    ~RecyclingGridItemVideoCard() override;

    void setCard(std::string pic, std::string title, std::string username,
                 int pubdate = 0, int view_count = 0, int danmaku = 0,
                 int duration = 0, std::string extra = "");

    void setCard(std::string pic, std::string title, std::string username,
                 int pubdate = 0, int view_count = 0, int danmaku = 0,
                 std::string rightBottomBadge = "", std::string extra = "");

    /** 设置视频下方的推荐原因
     * 热门 每周必看 视频下方都包含推荐原因
     */
    void setRCMDReason(std::string reason);

    /** 设置视频下方的推荐原因（黄色字体）
     * 热门 入站必刷 视频下方都包含此种样式的推荐原因
     */
    void setAchievement(std::string explain);

    static RecyclingGridItemVideoCard* create();

private:
    BRLS_BIND(TextBox, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDanmaku, "video/card/label/danmaku");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
    BRLS_BIND(brls::Box, boxHint, "video/card/hint");
    BRLS_BIND(brls::Label, labelHint, "video/card/label/hint");
    BRLS_BIND(SVGImage, svgUp, "video/svg/up");
    BRLS_BIND(brls::Box, boxRCMD, "video/card/rcmd_box");
    BRLS_BIND(brls::Label, labelRCMD, "video/card/label/rcmd");
    BRLS_BIND(brls::Box, boxAchievement, "video/card/achievement_box");
    BRLS_BIND(brls::Label, labelAchievement, "video/card/label/achievement");
};

class RecyclingGridItemRankVideoCard : public BaseVideoCard {
public:
    explicit RecyclingGridItemRankVideoCard(
        std::string res = "xml/views/video_card_rank.xml");

    ~RecyclingGridItemRankVideoCard() override;

    void setCard(std::string pic, std::string title, std::string username,
                 int pubdate = 0, int view_count = 0, int danmaku = 0,
                 int duration = 0, int index = 0);

    static RecyclingGridItemRankVideoCard* create(
        std::string res = "xml/views/video_card_rank.xml");

private:
    BRLS_BIND(SVGImage, svgIndex, "video/card/svg/index");
    BRLS_BIND(TextBox, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDanmaku, "video/card/label/danmaku");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Label, labelIndex, "video/card/label/index");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
};

class RecyclingGridItemLiveVideoCard : public BaseVideoCard {
public:
    RecyclingGridItemLiveVideoCard();

    ~RecyclingGridItemLiveVideoCard() override;

    void setCard(std::string pic, std::string title, std::string username,
                 std::string area, int view_count = 0, bool following = false);

    static RecyclingGridItemLiveVideoCard* create();

private:
    BRLS_BIND(TextBox, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
    BRLS_BIND(brls::Box, boxHint, "video/card/hint");
    BRLS_BIND(SVGImage, svgUp, "video/svg/up");
};

class RecyclingGridItemPGCVideoCard : public BaseVideoCard {
public:
    explicit RecyclingGridItemPGCVideoCard(bool vertical_cover = true);

    ~RecyclingGridItemPGCVideoCard() override;

    bool isVertical();

    void setCard(std::string pic, std::string title, std::string username,
                 std::string badge_top, std::string badge_bottom_left,
                 std::string badge_bottom_right);

    void cacheForReuse() override;

    static RecyclingGridItemPGCVideoCard* create(bool vertical_cover = true);

private:
    BRLS_BIND(brls::Image, badgeTop, "video/card/badge/top");
    BRLS_BIND(brls::Image, badgeBottomLeft, "video/card/badge/bottom/left");
    BRLS_BIND(TextBox, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
    BRLS_BIND(brls::Box, boxBadgeBottom, "video/card/badge/box/bottom");

    bool vertical_cover = true;
};

class RecyclingGridItemSearchPGCVideoCard : public BaseVideoCard {
public:
    RecyclingGridItemSearchPGCVideoCard();

    ~RecyclingGridItemSearchPGCVideoCard() override;

    void setCard(std::string pic, std::string title, std::string subtitle,
                 std::string actor, std::string desc, std::string badge_top,
                 std::string badge_color, std::string scoreCount,
                 std::string score, std::string type, std::string bottom);

    static RecyclingGridItem* create();

private:
    BRLS_BIND(brls::Box, boxTop, "video/card/badge/boxTop");
    BRLS_BIND(brls::Label, badgeTop, "video/card/badge/top");
    BRLS_BIND(brls::Label, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelSubtitle, "video/card/label/subtitle");
    BRLS_BIND(brls::Label, labelActor, "video/card/label/actor");
    BRLS_BIND(brls::Label, labelDesc, "video/card/label/desc");
    BRLS_BIND(brls::Label, labelScoreCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelScore, "video/card/label/score");
    BRLS_BIND(brls::Label, labelType, "video/card/label/type");
    BRLS_BIND(brls::Label, labelBottom, "video/card/label/bottom");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
};

class RecyclingGridItemViewMoreCard : public RecyclingGridItem {
public:
    explicit RecyclingGridItemViewMoreCard(bool vertical_cover = true);

    ~RecyclingGridItemViewMoreCard() override;

    bool isVertical();

    void prepareForReuse() override;

    void cacheForReuse() override;

    static RecyclingGridItem* create(bool vertical_cover = true);

private:
    bool vertical_cover = true;
};

class RecyclingGridItemHistoryVideoCard : public BaseVideoCard {
public:
    RecyclingGridItemHistoryVideoCard();

    ~RecyclingGridItemHistoryVideoCard() override;

    void setCard(std::string pic, std::string title, std::string username,
                 std::string leftBottomBadge  = "",
                 std::string rightBottomBadge = "",
                 std::string rightTopBadge = "", int deviceType = 0,
                 float progress = -1, bool showName = true);

    static RecyclingGridItemHistoryVideoCard* create();

private:
    BRLS_BIND(TextBox, labelTitle, "video/card/label/title");
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

class RecyclingGridItemCollectionVideoCard : public BaseVideoCard {
public:
    RecyclingGridItemCollectionVideoCard();

    ~RecyclingGridItemCollectionVideoCard() override;

    void setCard(std::string pic, std::string title, std::string username,
                 std::string leftBottomBadge  = "",
                 std::string rightBottomBadge = "");

    static RecyclingGridItemCollectionVideoCard* create();

private:
    BRLS_BIND(TextBox, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
};

class RecyclingGridItemRelatedVideoCard : public BaseVideoCard {
public:
    RecyclingGridItemRelatedVideoCard();

    ~RecyclingGridItemRelatedVideoCard() override;

    void setCard(std::string pic, std::string title, std::string username,
                 std::string playCount, std::string danmakuCount,
                 std::string rightBottomBadge = "");

    static RecyclingGridItemRelatedVideoCard* create();

private:
    BRLS_BIND(TextBox, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelDanmaku, "video/card/label/danmaku");
    BRLS_BIND(brls::Label, labelDuration, "video/card/label/duration");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
};

class RecyclingGridItemSeasonSeriesVideoCard : public BaseVideoCard {
public:
    RecyclingGridItemSeasonSeriesVideoCard();

    ~RecyclingGridItemSeasonSeriesVideoCard() override;

    void setCard(const std::string& pic, const std::string& title,
                 const std::string& username, const std::string& playCount,
                 const std::string& likeCount, const std::string& badge,
                 const std::string& badge_color);

    static RecyclingGridItem* create();

private:
    BRLS_BIND(TextBox, labelTitle, "video/card/label/title");
    BRLS_BIND(brls::Label, labelUsername, "video/card/label/username");
    BRLS_BIND(brls::Label, labelCount, "video/card/label/count");
    BRLS_BIND(brls::Label, labelLike, "video/card/label/like");
    BRLS_BIND(brls::Box, boxTop, "video/card/badge/boxTop");
    BRLS_BIND(brls::Label, badgeTop, "video/card/badge/top");
    BRLS_BIND(brls::Box, boxPic, "video/card/pic_box");
};