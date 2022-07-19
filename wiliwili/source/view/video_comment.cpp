//
// Created by fang on 2022/7/18.
//

#include "view/video_comment.hpp"

VideoComment::VideoComment() {
    brls::Logger::debug("View VideoComment: create");
    this->inflateFromXMLRes("xml/views/video_comment.xml");
}

VideoComment::~VideoComment() {
    brls::Logger::debug("View VideoCommentActivity: delete");
}

RecyclingGridItem* VideoComment::create() {
    return new VideoComment();
}

void VideoComment::setData(bilibili::VideoCommentResult data){
    this->comment_data = data;

    this->label->setText(data.content.message);

}