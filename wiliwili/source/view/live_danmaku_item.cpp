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
    
    // 获取contentLabel的父视图
    brls::Box* parent = dynamic_cast<brls::Box*>(this->contentLabel->getParent());
    if (parent) {
        // 替换contentLabel为contentBox
        parent->removeView(this->contentLabel);
        parent->addView(this->contentBox);
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
        
        // 保存SC ID (用户UID)
        this->scId = danmaku.super_chat->user_uid;
        
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
    
    // 清空SC ID
    this->scId = 0;
    
    // 设置用户名
    this->usernameLabel->setText(!danmaku.danmaku->user_name.empty() ? danmaku.danmaku->user_name : "用户");
    
    // 隐藏SC金额标签
    this->scPriceBox->setVisibility(brls::Visibility::GONE);
    
    // 设置用户等级
    this->levelLabel->setText("UL" + std::to_string(danmaku.danmaku->user_level));
    this->levelBox->setVisibility(brls::Visibility::VISIBLE);
    
    // 首先根据用户等级设置不同的颜色
    int level = danmaku.danmaku->user_level;
    NVGcolor levelColor;
    
    if (level >= 0 && level <= 60) {
        // 根据等级设置不同的颜色
        if (level >= 0 && level <= 5) {
            // 新手色 - 浅绿色
            levelColor = nvgRGBA(137, 207, 127, 255);
        } else if (level <= 10) {
            // 初级色 - 绿色
            levelColor = nvgRGBA(95, 179, 86, 255);
        } else if (level <= 20) {
            // 中级色 - 蓝色
            levelColor = nvgRGBA(92, 179, 239, 255);
        } else if (level <= 30) {
            // 高级色 - 紫色
            levelColor = nvgRGBA(172, 117, 243, 255);
        } else if (level <= 40) {
            // 资深色 - 橙色
            levelColor = nvgRGBA(255, 163, 72, 255);
        } else if (level <= 50) {
            // 专家色 - 红色
            levelColor = nvgRGBA(255, 102, 102, 255);
        } else {
            // 大神色 - 金色
            levelColor = nvgRGBA(255, 215, 0, 255);
        }
        this->levelBox->setBackgroundColor(levelColor);
    } else {
        // 根据弹幕类型设置不同的背景色（保留原有的逻辑）
        switch (danmaku.danmaku->dan_type) {
            case 0:  // 普通
                this->levelBox->setBackgroundColor(nvgRGBA(255, 102, 153, 255));
                break;
            case 1:  // 系统
                this->levelBox->setBackgroundColor(nvgRGBA(254, 153, 0, 255));
                break;
            case 2:  // 礼物
                this->levelBox->setBackgroundColor(nvgRGBA(102, 204, 51, 255));
                break;
            default:
                this->levelBox->setBackgroundColor(nvgRGBA(255, 102, 153, 255));
        }
    }
    
    // 处理粉丝牌子
    if (!danmaku.danmaku->fan_medal_name.empty() && danmaku.danmaku->fan_medal_level > 0) {
        // 设置粉丝牌子文本（名称+等级）
        this->fanMedalLabel->setText(danmaku.danmaku->fan_medal_name + std::to_string(danmaku.danmaku->fan_medal_level));
        
        // 设置粉丝牌子颜色
        NVGcolor medalColor;
        // 使用牌子的开始颜色作为背景色
        if (danmaku.danmaku->fan_medal_start_color != 0) {
            // 解析RGB颜色（假设fan_medal_start_color是RGB格式的十进制值）
            int r = (danmaku.danmaku->fan_medal_start_color >> 16) & 0xFF;
            int g = (danmaku.danmaku->fan_medal_start_color >> 8) & 0xFF;
            int b = danmaku.danmaku->fan_medal_start_color & 0xFF;
            medalColor = nvgRGBA(r, g, b, 255);
        } else {
            // 默认颜色
            medalColor = nvgRGBA(136, 136, 136, 255);
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
        // 直接使用RGB值，不需要乘以255
        this->setBackgroundColor(nvgRGBA(255, 153, 0, 76)); // 橙色，30%透明度
    } else {
        this->adminBox->setVisibility(brls::Visibility::GONE);
    }
    
    // 处理VIP标识（舰长、提督、总督）
    if (danmaku.danmaku->user_vip_level > 0 && danmaku.danmaku->user_vip_level <= 3) {
        std::string vipText;
        NVGcolor vipColor;
        
        // 使用粉丝牌子的font_color作为VIP标识的背景色
        int fontColor = danmaku.danmaku->fan_medal_font_color;
        if (fontColor != 0) {
            // 解析RGB颜色
            int r = (fontColor >> 16) & 0xFF;
            int g = (fontColor >> 8) & 0xFF;
            int b = fontColor & 0xFF;
            vipColor = nvgRGBA(r, g, b, 255);
        } else {
            // 没有粉丝牌子颜色时，使用默认VIP颜色
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
    
    // 设置弹幕内容 - 检查是否含有表情
    std::string danmakuText = !danmaku.danmaku->dan.empty() ? danmaku.danmaku->dan : "";
    
    // 获取LiveDanmakuCore的表情映射
    auto& liveDanmakuCore = LiveDanmakuCore::instance();
    
    // 检查是否是单个表情
    if (danmaku.danmaku->is_emoticon || liveDanmakuCore.isEmoticon(danmakuText)) {
        // 是单个表情，创建富文本
        RichTextData richText;
        
        // 表情尺寸
        float emoticonSize = 24.0f; // 相当于文字大小的1.5-2倍
        
        // 检查表情是否存在
        if (liveDanmakuCore.emoticons && liveDanmakuCore.emoticons->find(danmakuText) != liveDanmakuCore.emoticons->end()) {
            // 获取表情URL
            std::string baseUrl = (*liveDanmakuCore.emoticons)[danmakuText];
            
            // 添加表情尺寸后缀
            std::string url = baseUrl + ImageHelper::emoji_size1_ext;
            
            // 创建图片组件
            auto emoticonImage = std::make_shared<RichTextImage>(url, emoticonSize, emoticonSize);
            // 添加垂直对齐调整，向下偏移4像素以避免与用户等级重叠
            emoticonImage->v_align = 4.0f;
            // 添加顶部外边距，增加与其他元素的间距
            emoticonImage->t_margin = 4.0f;
            richText.push_back(emoticonImage);
        } else {
            // 表情不存在，显示文本
            auto textSpan = std::make_shared<RichTextSpan>(danmakuText, this->contentBox->getTextColor());
            richText.push_back(textSpan);
        }
        
        // 设置富文本
        this->contentBox->setRichText(richText);
    } else {
        // 检查是否含有混合表情
        auto emotePositions = liveDanmakuCore.findEmoticons(danmakuText);
        
        if (!emotePositions.empty()) {
            // 包含表情的混合文本
            RichTextData richText;
            size_t lastEnd = 0;
            
            // 表情尺寸
            float emoticonSize = 20.0f;
            
            for (const auto& [start, len] : emotePositions) {
                // 添加表情前的文本
                if (start > lastEnd) {
                    std::string textPart = danmakuText.substr(lastEnd, start - lastEnd);
                    auto textSpan = std::make_shared<RichTextSpan>(textPart, this->contentBox->getTextColor());
                    richText.push_back(textSpan);
                }
                
                // 添加表情
                std::string emoteName = danmakuText.substr(start, len);
                
                // 检查表情是否存在
                if (liveDanmakuCore.emoticons && liveDanmakuCore.emoticons->find(emoteName) != liveDanmakuCore.emoticons->end()) {
                    // 获取表情URL
                    std::string baseUrl = (*liveDanmakuCore.emoticons)[emoteName];
                    
                    // 添加表情尺寸后缀
                    std::string url = baseUrl + ImageHelper::emoji_size1_ext;
                    
                    // 创建图片组件
                    auto emoticonImage = std::make_shared<RichTextImage>(url, emoticonSize, emoticonSize);
                    // 添加垂直对齐调整，向下偏移4像素以避免与用户等级重叠
                    emoticonImage->v_align = 4.0f;
                    richText.push_back(emoticonImage);
                } else {
                    // 表情不存在，显示文本
                    auto textSpan = std::make_shared<RichTextSpan>(emoteName, this->contentBox->getTextColor());
                    richText.push_back(textSpan);
                }
                
                lastEnd = start + len;
            }
            
            // 添加剩余文本
            if (lastEnd < danmakuText.length()) {
                std::string textPart = danmakuText.substr(lastEnd);
                auto textSpan = std::make_shared<RichTextSpan>(textPart, this->contentBox->getTextColor());
                richText.push_back(textSpan);
            }
            
            // 设置富文本
            this->contentBox->setRichText(richText);
        } else {
            // 普通文本，直接设置
            this->contentBox->setText(danmakuText);
        }
    }
}

// 实现置顶状态设置方法
void LiveDanmakuItemView::setPinned(bool pinned) {
    if (this->pinned == pinned) {
        // 状态没有变化，不需要处理
        return;
    }
    
    this->pinned = pinned;
    
    if (pinned) {
        // 设置为置顶状态
        if (this->scId != 0) {
            // 只添加"置顶"标识，不修改颜色
            // 在价格标签旁边添加一个"⭐"符号
            std::string currentText = this->scPriceLabel->getFullText();
            if (currentText.find("⭐") == std::string::npos) {
                this->scPriceLabel->setText("⭐ " + currentText);
            }
        }
    } else {
        // 恢复原始状态
        if (this->scId != 0) {
            // 移除"置顶"标识
            std::string currentText = this->scPriceLabel->getFullText();
            size_t starPos = currentText.find("⭐ ");
            if (starPos != std::string::npos) {
                this->scPriceLabel->setText(currentText.substr(3)); // 移除"⭐ "
            }
        }
    }
}

LiveDanmakuItemView* LiveDanmakuItemView::create() {
    return new LiveDanmakuItemView();
} 