#include "view/live_danmaku_item.hpp"
#include "view/text_box.hpp"
#include "utils/image_helper.hpp"

// using namespace brls::literals;

LiveDanmakuItemView::LiveDanmakuItemView() {
    this->inflateFromXMLRes("xml/views/live_danmaku_item.xml");
    
    // 将contentLabel替换为TextBox
    this->contentBox = new TextBox();
    // this->contentBox->setDetachFromView(true); // 允许脱离视图设置
    this->contentBox->setWidthPercentage(100); // 使用百分比宽度
    // this->contentBox->setHeight(brls::View::AUTO);
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
    // 设置用户名
    this->usernameLabel->setText(danmaku.danmaku->user_name ? danmaku.danmaku->user_name : "用户");
    
    // 设置用户等级
    this->levelLabel->setText("UL" + std::to_string(danmaku.danmaku->user_level));
    
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
    
    // 设置弹幕内容 - 检查是否含有表情
    std::string danmakuText = danmaku.danmaku->dan ? danmaku.danmaku->dan : "";
    
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
            auto textSpan = std::make_shared<RichTextSpan>(danmakuText);
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
                    auto textSpan = std::make_shared<RichTextSpan>(textPart);
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
                    auto textSpan = std::make_shared<RichTextSpan>(emoteName);
                    richText.push_back(textSpan);
                }
                
                lastEnd = start + len;
            }
            
            // 添加剩余文本
            if (lastEnd < danmakuText.length()) {
                std::string textPart = danmakuText.substr(lastEnd);
                auto textSpan = std::make_shared<RichTextSpan>(textPart);
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

LiveDanmakuItemView* LiveDanmakuItemView::create() {
    return new LiveDanmakuItemView();
} 