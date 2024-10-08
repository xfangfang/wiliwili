//
// Created by fang on 2022/7/18.
//

#include <borealis/core/touch/tap_gesture.hpp>
#include <pystring.h>

#include "view/video_comment.hpp"
#include "view/text_box.hpp"
#include "view/svg_image.hpp"
#include "utils/number_helper.hpp"
#include "utils/string_helper.hpp"

using namespace brls::literals;

enum class CommentElementType {
    EMOTE,
    USER,
    TOPIC,
    JUMP,
    NONE,
};

/// 为评论中的表情包添加分辨率后缀，不加后缀则直接加载原图
static inline std::string parseUrl(const std::string& url, const std::string& extra) {
    // url 中可能已经包含了指定的尺寸，需要移除指定的尺寸并重新添加
    // 示例: http://i0.hdslb.com/bfs/garb/emote_diy/ee38500008e72e5623f8972a6ea3d922.png@162w
    // 绝大多数情况下，额外的尺寸都是不存在的
    for (auto it = url.rbegin(); it != url.rend(); ++it) {
        if (*it == '.') {
            return url + extra;
        } else if (*it == '@') {
            return url.substr(0, url.size() - (it - url.rbegin()) - 1) + extra;
        }
    }
    return url + extra;
}

class CommentElement {
public:
    CommentElement(const std::string& title, CommentElementType type) : title(title), type(type) {}
    std::string title;
    CommentElementType type;
    bool matchDone = false;  // 为真则不参与匹配
};

class CommentElementEmote : public CommentElement {
public:
    CommentElementEmote(const std::string& title, const std::string& url, int size)
        : CommentElement(title, CommentElementType::EMOTE), url(url), size(size) {}
    std::string url;
    int size = 1;
};

class CommentElementUser : public CommentElement {
public:
    CommentElementUser(const std::string& name, uint64_t id)
        : CommentElement("@" + name + " ", CommentElementType::USER), name(name), id(id) {}
    std::string name;
    uint64_t id;
};

class CommentElementTopic : public CommentElement {
public:
    CommentElementTopic(const std::string& topic, const std::string& uri)
        : CommentElement("#" + topic + "#", CommentElementType::TOPIC), topic(topic), uri(uri) {}
    std::string topic;
    std::string uri;
};

enum class CommentElementJumpType {
    SEARCH,
    NONE,
};

class CommentElementJump : public CommentElement {
public:
    CommentElementJump(const std::string& title, const std::string& show, const std::string& icon, int position,
                       CommentElementJumpType type)
        : CommentElement(title, CommentElementType::JUMP),
          showTitle(show),
          icon(icon),
          position(position),
          type(type) {}
    std::string showTitle;
    std::string icon;
    int position                = 1;
    CommentElementJumpType type = CommentElementJumpType::NONE;
};

typedef std::unordered_map<std::string, std::shared_ptr<CommentElement>> CEMap;

VideoComment::VideoComment() {
    brls::Logger::verbose("View VideoComment: create");
    this->inflateFromXMLRes("xml/views/video_comment.xml");

    this->registerColorXMLAttribute("mainTextColor", [this](NVGcolor value) { this->setMainTextColor(value); });

    this->registerFloatXMLAttribute("maxRows", [this](float value) { this->setMaxRows((size_t)value); });
}

VideoComment::~VideoComment() { brls::Logger::verbose("View VideoComment: delete"); }

RecyclingGridItem* VideoComment::create() { return new VideoComment(); }

void VideoComment::setMainTextColor(NVGcolor color) {
    this->commentContent->setTextColor(color);
    this->userInfo->setMainTextColor(color);
}

void VideoComment::setMaxRows(size_t value) { this->commentContent->setMaxRows(value); }

void VideoComment::setData(bilibili::VideoCommentResult data) {
    this->comment_data = data;

    std::string subtitle = wiliwili::sec2date(data.ctime);
    if (!data.reply_control.location.empty()) {
        subtitle += "  " + data.reply_control.location;
    }

    // 结尾加个空格用来正确识别尾部的@
    RichTextData d;
    std::string msg    = data.content.message + " ";
    auto theme         = brls::Application::getTheme();
    NVGcolor textColor = theme.getColor("brls/text");
    NVGcolor linkColor = theme.getColor("color/link");
    NVGcolor biliColor = theme.getColor("color/bilibili");

    if (data.top) {
        auto top      = std::make_shared<RichTextSpan>("置顶", biliColor);
        top->r_margin = 10;
        d.emplace_back(top);
    }

    // todo: 识别进度跳转

    // 将表情包、@、跳转、话题 整合进一个map里
    CEMap commentElement;
    for (auto& i : data.content.emote) {
        commentElement.insert({i.first, std::make_shared<CommentElementEmote>(i.first, i.second.url, i.second.size)});
    }
    for (auto& i : data.content.at_name_to_mid) {
        commentElement.insert({"@" + i.first + " ", std::make_shared<CommentElementUser>(i.first, i.second)});
    }
    for (auto& i : data.content.jump_url) {
        commentElement.insert(
            {i.first, std::make_shared<CommentElementJump>(
                          i.first, i.second.title, i.second.prefix_icon, i.second.icon_position,
                          i.second.search ? CommentElementJumpType::SEARCH : CommentElementJumpType::NONE)});
    }
    for (auto& i : data.content.topics_uri) {
        commentElement.insert({"#" + i.first + "#", std::make_shared<CommentElementTopic>(i.first, i.second)});
    }

    // 识别评论组件
    size_t start = 0;
    for (size_t i = 0; i < msg.length(); i++) {
        std::shared_ptr<CommentElement> matchElement = nullptr;
        size_t nextMatch                             = -1;
        for (auto& key : commentElement) {
            if (key.second->matchDone) continue;
            size_t position = msg.find(key.first, i);
            if (position < nextMatch) {
                nextMatch    = position;
                matchElement = key.second;
            }
        }
        if (matchElement == nullptr) nextMatch = msg.length() - 1;
        if (start < nextMatch) {
            // 纯文本
            auto item = std::make_shared<RichTextSpan>(msg.substr(start, nextMatch - start) + "\r", textColor);
            d.emplace_back(item);
        }
        if (matchElement == nullptr) break;
        // 根据 matchElement 类型判断
        switch (matchElement->type) {
            case CommentElementType::EMOTE: {
                auto* t = (CommentElementEmote*)matchElement.get();
                std::shared_ptr<RichTextImage> item;
                if (t->size == 2) {
                    item = std::make_shared<RichTextImage>(parseUrl(t->url, ImageHelper::emoji_size2_ext), 50, 50);
                    item->t_margin = 4;
                } else {
                    item = std::make_shared<RichTextImage>(parseUrl(t->url, ImageHelper::emoji_size1_ext), 30, 30);
                }
                item->v_align  = 4;
                item->l_margin = 2;
                item->r_margin = 2;
                d.emplace_back(item);
                break;
            }
            case CommentElementType::USER: {
                auto* t        = (CommentElementUser*)matchElement.get();
                auto item      = std::make_shared<RichTextSpan>(t->title, linkColor);
                item->l_margin = 8;
                item->r_margin = 8;
                d.emplace_back(item);
                break;
            }
            case CommentElementType::TOPIC: {
                auto* t   = (CommentElementTopic*)matchElement.get();
                auto item = std::make_shared<RichTextSpan>(t->title, linkColor);
                d.emplace_back(item);
                break;
            }
            case CommentElementType::JUMP: {
                auto* t = (CommentElementJump*)matchElement.get();
                if (!t->icon.empty() && t->position == 0) {
                    auto item     = std::make_shared<RichTextImage>(t->icon, 30, 30);
                    item->v_align = 5;
                    d.emplace_back(item);
                }
                d.emplace_back(std::make_shared<RichTextSpan>(t->showTitle, linkColor));
                if (!t->icon.empty() && t->position == 1) {
                    auto item      = std::make_shared<RichTextImage>(t->icon, 16, 30);
                    item->v_align  = 5;
                    item->r_margin = 2;
                    d.emplace_back(item);
                }
                // 关键字只匹配一次
                commentElement[t->title]->matchDone = true;
                break;
            }
            case CommentElementType::NONE:
                break;
        }

        i     = nextMatch + matchElement->title.length() - 1;
        start = i + 1;
    }

    // 笔记图片
    if (!data.content.pictures.empty()) d.emplace_back(std::make_shared<RichTextSpan>("\n\n", textColor));

    static constexpr float size    = 108;
    static constexpr float maxSize = 324;
    if (data.content.pictures.size() == 1) {
        // 只有一张图片时，按图片的比例显示
        auto& picture = data.content.pictures[0];
        float w = size, h = size;
        if (picture.img_height == 0 || picture.img_width == 0) {
        } else if (picture.img_height > picture.img_width) {
            h = picture.img_height / picture.img_width * w;
            if (h > maxSize) h = maxSize;
        } else {
            w = picture.img_width / picture.img_height * h;
            if (w > maxSize) w = maxSize;
        }

        const std::string custom_ext_jpg = "@{}w_{}h_85q_!note-comment-multiple.jpg";
        std::string custom_ext           = ImageHelper::note_custom_ext;
        if (picture.img_src.size() > 4 && picture.img_src.substr(picture.img_src.size() - 4, 4) == ".gif") {
            // gif 图片暂时按照 jpg 来解析
            custom_ext = custom_ext_jpg;
        }
        auto item = std::make_shared<RichTextImage>(picture.img_src + wiliwili::format(custom_ext,
#ifdef __PSV__
                                                                                       (int)(w * 0.5), (int)(h * 0.5)),
#else
                                                                                       (int)(w * 5), (int)(h * 5)),
#endif
                                                    w, h);
        item->t_margin = 8;
        d.emplace_back(item);
    } else {
        // 多张图片显示为正方形缩略图
        for (auto& picture : data.content.pictures) {
            auto item      = std::make_shared<RichTextImage>(picture.img_src + ImageHelper::note_ext, size, size);
            item->r_margin = 8;
            item->t_margin = 8;
            d.emplace_back(item);
        }
    }

    // 设置富文本
    this->commentContent->setRichText(d);

    // 设置用户信息
    this->userInfo->setUserInfo(data.member.avatar + ImageHelper::face_ext, data.member.uname, subtitle);
    if (data.member.vip.nickname_color.empty()) {
        this->userInfo->setMainTextColor(brls::Application::getTheme().getColor("brls/text"));
    } else {
        this->userInfo->getLabelName()->applyXMLAttribute("textColor", data.member.vip.nickname_color);
    }

    // 设置用户等级
    int lv = data.member.level_info.current_level;
    if (lv < 0 || lv > 6) {
        this->userLevel->setVisibility(brls::Visibility::GONE);
    } else {
        this->userLevel->setVisibility(brls::Visibility::VISIBLE);
        if (data.member.is_uploader) {
            this->userLevel->setImageFromSVGRes(fmt::format("svg/user-up.svg", lv));
        } else if (lv == 6 && data.member.is_senior_member) {
            this->userLevel->setImageFromSVGRes("svg/user-lv6p.svg");
        } else {
            this->userLevel->setImageFromSVGRes(fmt::format("svg/user-lv{}.svg", lv));
        }
    }

    // 设置点赞状态
    // action, 0: 未点赞, 1: 已点赞, 2: 已点踩
    this->setLiked(data.action);
    this->setLikeNum(data.like);
    this->setReplyNum(data.rcount);
}

bilibili::VideoCommentResult VideoComment::getData() { return this->comment_data; }

void VideoComment::setReplyNum(size_t num) {
    this->comment_data.rcount = num;
    this->labelReply->setText(wiliwili::num2w(num));
}

void VideoComment::setLikeNum(size_t num) {
    this->comment_data.like = num;
    this->labelLike->setText(wiliwili::num2w(num));
}

void VideoComment::setLiked(size_t action) {
    this->comment_data.action = action;
    if (action == 1) {
        this->svgLike->setImageFromSVGRes("svg/comment-agree-active.svg");
        this->svgDislike->setImageFromSVGRes("svg/comment-disagree-grey.svg");
    } else if (action == 2) {
        this->svgLike->setImageFromSVGRes("svg/comment-agree-grey.svg");
        this->svgDislike->setImageFromSVGRes("svg/comment-disagree-active.svg");
    } else {
        this->svgLike->setImageFromSVGRes("svg/comment-agree-grey.svg");
        this->svgDislike->setImageFromSVGRes("svg/comment-disagree-grey.svg");
    }
}

void VideoComment::prepareForReuse() { this->userInfo->getAvatar()->setImageFromRes("pictures/default_avatar.png"); }

void VideoComment::hideReplyIcon(bool hide) {
    if (hide) {
        this->labelReply->setVisibility(brls::Visibility::GONE);
        this->svgReply->setVisibility(brls::Visibility::GONE);
    } else {
        this->svgReply->setVisibility(brls::Visibility::VISIBLE);
        this->labelReply->setVisibility(brls::Visibility::VISIBLE);
    }
}

void VideoComment::cacheForReuse() { ImageHelper::clear(this->userInfo->getAvatar()); }

/// GridHintView

GridHintView::GridHintView() {
    this->setFocusable(false);
    hintLabel = new brls::Label();
    hintLabel->setFontSize(16);
    hintLabel->setMarginLeft(8);
    hintLabel->setTextColor(brls::Application::getTheme().getColor("font/grey"));
    this->addView(hintLabel);
}

RecyclingGridItem* GridHintView::create() { return new GridHintView(); }

/// VideoCommentReply

VideoCommentReply::VideoCommentReply() {
    auto theme = brls::Application::getTheme();
    this->setFocusable(true);
    this->setHeight(40);
    this->setJustifyContent(brls::JustifyContent::FLEX_START);
    this->setCornerRadius(8);
    this->setBackgroundColor(theme.getColor("color/grey_2"));
    hintLabel = new brls::Label();
    this->hintLabel->setMarginLeft(20);
    this->hintLabel->setFontSize(18);
    this->hintLabel->setText("wiliwili/player/single_comment/hint"_i18n);
    this->hintLabel->setTextColor(theme.getColor("font/grey"));
    this->addView(hintLabel);
}

RecyclingGridItem* VideoCommentReply::create() { return new VideoCommentReply(); }

/// VideoCommentSort

VideoCommentSort::VideoCommentSort() {
    auto theme = brls::Application::getTheme();
    this->setFocusable(true);
    this->setHeight(30);
    this->setJustifyContent(brls::JustifyContent::SPACE_BETWEEN);
    hintLabel = new brls::Label();
    this->hintLabel->setFontSize(18);
    this->addView(hintLabel);

    auto* rightBox = new brls::Box();
    rightBox->setAlignItems(brls::AlignItems::CENTER);
    svgImage = new SVGImage();
    this->svgImage->setSize(brls::Size(16, 16));
    this->svgImage->setImageFromSVGRes("svg/bpx-svg-sprite-sort.svg");
    rightBox->addView(svgImage);
    sortLabel = new brls::Label();
    this->sortLabel->setMarginLeft(4);
    this->sortLabel->setFontSize(16);
    this->sortLabel->setTextColor(theme.getColor("font/grey"));
    rightBox->addView(sortLabel);
    this->addView(rightBox);
    this->setPaddingBottom(6);
    this->setHideClickAnimation(true);
    this->addGestureRecognizer(new brls::TapGestureRecognizer(this));
}

RecyclingGridItem* VideoCommentSort::create() { return new VideoCommentSort(); }