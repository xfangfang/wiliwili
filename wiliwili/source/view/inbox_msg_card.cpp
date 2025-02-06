#include "view/inbox_msg_card.hpp"
#include "view/text_box.hpp"
#include "utils/string_helper.hpp"

using namespace brls::literals;

InboxMsgCard::InboxMsgCard() { this->inflateFromXMLRes("xml/views/inbox_msg.xml"); }

void InboxMsgCard::setCard(const bilibili::InboxMessageResult& r, const IEMap& m, uint64_t talker) {
    RichTextData d;
    auto theme     = brls::Application::getTheme();
    auto textColor = theme.getColor("brls/text");

    // 设置用户头像
    if (r.msg_type == 10) {
        this->talker->setVisibility(brls::Visibility::GONE);
        this->mine->setVisibility(brls::Visibility::GONE);
    } else if (talker == r.sender_uid) {
        this->talker->setVisibility(brls::Visibility::VISIBLE);
        this->mine->setVisibility(brls::Visibility::INVISIBLE);
    } else {
        this->talker->setVisibility(brls::Visibility::INVISIBLE);
        this->mine->setVisibility(brls::Visibility::VISIBLE);
    }

    this->msgTime->setText(wiliwili::sec2FullDate(r.timestamp));

    // 分享消息
    if (r.msg_type == 7) {
        if (r.content.contains("title")) {
            std::string title = r.content.at("title");
            this->shareMisc->setText(title);
        }
        if (r.content.contains("thumb")) {
            std::string thumb = r.content.at("thumb");
            ImageHelper::with(this->shareThumb)->load(thumb + ImageHelper::h_ext);
        }
        if (r.content.contains("author")) {
            std::string author = r.content.at("author");
            this->shareAuthor->setText(author);
        } else if (r.content.contains("source_desc")) {
            std::string desc = r.content.at("source_desc");
            this->shareAuthor->setText(desc);
        }
        this->shareBox->setVisibility(brls::Visibility::VISIBLE);
        this->msgBox->setVisibility(brls::Visibility::GONE);
        return;
    }

    this->shareBox->setVisibility(brls::Visibility::GONE);
    this->msgBox->setVisibility(brls::Visibility::VISIBLE);

    switch (r.msg_type) {
        case 1: {  // 文本消息
            if (!r.content.contains("content")) break;
            std::string msg = r.content.at("content");
            size_t start    = 0;
            for (size_t i = 0; i < msg.length(); i++) {
                InboxEmotePtr matched = nullptr;
                size_t nextMatch      = -1;
                for (auto& key : m) {
                    size_t position = msg.find(key.first, i);
                    if (position < nextMatch) {
                        nextMatch = position;
                        matched   = key.second;
                        break;
                    }
                }
                if (matched == nullptr) nextMatch = msg.length();
                if (start < nextMatch) {
                    // 纯文本
                    std::string text = msg.substr(start, nextMatch - start);
                    d.push_back(std::make_shared<RichTextSpan>(text, textColor));
                }
                if (matched == nullptr) break;

                // 处理表情
                std::shared_ptr<RichTextImage> item;
                if (matched->size == 2) {
                    item           = std::make_shared<RichTextImage>(matched->url, 50, 50);
                    item->t_margin = 4;
                } else {
                    item = std::make_shared<RichTextImage>(matched->url, 30, 30);
                }
                item->v_align  = 4;
                item->l_margin = 2;
                item->r_margin = 2;
                d.push_back(item);

                i     = nextMatch + matched->text.length() - 1;
                start = i + 1;
            }
            break;
        }
        case 2: {  // 图片消息
            if (!r.content.contains("url")) break;
            std::string pic = r.content.at("url");
            float width     = r.content.at("width");
            float height    = r.content.at("height");

            if (width > 400.f) {
                height = height * 400.f / width;
                width  = 400.f;
            }
            if (height > 400.f) {
                width  = width * 400.f / height;
                height = 400.f;
            }

            textBox->setLineHeight(1.0f);
            d.push_back(std::make_shared<RichTextImage>(
                ImageHelper::parseNoteImageUrl(pic, width * ImageHelper::note_small, height * ImageHelper::note_small),
                width, height, 8));
            break;
        }
        case 10: {  // 系统消息
            if (!r.content.contains("title")) break;
            auto titleColor   = theme.getColor("color/bilibili");
            std::string title = r.content.at("title");
            std::string text  = r.content.at("text");
            d.push_back(std::make_shared<RichTextSpan>(title, titleColor));
            // todo: 仔细研究一下开头的 \n 到底在什么情况下有效果
            if (!text.empty() && text[0] != '\n')
                d.push_back(std::make_shared<RichTextBreak>());
            d.push_back(std::make_shared<RichTextSpan>(text, textColor));
            break;
        }
        default: {
            auto fontGrey = theme.getColor("font/grey");
            d.push_back(std::make_shared<RichTextSpan>("wiliwili/inbox/chat/unknown"_i18n, fontGrey));
        }
    }

    this->textBox->setRichText(d);
}

void InboxMsgCard::setAvatar(const std::string& face) {
    ImageHelper::with(this->talker)->load(face + ImageHelper::face_ext);
}

void InboxMsgCard::setTimeVisible(bool visible) {
    this->msgTime->setVisibility(visible ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
}