//
// Created by fang on 2022/12/4.
//

#include "view/text_box.hpp"

#include <utility>
#include "bilibili/result/video_detail_result.h"

const char* TEXTBOX_MORE = "更多";

// Modified from https://github.com/memononen/nanovg
// Do not directly modify nanovg for easy upgrade in the future
// Great thanks to nanovg!

inline static float minf(float a, float b) { return a < b ? a : b; }
inline static float maxf(float a, float b) { return a > b ? a : b; }

inline static float quantize(float a, float d) {
    return ((int)(a / d + 0.5f)) * d;
}

inline static float getAverageScale(float* t) {
    float sx = sqrtf(t[0] * t[0] + t[2] * t[2]);
    float sy = sqrtf(t[1] * t[1] + t[3] * t[3]);
    return (sx + sy) * 0.5f;
}

inline static float getFontScale(NVGcontext* ctx) {
    float xform[6];
    nvgCurrentTransform(ctx, xform);
    return minf(quantize(getAverageScale(xform), 0.01f), 4.0f);
}

inline static float getPixelRatio() {
    return (float)brls::Application::windowWidth /
           (float)brls::Application::windowHeight;
}

static void richTextBoxBounds(NVGcontext* ctx, float x, float y,
                              float breakRowWidth, const char* string,
                              const char* end, float lineHeight, float sx,
                              float* bounds) {
    NVGtextRow rows[2];
    float scale    = getFontScale(ctx) * getPixelRatio();
    float invscale = 1.0f / scale;
    int nrows      = 0, i;
    float lineh = 0, rminy = 0, rmaxy = 0;
    float minx = 0, miny = 0, maxx = 0, maxy = 0, endx = 0, endy = 0;

    nvgTextMetrics(ctx, nullptr, nullptr, &lineh);

    minx = maxx = x;
    miny = maxy = y;
    rminy *= invscale;
    rmaxy *= invscale;

    nrows = nvgTextBreakLines(ctx, string, end, breakRowWidth - sx, rows, 1);
    NVGtextRow* row = &rows[0];
    if (nrows > 0) {
        if (row->end - row->start == 1 && row->width / 2 + sx > breakRowWidth) {
            // 只有一个字符且宽度超出了范围
            // 这里使用 row->width / 2 来判断是因为 nanovg在这种情况下会错误的返回前两个字符的宽度
            // 当前行不变
        } else {
            minx   = minf(minx, x + row->minx);
            maxx   = maxf(maxx, x + row->maxx);
            miny   = minf(miny, y + rminy);
            maxy   = maxf(maxy, y + rmaxy);
            endx   = sx + row->width;
            endy   = y;
            string = row->next;
        }
        y += lineh * lineHeight;
    }

    while (
        (nrows = nvgTextBreakLines(ctx, string, end, breakRowWidth, rows, 2))) {
        for (i = 0; i < nrows; i++) {
            row = &rows[i];
            float rminx, rmaxx, dx = 0;
            dx    = 0;
            endx  = x + row->width;
            endy  = y;
            rminx = x + row->minx + dx;
            rmaxx = x + row->maxx + dx;
            minx  = minf(minx, rminx);
            maxx  = maxf(maxx, rmaxx);
            // Vertical bounds.
            miny = minf(miny, y + rminy);
            maxy = maxf(maxy, y + rmaxy);

            y += lineh * lineHeight;
        }
        string = rows[nrows - 1].next;
    }

    if (bounds != nullptr) {
        bounds[0] = minx;
        bounds[1] = miny;
        bounds[2] = maxx;
        bounds[3] = maxy;
        bounds[4] = endx;
        bounds[5] = endy;
    }
}

inline static std::shared_ptr<RichTextComponent> genRichTextSpan(
    const std::string& text, float x, float y, NVGcolor c) {
    auto item = std::make_shared<RichTextSpan>(text, c);
    item->setPosition(x, y);
    return item;
}

inline static std::shared_ptr<RichTextComponent> genRichTextImage(
    const std::string& url, float width, float height, float x, float y) {
    auto item = std::make_shared<RichTextImage>(url, width, height);
    item->setPosition(x, y);
    return item;
}

RichTextData richTextBreakLines(NVGcontext* ctx, float x, float y,
                                float breakRowWidth, const std::string& text,
                                NVGcolor c, float lineHeight, float sx,
                                float* lx, float* ly) {
    NVGtextRow rows[2];
    int nrows   = 0, i;
    float lineh = 0;
    NVGtextRow* row;
    RichTextData res;
    const char* stringStart = text.c_str();
    const char* string      = stringStart;

    nvgTextMetrics(ctx, nullptr, nullptr, &lineh);

    // 第一行
    nrows =
        nvgTextBreakLines(ctx, string, nullptr, breakRowWidth - sx, rows, 1);
    if (nrows > 0) {
        row = &rows[0];
        if (row->end - row->start == 1 && row->width / 2 + sx > breakRowWidth) {
            // 只有一个字符且宽度超出了范围
            // 这里使用 row->width / 2 来判断是因为 nanovg在这种情况下会错误的返回前两个字符的宽度
            // 添加空白的一行
            res.emplace_back(genRichTextSpan("", x + sx, y, c));
        } else {
            res.emplace_back(genRichTextSpan(
                text.substr(row->start - stringStart, row->end - row->start),
                x + sx, y, c));
            if (lx) *lx = sx + row->width;
            if (ly) *ly = y;
            string = row->next;
        }
        y += lineh * lineHeight;
    }

    // 之后的若干行
    while ((nrows = nvgTextBreakLines(ctx, string, nullptr, breakRowWidth, rows,
                                      2))) {
        for (i = 0; i < nrows; i++) {
            row = &rows[i];
            res.emplace_back(genRichTextSpan(
                text.substr(row->start - stringStart, row->end - row->start), x,
                y, c));
            if (lx) *lx = row->width;
            if (ly) *ly = y;
            y += lineh * lineHeight;
        }
        string = rows[nrows - 1].next;
    }
    return res;
}

// End of nanovg modification

static YGSize textBoxMeasureFunc(YGNodeRef node, float width,
                                 YGMeasureMode widthMode, float height,
                                 YGMeasureMode heightMode) {
    auto* textBox       = (TextBox*)YGNodeGetContext(node);
    auto& richTextData  = textBox->getRichText();
    float lineHeight    = textBox->getLineHeight();
    float pxLineHeight  = textBox->getFontSize() * lineHeight;
    float pxBottomSpace = textBox->getFontSize() * (lineHeight - 1);

    YGSize size = {
        .width  = width,
        .height = height,
    };

    if (richTextData.empty() || isnan(width)) return size;

    NVGcontext* vg = brls::Application::getNVGContext();
    nvgFontSize(vg, textBox->getFontSize());
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, textBox->getFont());
    nvgTextLineHeight(vg, textBox->getLineHeight());

    size_t maxRows = textBox->getMaxRows();
    if (textBox->isShowMoreText() && maxRows != SIZE_T_MAX) maxRows++;
    float maxHeight = maxRows == SIZE_T_MAX
                          ? 0
                          : (float)maxRows * pxLineHeight - pxBottomSpace;

    // 计算最大高度
    float bounds[6];
    float lx = 0, ly = 0;
    for (const auto& i : richTextData) {
        if (i->type == RichTextType::Text) {
            auto* t = (RichTextSpan*)i.get();
            if (t->text.empty()) continue;
            richTextBoxBounds(vg, 0, ly, width, t->text.c_str(), nullptr,
                              lineHeight, lx, bounds);
            lx = bounds[4];
            ly = bounds[5];
        } else if (i->type == RichTextType::Image) {
            auto* t = (RichTextImage*)i.get();
            // 当前行长度不够，就换到下一行
            if (lx + t->width - 2 > width) {
                lx = 0;
                ly += pxLineHeight;
            }
            lx += t->width;
        }
        size.height = ly + textBox->getFontSize();
        if (maxRows != SIZE_T_MAX && size.height > maxHeight) {
            size.height = maxHeight;
            return size;
        }
    }

    return size;
}

TextBox::TextBox() {
    this->brls::Label::setAnimated(false);

    this->registerFloatXMLAttribute(
        "maxRows", [this](float value) { this->setMaxRows((size_t)value); });

    this->registerBoolXMLAttribute(
        "showMore", [this](bool value) { this->setShowMoreText(value); });

    this->registerStringXMLAttribute(
        "text", [this](const std::string& value) { this->setText(value); });

    YGNodeSetMeasureFunc(this->ygNode, textBoxMeasureFunc);

    // todo 因为 nanovg 限制每个富文本结尾的\n和开头的空格不会被渲染
}

void TextBox::setRichText(const RichTextData& value) {
    this->richContent = value;
    // 设置内容后调用 invalidate 会触发 textBoxMeasureFunc 重排布局
    this->invalidate();
}

RichTextData& TextBox::getRichText() { return this->richContent; }

void TextBox::setText(const std::string& text) {
    this->richContent.clear();
    this->richContent.emplace_back(
        std::make_shared<RichTextSpan>(text, this->textColor));
    this->invalidate();
}

void TextBox::onLayout() {
    this->lineContent.clear();
    if (this->richContent.empty()) return;

    float width = this->getWidth();
    auto* vg    = brls::Application::getNVGContext();

    nvgFontSize(vg, this->fontSize);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, this->font);
    nvgTextLineHeight(vg, this->lineHeight);
    nvgFillColor(vg, a(this->textColor));

    float lx = 0, ly = 0;
    RichTextData tempData;
    for (const auto& i : richContent) {
        if (i->type == RichTextType::Text) {
            auto* t = (RichTextSpan*)i.get();
            if (t->text.empty()) continue;
            auto rows = richTextBreakLines(vg, 0, ly, width, t->text, t->color,
                                           this->lineHeight, lx, &lx, &ly);
            if (rows.size() == 1) {
                tempData.emplace_back(rows[0]);
            } else {
                if (!((RichTextSpan*)rows[0].get())->text.empty()) {
                    tempData.emplace_back(rows[0]);
                }
                for (auto it = rows.begin() + 1; it != rows.end(); it++) {
                    if (!tempData.empty())
                        this->lineContent.emplace_back(tempData);
                    tempData.clear();
                    tempData.emplace_back(*it);
                }
            }

        } else if (i->type == RichTextType::Image) {
            auto* t = (RichTextImage*)i.get();

            if (lx + t->width - 2 > width) {
                // 当前行长度不够，就换到下一行
                lx = 0;
                ly += fontSize * lineHeight;
                tempData.emplace_back(
                    genRichTextImage(t->url, t->width, t->height, lx,
                                     ly - (t->height - fontSize) / 2));
                this->lineContent.emplace_back(tempData);
                tempData.clear();
            } else {
                // 当前行可以容纳
                tempData.emplace_back(
                    genRichTextImage(t->url, t->width, t->height, lx,
                                     ly - (t->height - fontSize) / 2));
            }
            lx += t->width;
        }
    }
    if (!tempData.empty()) lineContent.emplace_back(tempData);
}

void TextBox::draw(NVGcontext* vg, float x, float y, float width, float height,
                   brls::Style style, brls::FrameContext* ctx) {
    if (width == 0) return;

    nvgFontSize(vg, this->fontSize);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, this->font);
    nvgTextLineHeight(vg, this->lineHeight);
    nvgFillColor(vg, a(this->textColor));

    // 当显示 "更多" 提示时，最大的绘制行数加1
    size_t drawRow = this->getMaxRows();
    if (drawRow != SIZE_T_MAX && this->showMoreText) {
        drawRow++;
    }

    for (size_t line = 0; line < drawRow && line < lineContent.size(); line++) {
        // 当最后一行给 "更多" 留出空闲区域时，跳出循环
        if (showMoreText && line == drawRow - 1 &&
            lineContent.size() != drawRow)
            break;

        // 绘制第 line 行
        for (auto& i : lineContent[line]) {
            if (i->type == RichTextType::Text) {
                auto* t = (RichTextSpan*)i.get();
                if (t->text.empty()) continue;
                nvgFillColor(vg, a(t->color));
                nvgText(vg, x + t->x, y + t->y, t->text.c_str(), nullptr);
            } else if (i->type == RichTextType::Image) {
                auto* t = (RichTextImage*)i.get();
                t->image->setAlpha(this->getAlpha());
                t->image->draw(vg, x + t->x, y + t->y, t->width, t->height,
                               style, ctx);
            }
        }
    }

    // 已经显示了全部文字
    if (lineContent.size() <= drawRow) {
        return;
    }

    // 显示 "更多" 提示
    if (!showMoreText) return;
    static auto linkColor = ctx->theme.getColor("color/link");
    nvgFontSize(vg, 18);
    nvgFillColor(vg, a(linkColor));
    nvgText(vg, x, y + maxRows * this->getFontSize() * this->getLineHeight(),
            TEXTBOX_MORE, nullptr);
}

brls::View* TextBox::create() { return new TextBox(); }

TextBox::~TextBox() = default;

void TextBox::setMaxRows(size_t value) {
    this->maxRows = value;
    this->invalidate();
}

size_t TextBox::getMaxRows() const { return this->maxRows; }

void TextBox::setShowMoreText(bool value) {
    this->showMoreText = value;
    this->invalidate();
}

bool TextBox::isShowMoreText() const { return this->showMoreText; }
