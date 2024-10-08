//
// Created by fang on 2022/7/18.
//

// register this view in main.cpp
//#include "view/video_comment.hpp"
//    brls::Application::registerXMLView("VideoComment", VideoComment::create);
// <brls:View xml=@res/xml/views/video_comment.xml

#pragma once

#include "view/recycling_grid.hpp"
#include "view/user_info.hpp"
#include "bilibili/result/video_detail_result.h"

class SVGImage;
class TextBox;

/// GridHintView

class GridHintView : public RecyclingGridItem {
public:
    GridHintView();

    static RecyclingGridItem* create();

    brls::Label* hintLabel;
};

class VideoCommentReply : public RecyclingGridItem {
public:
    VideoCommentReply();

    static RecyclingGridItem* create();

    brls::Label* hintLabel;
};

class VideoCommentSort : public RecyclingGridItem {
public:
    VideoCommentSort();

    static RecyclingGridItem* create();

    brls::Label* hintLabel;
    brls::Label* sortLabel;
    SVGImage* svgImage;
};

class VideoComment : public RecyclingGridItem {
public:
    VideoComment();

    ~VideoComment() override;

    static RecyclingGridItem* create();

    void setMaxRows(size_t value);

    void setData(bilibili::VideoCommentResult data);

    void setReplyNum(size_t num);

    void setLikeNum(size_t num);

    void setLiked(size_t action);

    bilibili::VideoCommentResult getData();

    void setMainTextColor(NVGcolor color);

    void hideReplyIcon(bool hide);

    void prepareForReuse() override;

    void cacheForReuse() override;

protected:
    BRLS_BIND(TextBox, commentContent, "comment/label/content");
    BRLS_BIND(brls::Label, labelLike, "comment/label/like");
    BRLS_BIND(brls::Label, labelReply, "comment/label/reply");
    BRLS_BIND(UserInfoView, userInfo, "comment/userinfo");
    BRLS_BIND(SVGImage, userLevel, "comment/user/level");
    BRLS_BIND(SVGImage, svgReply, "comment/svg/reply");
    BRLS_BIND(SVGImage, svgLike, "comment/svg/like");
    BRLS_BIND(SVGImage, svgDislike, "comment/svg/dislike");
    bilibili::VideoCommentResult comment_data;
};