//
// Created by fang on 2022/7/18.
//

// register this view in main.cpp
//#include "view/video_comment.hpp"
//    brls::Application::registerXMLView("VideoComment", VideoComment::create);
// <brls:View xml=@res/xml/views/video_comment.xml

#pragma once

#include <borealis.hpp>
#include "bilibili.h"
#include "view/recycling_grid.hpp"

class VideoComment : public RecyclingGridItem {

public:
    VideoComment();

    ~VideoComment();

    static RecyclingGridItem* create();

    void setData(bilibili::VideoCommentResult data);

    void prepareForReuse(){}

    void cacheForReuse(){}

private:
     BRLS_BIND(brls::Label, label, "video/comment/label/content");
     bilibili::VideoCommentResult comment_data;

};