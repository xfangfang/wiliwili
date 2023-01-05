//
// Created by fang on 2022/7/18.
//

#include "view/video_comment.hpp"
#include "view/text_box.hpp"
#include "view/svg_image.hpp"
#include "utils/number_helper.hpp"
#include "bilibili.h"

VideoComment::VideoComment() {
    brls::Logger::debug("View VideoComment: create");
    this->inflateFromXMLRes("xml/views/video_comment.xml");
}

VideoComment::~VideoComment() {
    brls::Logger::debug("View VideoComment: delete");
}

RecyclingGridItem* VideoComment::create() { return new VideoComment(); }

void VideoComment::setData(bilibili::VideoCommentResult data) {
    this->comment_data = data;

    std::string subtitle = wiliwili::sec2date(data.ctime);
    if (!data.reply_control.location.empty()) {
        subtitle += "  " + data.reply_control.location;
    }

    this->commentContent->setText(data.content.message);
    this->userInfo->setUserInfo(data.member.avatar + ImageHelper::face_ext,
                                data.member.uname, subtitle);

    int lv = data.member.level_info.current_level;
    if (lv < 0 || lv > 6) {
        this->userLevel->setVisibility(brls::Visibility::GONE);
    } else {
        this->userLevel->setVisibility(brls::Visibility::VISIBLE);
        if (data.member.is_uploader) {
            this->userLevel->setImageFromSVGRes(
                fmt::format("svg/user-up.svg", lv));
        } else if (lv == 6 && data.member.is_senior_member) {
            this->userLevel->setImageFromSVGRes("svg/user-lv6p.svg");
        } else {
            this->userLevel->setImageFromSVGRes(
                fmt::format("svg/user-lv{}.svg", lv));
        }
    }

    this->labelLike->setText(wiliwili::num2w(data.like));
    this->labelReply->setText(wiliwili::num2w(data.rcount));

}

void VideoComment::prepareForReuse() {
    this->userInfo->getAvatar()->setImageFromRes("pictures/default_avatar.png");
}

void VideoComment::cacheForReuse() {
    ImageHelper::clear(this->userInfo->getAvatar());
}