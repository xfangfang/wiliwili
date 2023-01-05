//
// Created by fang on 2022/7/18.
//

// register this view in main.cpp
//#include "view/video_comment.hpp"
//    brls::Application::registerXMLView("VideoComment", VideoComment::create);
// <brls:View xml=@res/xml/views/video_comment.xml

#pragma once

#include <borealis.hpp>
#include "view/recycling_grid.hpp"
#include "view/user_info.hpp"
#include "bilibili/result/video_detail_result.h"

class SVGImage;
class TextBox;
class VideoComment : public RecyclingGridItem {
public:
    VideoComment();

    ~VideoComment();

    static RecyclingGridItem* create();

    void setData(bilibili::VideoCommentResult data);

    void prepareForReuse() override;

    void cacheForReuse() override;

private:
    BRLS_BIND(TextBox, commentContent, "comment/label/content");
    BRLS_BIND(brls::Label, labelLike, "comment/label/like");
    BRLS_BIND(brls::Label, labelReply, "comment/label/reply");
    BRLS_BIND(UserInfoView, userInfo, "comment/userinfo");
    BRLS_BIND(SVGImage, userLevel, "comment/user/level");
    bilibili::VideoCommentResult comment_data;
};