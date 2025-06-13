#include "view/live_danmaku_item.hpp"
#include "view/text_box.hpp"
#include "utils/image_helper.hpp"

// using namespace brls::literals;

LiveDanmakuItemView::LiveDanmakuItemView() {
    this->inflateFromXMLRes("xml/views/live_danmaku_item.xml");
    
    // 将contentLabel替换为TextBox
    this->contentBox = new TextBox();
    this->contentBox->setWidthPercentage(100); // 使用百分比宽度
    this->contentBox->setMarginTop(4);
    this->contentBox->setFontSize(14);
    this->contentBox->setSingleLine(false);
    this->contentBox->setMaxWidthPercentage(95); // 添加最大宽度限制
    
    // 获取contentLabel的父视图
    brls::Box* parent = dynamic_cast<brls::Box*>(this->contentLabel->getParent());
    if (parent) {
        // 替换contentLabel为contentBox
        parent->removeView(this->contentLabel);
        parent->addView(this->contentBox);
    }
}

LiveDanmakuItemView::~LiveDanmakuItemView() {
    // 释放头像图片资源
    if (this->avatarImage) {
        ImageHelper::clear(this->avatarImage);
    }
}

void LiveDanmakuItemView::setDanmaku(const LiveDanmakuItem& danmaku) {
    // 重置item背景颜色为默认透明
    this->setBackgroundColor(nvgRGBA(0, 0, 0, 0));
    
    // 默认隐藏头像
    this->avatarImage->setVisibility(brls::Visibility::GONE);
    
    // 判断是否是超级留言(SC)
    if (danmaku.type == LiveDanmakuItem::Type::SUPER_CHAT) {
        // 处理超级留言
        
        // 设置用户名
        this->usernameLabel->setText(!danmaku.super_chat->user_name.empty() ? danmaku.super_chat->user_name : "用户");
        
        // 隐藏等级
        this->levelBox->setVisibility(brls::Visibility::GONE);
        
        // 处理粉丝牌子
        if (!danmaku.super_chat->fan_medal_name.empty() && danmaku.super_chat->fan_medal_level > 0) {
            // 设置粉丝牌子文本（名称+等级）
            this->fanMedalLabel->setText(danmaku.super_chat->fan_medal_name + 
                                       std::to_string(danmaku.super_chat->fan_medal_level));
            this->fanMedalBox->setVisibility(brls::Visibility::VISIBLE);
        } else {
            this->fanMedalBox->setVisibility(brls::Visibility::GONE);
        }
        
        // 隐藏房管标识和VIP标识
        this->adminBox->setVisibility(brls::Visibility::GONE);
        this->vipBox->setVisibility(brls::Visibility::GONE);
        
        // 设置SC金额
        std::string priceText = "¥" + std::to_string(danmaku.super_chat->price);
        this->scPriceLabel->setText(priceText);
        this->scPriceBox->setVisibility(brls::Visibility::VISIBLE);
        
        // 设置用户头像
        if (!danmaku.super_chat->user_face.empty()) {
            // 显示头像组件
            this->avatarImage->setVisibility(brls::Visibility::VISIBLE);
            // 加载头像图片
            ImageHelper::with(this->avatarImage)->load(danmaku.super_chat->user_face);
        }
        
        // 设置SC内容
        std::string scContent = !danmaku.super_chat->message.empty() ? danmaku.super_chat->message : "";
        
        // 设置SC背景色
        NVGcolor backgroundColor;
        
        // 使用 background_price_color
        if (!danmaku.super_chat->background_price_color.empty()) {
            // 解析十六进制颜色
            std::string colorHex = danmaku.super_chat->background_price_color;
            // 去掉 # 前缀
            if (colorHex.length() > 0 && colorHex[0] == '#') {
                colorHex = colorHex.substr(1);
            }
            
            // 转换为RGB
            unsigned int r = 0, g = 0, b = 0;
            if (colorHex.length() >= 6) {
                if (sscanf(colorHex.c_str(), "%02x%02x%02x", &r, &g, &b) == 3) {
                    backgroundColor = nvgRGBA(r, g, b, 230); // 90%不透明度
                    this->setBackgroundColor(backgroundColor);
                    
                    // 保存原始背景色，用于置顶/取消置顶切换
                    this->originalBgColor = backgroundColor;
                    
                    brls::Logger::debug("SC价格背景色设置成功: #{:02x}{:02x}{:02x}", r, g, b);
                }
            }
        } else {
            backgroundColor = nvgRGBA(49, 113, 210, 230); // 默认SC背景色 (蓝色)
            this->setBackgroundColor(backgroundColor);
            
            // 保存原始背景色，用于置顶/取消置顶切换
            this->originalBgColor = backgroundColor;
            
            brls::Logger::debug("使用默认SC背景色");
        }
        
        // 根据背景颜色亮度计算文本颜色（简化为只使用黑色或白色）
        // 计算颜色亮度 (基于YIQ公式: 亮度 = 0.299*R + 0.587*G + 0.114*B)
        double brightness = (0.299 * (backgroundColor.r * 255) + 
                            0.587 * (backgroundColor.g * 255) + 
                            0.114 * (backgroundColor.b * 255)) / 255;
        
        // 如果亮度较高(浅色背景)，使用深色文本；否则使用浅色文本
        NVGcolor textColor = (brightness > 0.5) ? 
                    nvgRGB(0, 0, 0) :     // 黑色文本(深色)
                    nvgRGB(255, 255, 255); // 白色文本(浅色)
        
        brls::Logger::debug("SC文本颜色根据背景亮度计算: 亮度={:.2f}, 使用{}色文本", 
                           brightness, (brightness > 0.5) ? "黑" : "白");
        
        // 设置用户名颜色与内容颜色一致，提高可读性
        this->usernameLabel->setTextColor(textColor);
        
        // 创建富文本内容
        RichTextData richText;
        auto contentSpan = std::make_shared<RichTextSpan>(scContent, textColor);
        richText.push_back(contentSpan);
        
        // 设置富文本
        this->contentBox->setRichText(richText);
        
        return;
    }
    
    // 以下是普通弹幕的处理逻辑
    
    // 设置用户名
    this->usernameLabel->setText(!danmaku.danmaku->user_name.empty() ? danmaku.danmaku->user_name : "用户");
    
    // 隐藏SC金额标签
    this->scPriceBox->setVisibility(brls::Visibility::GONE);
    
    // 设置用户等级
    this->levelLabel->setText("UL" + std::to_string(danmaku.danmaku->user_level));
    this->levelBox->setVisibility(brls::Visibility::VISIBLE);
    
    // 设置用户等级颜色
    int level = danmaku.danmaku->user_level;
    NVGcolor levelColor = nvgRGBA(92, 179, 239, 255);

    if (level >= 0 && level <= 5) {
        levelColor = nvgRGBA(137, 207, 127, 255);
    } else if (level <= 10) {
        levelColor = nvgRGBA(95, 179, 86, 255);
    } else if (level <= 20) {
        levelColor = nvgRGBA(92, 179, 239, 255);
    } else if (level <= 30) {
        levelColor = nvgRGBA(172, 117, 243, 255);
    } else if (level <= 40) {
        levelColor = nvgRGBA(255, 163, 72, 255);
    } else if (level <= 50) {
        levelColor = nvgRGBA(255, 102, 102, 255);
    } else if (level <= 60) {
        levelColor = nvgRGBA(255, 215, 0, 255);
    }
    this->levelBox->setBackgroundColor(levelColor);

    // 处理粉丝牌子
    if (!danmaku.danmaku->fan_medal_name.empty() && danmaku.danmaku->fan_medal_level > 0) {
        // 设置粉丝牌子文本（名称+等级）
        this->fanMedalLabel->setText(danmaku.danmaku->fan_medal_name + std::to_string(danmaku.danmaku->fan_medal_level));
        
        // 设置粉丝牌子颜色
        NVGcolor medalColor;
        // 使用牌子的开始颜色作为背景色
        if (danmaku.danmaku->fan_medal_start_color != 0) {
            // 解析RGB颜色
            int r = (danmaku.danmaku->fan_medal_start_color >> 16) & 0xFF;
            int g = (danmaku.danmaku->fan_medal_start_color >> 8) & 0xFF;
            int b = danmaku.danmaku->fan_medal_start_color & 0xFF;
            medalColor = nvgRGBA(r, g, b, 255);
        } else {
            // 默认颜色
            medalColor = levelColor;
        }
        this->fanMedalBox->setBackgroundColor(medalColor);
        this->fanMedalBox->setVisibility(brls::Visibility::VISIBLE);
    } else {
        // 没有粉丝牌子，隐藏
        this->fanMedalBox->setVisibility(brls::Visibility::GONE);
    }
    
    // 处理房管标识
    if (danmaku.danmaku->is_guard) {
        this->adminLabel->setText("房管");
        this->adminBox->setVisibility(brls::Visibility::VISIBLE);
        
        // 为房管的弹幕项添加橙色背景（30%透明度）
        this->setBackgroundColor(nvgRGBA(255, 153, 0, 76));
    } else {
        this->adminBox->setVisibility(brls::Visibility::GONE);
    }
    
    // 处理VIP标识（舰长、提督、总督）
    if (danmaku.danmaku->user_vip_level > 0 && danmaku.danmaku->user_vip_level <= 3) {
        std::string vipText;
        NVGcolor vipColor;
        
        switch (danmaku.danmaku->user_vip_level) {
            case 3:
                vipColor = nvgRGBA(92, 179, 239, 255); // 蓝色 - 舰长
                break;
            case 2:
                vipColor = nvgRGBA(172, 117, 243, 255); // 紫色 - 提督
                break;
            case 1:
                vipColor = nvgRGBA(255, 102, 102, 255); // 红色 - 总督
                break;
            default:
                vipColor = nvgRGBA(92, 179, 239, 255); // 默认蓝色
        }
        
        switch (danmaku.danmaku->user_vip_level) {
            case 3: vipText = "舰长"; break;
            case 2: vipText = "提督"; break;
            case 1: vipText = "总督"; break;
            default: vipText = "舰长";
        }
        
        this->vipLabel->setText(vipText);
        this->vipBox->setBackgroundColor(vipColor);
        this->vipBox->setVisibility(brls::Visibility::VISIBLE);
        
        // 只有在不是房管的情况下才设置VIP的背景颜色
        // 这样确保房管的橙色背景优先级高于VIP
        if (!danmaku.danmaku->is_guard) {
            // 直接使用RGB值和30%透明度
            // 从已经计算好的vipColor中提取RGB值
            unsigned char r = vipColor.r * 255;
            unsigned char g = vipColor.g * 255;
            unsigned char b = vipColor.b * 255;
            this->setBackgroundColor(nvgRGBA(r, g, b, 76)); // 30%透明度
        }
    } else {
        this->vipBox->setVisibility(brls::Visibility::GONE);
    }
    
    // 弹幕内容处理
    std::string danmakuText = !danmaku.danmaku->dan.empty() ? danmaku.danmaku->dan : "";
    auto& liveDanmakuCore = LiveDanmakuCore::instance();
    
    // 使用LiveDanmakuCore::determineContentType统一处理弹幕类型
    LiveDanmakuItem::ContentType contentType = danmaku.contentType;
    std::vector<std::pair<size_t, size_t>> emotePositions = danmaku.emotePositions;
    
    // 如果contentType是默认值(TEXT)，则使用核心方法判断
    if (contentType == LiveDanmakuItem::ContentType::TEXT && danmaku.emotePositions.empty()) {
        contentType = liveDanmakuCore.determineContentType(danmaku);
        
        // 对于混合类型弹幕，需要计算表情位置
        if (contentType == LiveDanmakuItem::ContentType::MIXED) {
            emotePositions = liveDanmakuCore.findEmoticons(danmakuText);
        }
    }
    
    // 根据弹幕内容类型渲染
    switch (contentType) {
        case LiveDanmakuItem::ContentType::EMOTICON: {
            // 纯表情弹幕
            RichTextData richText;
            float emoticonSize = 28.0f;
            
            if (liveDanmakuCore.emoticons && liveDanmakuCore.emoticons->find(danmakuText) != liveDanmakuCore.emoticons->end()) {
                std::string baseUrl = (*liveDanmakuCore.emoticons)[danmakuText];
                std::string url = baseUrl + ImageHelper::emoji_size1_ext;
                
                auto emoticonImage = std::make_shared<RichTextImage>(url, emoticonSize, emoticonSize);
                emoticonImage->v_align = 4.0f;
                emoticonImage->t_margin = 4.0f;
                richText.push_back(emoticonImage);
            } else {
                auto textSpan = std::make_shared<RichTextSpan>(danmakuText, this->contentBox->getTextColor());
                richText.push_back(textSpan);
            }
            
            this->contentBox->setRichText(richText);
            break;
        }
        case LiveDanmakuItem::ContentType::MIXED: {
            // 混合表情弹幕
            RichTextData richText;
            size_t lastEnd = 0;
            float emoticonSize = 20.0f;
            
            for (const auto& [start, len] : emotePositions) {
                if (start > lastEnd) {
                    std::string textPart = danmakuText.substr(lastEnd, start - lastEnd);
                    auto textSpan = std::make_shared<RichTextSpan>(textPart, this->contentBox->getTextColor());
                    richText.push_back(textSpan);
                }
                
                std::string emoteName = danmakuText.substr(start, len);
                if (liveDanmakuCore.emoticons && liveDanmakuCore.emoticons->find(emoteName) != liveDanmakuCore.emoticons->end()) {
                    std::string baseUrl = (*liveDanmakuCore.emoticons)[emoteName];
                    std::string url = baseUrl + ImageHelper::emoji_size1_ext;
                    
                    auto emoticonImage = std::make_shared<RichTextImage>(url, emoticonSize, emoticonSize);
                    emoticonImage->v_align = 4.0f;
                    richText.push_back(emoticonImage);
                } else {
                    auto textSpan = std::make_shared<RichTextSpan>(emoteName, this->contentBox->getTextColor());
                    richText.push_back(textSpan);
                }
                
                lastEnd = start + len;
            }
            
            if (lastEnd < danmakuText.length()) {
                std::string textPart = danmakuText.substr(lastEnd);
                auto textSpan = std::make_shared<RichTextSpan>(textPart, this->contentBox->getTextColor());
                richText.push_back(textSpan);
            }
            
            this->contentBox->setRichText(richText);
            break;
        }
        case LiveDanmakuItem::ContentType::TEXT:
        default: {
            // 纯文本弹幕
            this->contentBox->setText(danmakuText);
            break;
        }
    }
}

// 实现置顶状态设置方法
void LiveDanmakuItemView::setPinned(bool pinned) {
    if (this->pinned == pinned) {
        return;
    }
    
    this->pinned = pinned;
}

LiveDanmakuItemView* LiveDanmakuItemView::create() {
    return new LiveDanmakuItemView();
} 