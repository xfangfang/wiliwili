//
// Created by fang on 2022/5/30.
//

#pragma once

#include <borealis/views/label.hpp>
#include "utils/image_helper.hpp"
#include "utils/number_helper.hpp"

enum class RichTextType { Text, Image, Break };

class RichTextComponent {
public:
    explicit RichTextComponent(RichTextType type) : type(type) {}

    void setPosition(float x, float y) {
        this->x = x;
        this->y = y;
    }

    RichTextType type;
    float x = 0, y = 0;

    // 左侧与右侧的空隙
    float l_margin = 0;
    float r_margin = 0;

    // 垂直偏移，目前仅图片组件用到
    float v_align = 0;

    // 顶部空隙，目前仅图片组件用到
    float t_margin = 0;
};

/// 富文本 文字组件
class RichTextSpan : public RichTextComponent {
public:
    RichTextSpan(std::string t, NVGcolor c = nvgRGB(0, 0, 0))
        : RichTextComponent(RichTextType::Text), text(std::move(t)), color(c) {}

    std::string text;
    float fontSize = 0;
    NVGcolor color;
};

/// 富文本 图片组件
class RichTextImage : public RichTextComponent {
public:
    RichTextImage(std::string url, float width, float height, bool autoLoad = false);

    ~RichTextImage();

    std::string url;
    brls::Image* image;
    float width, height;
};

/// 富文本 换行组件
class RichTextBreak : public RichTextComponent {
public:
    RichTextBreak();
};

typedef std::vector<std::shared_ptr<RichTextComponent>> RichTextData;

/**
 * 富文本
 * 支持不同颜色文字与图片绘制
 * 支持在宽度固定时根据最大行数的限制自动计算组件高度
 * 其他布局上暂时没有完整实现，比如：不会根据指定的高度自动判断最大行数，不会在指定高度时自动计算宽度
 */
class TextBox : public brls::Label {
public:
    TextBox();

    /**
     * 设置绘制的最大行数，若宽度已知、高度未知，会根据最大行数推断组件高度
     */
    void setMaxRows(size_t value);

    [[nodiscard]] size_t getMaxRows() const;

    /**
     * 设置若内容超出最大行数，是否在最后一行展示 "更多" 提示
     */
    void setShowMoreText(bool value);

    [[nodiscard]] bool isShowMoreText() const;

    /**
     * 设置富文本内容
     */
    void setRichText(const RichTextData& value);

    RichTextData& getRichText();

    void setText(const std::string& text) override;

    void onLayout() override;

    /**
     * 内部使用
     * 标记为真后，onLayout 执行时不会重新按行分割文本 ( cutRichTextLines() )
     * 当指定宽度，高度位置时，会自动计算高度，在计算的过程中已经做好了分割文本，这时候可以设置标记，可以减少计算
     */
    void setParsedDone(bool value) { this->parsedDone = value; }

    /**
     * 按行重新分割富文本数据
     * @param width 设定分割的宽度
     * @return 总高度
     */
    float cutRichTextLines(float width);

    /**
     * 获取第 line 行的 Y 值
     * @return
     */
    float getLineY(size_t line);

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    static brls::View* create();

    ~TextBox() override;

protected:
    // 最大的行数
    size_t maxRows = SIZE_T_MAX;
    // 若达到最大行数是否在底部展示提示
    bool showMoreText = false;
    // 富文本数据
    RichTextData richContent;
    // 按行分割的富文本数据。开发者设置富文本数据后，会按行重新分割。
    std::vector<RichTextData> lineContent;

    bool parsedDone = false;
};