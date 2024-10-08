//
// Created by fang on 2024/3/21.
//

#pragma once

#include <borealis/core/bind.hpp>

#include "view/recycling_grid.hpp"
#include "view/button_close.hpp"
#include "bilibili/result/dynamic_article.h"
#include "presenter/dynamic_video.hpp"
#include "presenter/comment_related.hpp"

class UserInfoView;
class TextBox;
class DynamicVideoCardView;
class SVGImage;

class DynamicArticleDetail : public brls::Box, public CommentRequest, public DynamicArticleRequest {
public:
    DynamicArticleDetail(const bilibili::DynamicArticleResult& data,
                         const bilibili::DynamicArticleModuleState& state);

    explicit DynamicArticleDetail(const std::string& id);

    void initList(const bilibili::DynamicArticleResult& data,
              const bilibili::DynamicArticleModuleState& state);

    void onCommentInfo(const bilibili::VideoCommentResultWrapper& result) override;

    void onDynamicArticle(const bilibili::DynamicArticleResult& result) override;

    void toggleCommentMode();

    brls::Event<size_t> likeStateEvent;
    brls::Event<size_t> likeNumEvent;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "dynamic/detail/grid");
    BRLS_BIND(ButtonClose, buttonClose, "button/close");
    bilibili::DynamicArticleResult data;
    bilibili::DynamicArticleModuleState state;
};


class DynamicArticleView : public RecyclingGridItem {
public:
    DynamicArticleView();

    void setCard(const bilibili::DynamicArticleResult& result);

    void setForwardCard(const bilibili::dynamic_forward::DynamicArticleResult& result);

    void openDetail();

    void prepareForReuse() override;

    void cacheForReuse() override;

    void setGrow();

    void setLiked(bool liked);

    void setLikeNum(size_t num);

    void setReplyNum(size_t num);

    void setForwardNum(size_t num);

    static RecyclingGridItem* create();

private:
    // 作者
    BRLS_BIND(UserInfoView, author, "dynamic/author");
    // 动态话题
    BRLS_BIND(brls::Label, labelTopic, "dynamic/topic");
    // 动态警告
    BRLS_BIND(brls::Label, labelDispute, "dynamic/dispute");
    // 动态文本
    BRLS_BIND(TextBox, textBox, "dynamic/content");
    // 动态图片
    BRLS_BIND(TextBox, imageBox, "dynamic/image");
    // 动态 转发/评论/点赞 信息
    BRLS_BIND(brls::Label, labelFroward, "comment/label/forward");
    BRLS_BIND(brls::Label, labelReply, "comment/label/reply");
    BRLS_BIND(brls::Label, labelLike, "comment/label/like");
    BRLS_BIND(SVGImage, svgLike, "comment/svg/like");
    // 动态不同分区
    BRLS_BIND(brls::Box, disputeArea, "dynamic/dispute_box");
    BRLS_BIND(brls::Box, topicArea, "dynamic/topic_box");
    BRLS_BIND(brls::Box, contentArea, "dynamic/content_box");
    BRLS_BIND(brls::Box, imageArea, "dynamic/image_box");
    BRLS_BIND(DynamicVideoCardView, videoArea, "dynamic/video_box");
    BRLS_BIND(brls::Box, forwardArea, "dynamic/forward_box");

    /// 转发内容
    // 作者
    BRLS_BIND(brls::Label, authorForward, "dynamic/author/forward");
    // 动态警告
    BRLS_BIND(brls::Label, labelDisputeForward, "dynamic/dispute/forward");
    // 动态话题
    BRLS_BIND(brls::Label, labelTopicForward, "dynamic/topic/forward");
    // 动态文本
    BRLS_BIND(TextBox, textBoxForward, "dynamic/content/forward");
    // 动态图片
    BRLS_BIND(TextBox, imageBoxForward, "dynamic/image/forward");
    // 动态不同分区
    BRLS_BIND(brls::Box, disputeAreaForward, "dynamic/dispute_box/forward");
    BRLS_BIND(brls::Box, topicAreaForward, "dynamic/topic_box/forward");
    BRLS_BIND(brls::Box, contentAreaForward, "dynamic/content_box/forward");
    BRLS_BIND(brls::Box, imageAreaForward, "dynamic/image_box/forward");
    BRLS_BIND(DynamicVideoCardView, videoAreaForward, "dynamic/video_box/forward");

    // 动态数据
    bilibili::DynamicArticleResult articleData;
    // 转评赞数据
    bilibili::DynamicArticleModuleState state;
};