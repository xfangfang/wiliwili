//
// Created by fang on 2022/7/18.
//

#include "view/video_comment.hpp"
#include "utils/number_helper.hpp"

VideoComment::VideoComment() {
    brls::Logger::debug("View VideoComment: create");
    this->inflateFromXMLRes("xml/views/video_comment.xml");
}

VideoComment::~VideoComment() {
    brls::Logger::debug("View VideoComment: delete");
    ImageHelper::clear(this->userInfo->getAvatar());
}

RecyclingGridItem* VideoComment::create() {
    return new VideoComment();
}

void VideoComment::setData(bilibili::VideoCommentResult data){
    this->comment_data = data;

    this->label->setText(data.content.message);
    this->userInfo->setUserInfo(data.member.avatar, data.member.uname, wiliwili::sec2date(data.ctime));
}

void VideoComment::prepareForReuse(){

}

void VideoComment::cacheForReuse(){
    ImageHelper::clear(this->userInfo->getAvatar());
}