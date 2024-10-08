//
// Created by fang on 2023/1/6.
//

// register this fragment in main.cpp
//#include "fragment/player_single_comment.hpp"
//    brls::Application::registerXMLView("PlayerSingleComment", PlayerSingleComment::create);
// <brls:View xml=@res/xml/fragment/player_single_comment.xml

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

#include "bilibili/result/video_detail_result.h"

class VideoComment;
class RecyclingGrid;
class SVGImage;
class ButtonClose;

enum CommentUiType {
    COMMENT_UI_TYPE_VIDEO,
    COMMENT_UI_TYPE_DYNAMIC,

};

/// 评论详情页面
class PlayerSingleComment : public brls::Box {
public:
    PlayerSingleComment(CommentUiType type = COMMENT_UI_TYPE_VIDEO);

    void setCommentData(const bilibili::VideoCommentResult& result, float y, int type);

    void requestData();

    View* getDefaultFocus() override;

    void dismiss(std::function<void(void)> cb = nullptr) override;

    ~PlayerSingleComment() override;

    void showStartAnimation(float y);

    void showDismissAnimation();

    brls::Event<size_t> likeStateEvent;
    brls::Event<size_t> likeNumEvent;
    brls::Event<size_t> replyNumEvent;
    brls::Event<> deleteEvent;

private:
    bilibili::VideoCommentResult root;
    bilibili::VideoCommentCursor cursor;
    BRLS_BIND(RecyclingGrid, recyclingGrid, "player/single/comment/recyclingGrid");
    BRLS_BIND(ButtonClose, closeBtn, "button/close");
    BRLS_BIND(brls::Box, backgroundBox, "box/background");
    BRLS_BIND(brls::Box, cancel, "player/cancel");

    brls::Animatable position     = 0.0f;
    float commentOriginalPosition = 0.0f;
    int commentType               = 1;
    CommentUiType uiType          = COMMENT_UI_TYPE_VIDEO;
};

/// 选择对单条评论的行为：点赞、回复、删除
class PlayerCommentAction : public brls::Box {
public:
    PlayerCommentAction(CommentUiType type);

    void setActionData(const bilibili::VideoCommentResult& data, float y);

    void showDelete();

    View* getDefaultFocus() override;

    void dismiss(std::function<void(void)> cb = nullptr) override;

    brls::Event<> replyClickEvent, deleteClickEvent;
    brls::Event<size_t> likeClickEvent;

    void showStartAnimation();

    void showDismissAnimation();

private:
    BRLS_BIND(SVGImage, svgReply, "comment/action/svg/reply");
    BRLS_BIND(SVGImage, svgLike, "comment/action/svg/like");
    BRLS_BIND(SVGImage, svgDisLike, "comment/action/svg/dislike");
    BRLS_BIND(SVGImage, svgDelete, "comment/action/svg/delete");
    BRLS_BIND(SVGImage, svgGallery, "comment/action/svg/gallery");
    BRLS_BIND(brls::Box, actionBox, "comment/action/box");
    BRLS_BIND(VideoComment, comment, "comment/action/comment");
    BRLS_BIND(ButtonClose, closeBtn, "button/close");
    BRLS_BIND(brls::Box, backgroundBox, "box/background");
    BRLS_BIND(brls::Box, cancel, "player/cancel");

    brls::Animatable position     = 0.0f;
    float commentOriginalPosition = 1.0f;
};