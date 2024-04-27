#include "view/inbox_msg_card.hpp"
#include "view/text_box.hpp"

InboxMsgCard::InboxMsgCard() { this->inflateFromXMLRes("xml/views/inbox_msg.xml"); }

void InboxMsgCard::setCard(const bilibili::InboxMessageResult& r, const IEMap& m, uint64_t talker) {
    RichTextData d;
    auto theme     = brls::Application::getTheme();
    auto textColor = theme.getColor("brls/text");

    // 设置用户头像
    if (r.msg_type >= 10) {
        this->talker->setVisibility(brls::Visibility::GONE);
        this->mine->setVisibility(brls::Visibility::GONE);
    } else if (talker == r.sender_uid) {
        this->talker->setVisibility(brls::Visibility::VISIBLE);
        this->mine->setVisibility(brls::Visibility::INVISIBLE);
    } else {
        this->talker->setVisibility(brls::Visibility::INVISIBLE);
        this->mine->setVisibility(brls::Visibility::VISIBLE);
    }

    switch (r.msg_type) {
        case 1: {  // 文本消息
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
            std::string pic = r.content.at("url");
            float width     = r.content.at("width");
            float height    = r.content.at("height");

            if (width > 400.f) {
                height = height * 400.f / width;
                width  = 400.f;
            }
            d.push_back(std::make_shared<RichTextImage>(pic, width, height));
            break;
        }
        case 7: {  // 分享消息
            std::string title = r.content.at("title");
            std::string thumb = r.content.at("thumb");
            d.push_back(std::make_shared<RichTextSpan>(title, textColor));
            d.push_back(std::make_shared<RichTextImage>(thumb, 400.f, 100.f));
            break;
        }
        case 10: {  // 系统消息
            std::string title = r.content.at("title");
            std::string text  = r.content.at("text");
            d.push_back(std::make_shared<RichTextSpan>(title, theme.getColor("color/bilibili")));
            d.push_back(std::make_shared<RichTextBreak>());
            d.push_back(std::make_shared<RichTextSpan>(text, textColor));
            break;
        }
        default:;
    }

    this->textBox->setRichText(d);
}

void InboxMsgCard::setAvatar(const std::string& face) {
    ImageHelper::with(this->talker)->load(face + ImageHelper::face_ext);
}
