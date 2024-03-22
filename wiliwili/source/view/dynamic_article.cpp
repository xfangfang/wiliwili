//
// Created by fang on 2024/3/22.
//

#include "view/dynamic_article.hpp"
#include "view/user_info.hpp"
#include "view/text_box.hpp"
#include "view/dynamic_video_card.hpp"

#include "utils/image_helper.hpp"
#include "utils/number_helper.hpp"
#include "utils/string_helper.hpp"

DynamicArticleView::DynamicArticleView() {
    this->inflateFromXMLRes("xml/views/dynamic_card.xml");
    author->getLabelName()->setFontSize(18);
    author->getLabelMisc()->setFontSize(16);
    author->getLabelName()->setMarginBottom(4);
    author->getLabelMisc()->setMarginTop(4);
    if (brls::Application::ORIGINAL_WINDOW_HEIGHT == 544) {
        videoArea->setWidthPercentage(80);
        videoAreaForward->setWidthPercentage(80);
    }
}

void DynamicArticleView::setCard(const bilibili::DynamicArticleResult& result) {
    // 清空内容
    this->contentArea->setVisibility(brls::Visibility::GONE);
    this->imageArea->setVisibility(brls::Visibility::GONE);
    this->videoArea->setVisibility(brls::Visibility::GONE);
    this->forwardArea->setVisibility(brls::Visibility::GONE);
    this->topicArea->setVisibility(brls::Visibility::GONE);

    for (auto& j : result.modules) {
        switch ((bilibili::DynamicArticleModuleType)j.data.index()) {
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_AUTHOR: {
                auto& data = std::get<bilibili::DynamicArticleModuleAuthor>(j.data);
                // 作者
                this->author->setUserInfo(data.user.face + ImageHelper::face_ext, data.user.name, data.pub_text);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_DESC: {
                auto& data = std::get<bilibili::DynamicArticleModuleDesc>(j.data);
                // 文本
                this->textBox->setText(data.text);
                this->contentArea->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_DATA: {
                auto& internal = std::get<bilibili::DynamicArticleModuleData>(j.data);
                // 内嵌数据
                switch ((bilibili::DynamicArticleModuleDataType)internal.data.index()) {
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_VIDEO: {
                        auto& video = std::get<bilibili::DynamicArticleModuleArchive>(internal.data);
                        // 视频
                        this->videoArea->setCard(video.cover + ImageHelper::h_ext, video.title, video.stat.play, video.stat.danmaku,
                                                 video.duration_text);
                        this->videoArea->setVisibility(brls::Visibility::VISIBLE);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_IMAGE: {
                        auto& image = std::get<bilibili::DynamicArticleModuleDraw>(internal.data);
                        // 图片
                        RichTextData d;
                        float size = brls::Application::ORIGINAL_WINDOW_HEIGHT == 544 ? 18 : 36;
                        if (image.items.size() <= 3)
                            size *= 4;
                        else if (image.items.size() <= 6)
                            size *= 3;
                        else
                            size *= 2;
                        float margin = brls::Application::ORIGINAL_WINDOW_HEIGHT == 544 ? 4: 8;
                        int i = 0;
                        for (auto& p : image.items) {
                            auto item = std::make_shared<RichTextImage>(p.src + ImageHelper::note_ext, size, size);
                            item->r_margin = margin;
                            item->t_margin = margin;
                            d.emplace_back(item);
                            // 每3张图片，自动换行
                            if (++i % 3 == 0) d.emplace_back(std::make_shared<RichTextBreak>());
                        }
                        this->imageBox->setRichText(d);
                        this->imageArea->setVisibility(brls::Visibility::VISIBLE);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_FORWARD: {
                        auto forward = std::get<bilibili::DynamicArticleModuleForward>(internal.data);
                        // 转发
                        this->forwardArea->setVisibility(brls::Visibility::VISIBLE);
                        this->setForwardCard(forward.item);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_NONE:
                    default:
                        brls::Logger::error("\t unknown module data type: {}", internal.type);
                        break;
                }
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_STAT: {
                auto& data = std::get<bilibili::DynamicArticleModuleState>(j.data);
                // 转发 回复 点赞
                // todo: is_forbidden: 是否禁止转发，评论和点赞是否也有类似情况?
                this->labelFroward->setText(data.forward.count == 0 ? "转发" : wiliwili::num2w(data.forward.count));
                this->labelReply->setText(data.comment.count == 0 ? "评论" : wiliwili::num2w(data.comment.count));
                this->labelLike->setText(data.like.count == 0 ? "点赞" : wiliwili::num2w(data.like.count));
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_TOPIC: {
                // 话题
                auto& data = std::get<bilibili::DynamicArticleModuleTopic>(j.data);
                this->labelTopic->setText(data.name);
                this->topicArea->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_NULL: {
                // 转发的动态已失效
                auto& data = std::get<bilibili::DynamicArticleModuleNull>(j.data);
                this->forwardArea->setVisibility(brls::Visibility::VISIBLE);
                this->contentAreaForward->setVisibility(brls::Visibility::GONE);
                this->imageAreaForward->setVisibility(brls::Visibility::GONE);
                this->videoAreaForward->setVisibility(brls::Visibility::GONE);
                this->topicAreaForward->setVisibility(brls::Visibility::GONE);
                this->authorForward->setText(data.text);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_NONE:
            default:
                brls::Logger::error("\t unknown module type: {}", j.module_type);
                break;
        }
    }
}

void DynamicArticleView::setForwardCard(const bilibili::dynamic_forward::DynamicArticleResult& result) {
    // 清空内容
    this->contentAreaForward->setVisibility(brls::Visibility::GONE);
    this->imageAreaForward->setVisibility(brls::Visibility::GONE);
    this->videoAreaForward->setVisibility(brls::Visibility::GONE);
    this->topicAreaForward->setVisibility(brls::Visibility::GONE);

    for (auto& j : result.modules) {
        switch ((bilibili::DynamicArticleModuleType)j.data.index()) {
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_AUTHOR: {
                auto& data = std::get<bilibili::DynamicArticleModuleAuthor>(j.data);
                // 作者
                this->authorForward->setText("@" + data.user.name);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_DESC: {
                auto& data = std::get<bilibili::DynamicArticleModuleDesc>(j.data);
                // 文本
                this->textBoxForward->setText(data.text);
                this->contentAreaForward->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_DATA: {
                auto& internal = std::get<bilibili::dynamic_forward::DynamicArticleModuleData>(j.data);
                // 内嵌数据
                switch ((bilibili::DynamicArticleModuleDataType)internal.data.index()) {
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_VIDEO: {
                        auto& video = std::get<bilibili::DynamicArticleModuleArchive>(internal.data);
                        // 视频
                        this->videoAreaForward->setCard(video.cover + ImageHelper::h_ext, video.title, video.stat.play,
                                                        video.stat.danmaku, video.duration_text);
                        this->videoAreaForward->setVisibility(brls::Visibility::VISIBLE);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_IMAGE: {
                        auto& image = std::get<bilibili::DynamicArticleModuleDraw>(internal.data);
                        // 图片
                        RichTextData d;
                        float size = brls::Application::ORIGINAL_WINDOW_HEIGHT == 544 ? 18 : 36;
                        size *= 4 - ((image.items.size() - 1) / 3);
                        float margin = brls::Application::ORIGINAL_WINDOW_HEIGHT == 544 ? 4: 8;
                        // 显示为正方形缩略图
                        int i = 0;
                        for (auto& p : image.items) {
                            auto item = std::make_shared<RichTextImage>(p.src + ImageHelper::note_ext, size, size);
                            item->r_margin = margin;
                            item->t_margin = margin;
                            d.emplace_back(item);
                            if (++i % 3 == 0) d.emplace_back(std::make_shared<RichTextBreak>());
                        }
                        this->imageBoxForward->setRichText(d);
                        this->imageAreaForward->setVisibility(brls::Visibility::VISIBLE);
                        break;
                    }
                    case bilibili::DynamicArticleModuleDataType::MODULE_TYPE_NONE:
                    default:
                        brls::Logger::error("\t unknown module data type: {}", internal.type);
                        break;
                }
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_TOPIC: {
                // 话题
                auto& data = std::get<bilibili::DynamicArticleModuleTopic>(j.data);
                this->labelTopicForward->setText(data.name);
                this->topicAreaForward->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case bilibili::DynamicArticleModuleType::MODULE_TYPE_NONE:
            default:
                brls::Logger::error("\t unknown module type: {}", j.module_type);
                break;
        }
    }
}

void DynamicArticleView::prepareForReuse() {}

void DynamicArticleView::cacheForReuse() {
    ImageHelper::clear(this->author->getAvatar());
    if (this->videoArea->getVisibility() == brls::Visibility::VISIBLE) {
        ImageHelper::clear(this->videoArea->picture);
    }
    if (this->videoAreaForward->getVisibility() == brls::Visibility::VISIBLE) {
        ImageHelper::clear(this->videoAreaForward->picture);
    }
}

RecyclingGridItem* DynamicArticleView::create() { return new DynamicArticleView(); }