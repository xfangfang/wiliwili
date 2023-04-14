//
// Created by fang on 2022/5/30.
//

#pragma once

#include <borealis.hpp>
#include "utils/image_helper.hpp"
#include "utils/number_helper.hpp"

enum class RichTextType { Text, Image };

class RichTextComponent {
public:
    explicit RichTextComponent(RichTextType type) : type(type) {}

    void setPosition(float x, float y) {
        this->x = x;
        this->y = y;
    }

    RichTextType type;
    float x = 0, y = 0;
};

class RichTextSpan : public RichTextComponent {
public:
    RichTextSpan(std::string t, NVGcolor c = nvgRGB(0, 0, 0))
        : RichTextComponent(RichTextType::Text), text(std::move(t)), color(c) {}

    std::string text;
    NVGcolor color;
};

class RichTextImage : public RichTextComponent {
public:
    RichTextImage(std::string url, float width, float height)
        : RichTextComponent(RichTextType::Image),
          url(std::move(url)),
          width(width),
          height(height) {
        image = new brls::Image();
        image->setWidth(width);
        image->setHeight(height);
        image->setScalingType(brls::ImageScalingType::FIT);
        ImageHelper::with(image)->load(this->url);
    }

    ~RichTextImage() {
        ImageHelper::clear(this->image);
        this->image->setParent(nullptr);
        if (!this->image->isPtrLocked()) {
            delete this->image;
        } else {
            this->image->freeView();
        }
    }

    std::string url;
    brls::Image* image;
    float width, height;
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

    void setText(const std::string& text);

    /**
     * 按行重新分割富文本数据
     */
    void onLayout() override;

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

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
};