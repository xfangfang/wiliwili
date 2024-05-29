//
// Created by fang on 2022/12/4.
//

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <utility>
#include <codecvt>
#include <locale>
#include <borealis/core/application.hpp>

#include "view/text_box.hpp"

const char* TEXTBOX_MORE = "更多";

// Modified from https://github.com/memononen/nanovg
// Do not directly modify nanovg for easy upgrade in the future
// Great thanks to nanovg!

inline float minf(float a, float b) { return a < b ? a : b; }
inline float maxf(float a, float b) { return a > b ? a : b; }

inline static std::shared_ptr<RichTextComponent> genRichTextSpan(const std::string& text, float x, float y,
                                                                 NVGcolor c) {
    auto item = std::make_shared<RichTextSpan>(text, c);
    item->setPosition(x, y);
    return item;
}

inline static std::shared_ptr<RichTextComponent> genRichTextImage(const std::string& url, float width, float height,
                                                                  float x, float y) {
    auto item = std::make_shared<RichTextImage>(url, width, height, true);
    item->setPosition(x, y);
    return item;
}

RichTextData richTextBreakLines(NVGcontext* ctx, float x, float y, float breakRowWidth, const std::string& text,
                                NVGcolor c, float lineHeight, float sx, float* lx, float* ly) {
    NVGtextRow rows[2];
    int nrows   = 0, i;
    float lineh = 0;
    NVGtextRow* row;
    RichTextData res;
    const char* stringStart = text.c_str();
    const char* string      = stringStart;

    nvgTextMetrics(ctx, nullptr, nullptr, &lineh);

    // 第一行
    nrows = nvgTextBreakLines(ctx, string, nullptr, breakRowWidth - sx, rows, 1);
    if (nrows > 0) {
        row                     = &rows[0];
        std::string currentText = text.substr(row->start - stringStart, row->end - row->start);
        auto firstLine          = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(currentText);
        if (firstLine.length() == 1 && row->width / 2 + sx > breakRowWidth) {
            // 只有一个字符且宽度超出了范围
            // 这里使用 row->width / 2 来判断是因为 nanovg在这种情况下会错误的返回前两个字符的宽度
            // 添加空白的一行
            res.emplace_back(genRichTextSpan("", x + sx, y, c));
        } else {
            res.emplace_back(genRichTextSpan(currentText, x + sx, y, c));
            if (lx) *lx = sx + row->width;
            if (ly) *ly = y;
            string = row->next;
        }
        y += lineh * lineHeight;
    }

    // 之后的若干行
    while ((nrows = nvgTextBreakLines(ctx, string, nullptr, breakRowWidth, rows, 2))) {
        for (i = 0; i < nrows; i++) {
            row = &rows[i];
            res.emplace_back(genRichTextSpan(text.substr(row->start - stringStart, row->end - row->start), x, y, c));
            if (lx) *lx = row->width;
            if (ly) *ly = y;
            y += lineh * lineHeight;
        }
        string = rows[nrows - 1].next;
    }
    return res;
}

// End of nanovg modification

static YGSize textBoxMeasureFunc(YGNodeRef node, float width, YGMeasureMode widthMode, float height,
                                 YGMeasureMode heightMode) {
    auto* textBox      = (TextBox*)YGNodeGetContext(node);
    auto& richTextData = textBox->getRichText();

    YGSize size = {
        .width  = width,
        .height = height,
    };

    if (heightMode == YGMeasureMode::YGMeasureModeExactly) return size;
    if (richTextData.empty() || isnan(width)) return size;

    size.height = textBox->cutRichTextLines(width);
    textBox->setParsedDone(true);
    return size;
}

TextBox::TextBox() {
    this->brls::Label::setAnimated(false);

    this->registerFloatXMLAttribute("maxRows", [this](float value) { this->setMaxRows((size_t)value); });

    this->registerBoolXMLAttribute("showMore", [this](bool value) { this->setShowMoreText(value); });

    this->registerStringXMLAttribute("text", [this](const std::string& value) { this->setText(value); });

    YGNodeSetMeasureFunc(this->ygNode, textBoxMeasureFunc);

    // todo 因为 nanovg 限制每个富文本结尾的\n和开头的空格不会被渲染
}

void TextBox::setRichText(const RichTextData& value) {
#ifdef OPENCC
    static bool trans =
        brls::Application::getLocale() == brls::LOCALE_ZH_HANT || brls::Application::getLocale() == brls::LOCALE_ZH_TW;
    if (trans && OPENCC_ON) {
        this->richContent.clear();
        for (auto& i : value) {
            if (i->type == RichTextType::Text) {
                auto* t = (RichTextSpan*)i.get();
                t->text = Label::STConverter(t->text);
            }
            this->richContent.emplace_back(i);
        }
    } else {
        this->richContent = value;
    }
#else
    this->richContent = value;
#endif
    this->lineContent.clear();
    this->setParsedDone(false);
    // 设置内容后调用 invalidate 会触发 textBoxMeasureFunc 重排布局
    this->invalidate();
}

RichTextData& TextBox::getRichText() { return this->richContent; }

void TextBox::setText(const std::string& value) {
    std::string text;
#ifdef OPENCC
    static bool trans =
        brls::Application::getLocale() == brls::LOCALE_ZH_HANT || brls::Application::getLocale() == brls::LOCALE_ZH_TW;
    if (trans && OPENCC_ON) {
        text = Label::STConverter(value);
    } else {
        text = value;
    }
#else
    text = value;
#endif
    this->richContent.clear();
    this->setParsedDone(false);
    this->richContent.emplace_back(std::make_shared<RichTextSpan>(text, this->textColor));
    this->invalidate();
}

void TextBox::onLayout() {
    float width = getWidth();
    if (isnan(width) || width == 0) return;
    if (this->richContent.empty()) return;
    if (!this->parsedDone) this->cutRichTextLines(width);
}

float TextBox::cutRichTextLines(float width) {
    this->lineContent.clear();
    if (this->richContent.empty()) return 0;

    auto* vg = brls::Application::getNVGContext();

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
            auto rows =
                richTextBreakLines(vg, 0, ly, width, t->text, t->color, this->lineHeight, lx + t->l_margin, &lx, &ly);
            lx += t->r_margin;
            if (rows.empty()) {
                // 应该不会出现这种情况
                brls::Logger::error("TextBox: got empty line: {}", t->text);
            } else if (rows.size() == 1) {
                tempData.emplace_back(rows[0]);
            } else {
                if (!((RichTextSpan*)rows[0].get())->text.empty()) {
                    tempData.emplace_back(rows[0]);
                }
                for (auto it = rows.begin() + 1; it != rows.end(); it++) {
                    if (!tempData.empty()) this->lineContent.emplace_back(tempData);
                    tempData.clear();
                    tempData.emplace_back(*it);
                }
            }

        } else if (i->type == RichTextType::Image) {
            auto* t = (RichTextImage*)i.get();

            if (lx + t->width + 2 + t->l_margin + t->r_margin - 2 > width) {
                // 当前行长度不够，就换到下一行
                // 提交之前的行
                this->lineContent.emplace_back(tempData);
                tempData.clear();
                // 设置下一行的起始位置
                lx = 0;
                ly += fontSize * lineHeight;
            }
            auto item =
                genRichTextImage(t->url, t->width, t->height, lx + t->l_margin, ly - t->height + fontSize + t->v_align);
            item->t_margin = t->t_margin;
            tempData.emplace_back(item);
            lx += t->width + t->l_margin + t->r_margin;
        } else if (i->type == RichTextType::Break) {
            // 提交之前的行
            this->lineContent.emplace_back(tempData);
            tempData.clear();
            // 设置下一行的起始位置
            lx = 0;
            ly += fontSize * lineHeight;
        }
    }
    if (!tempData.empty()) lineContent.emplace_back(tempData);

    // 重新扫描一遍，根据图片高度调整行高
    float height = fontSize * lineHeight;
    float bias   = 0;
    for (auto& i : lineContent) {
        // 获取最大行高
        float maxLineHeight = height;
        for (auto& j : i) {
            if (j->type == RichTextType::Image) {
                auto* t       = (RichTextImage*)j.get();
                maxLineHeight = maxf(maxLineHeight, t->height + t->t_margin);
            }
        }
        bias += maxLineHeight - height;
        for (auto& j : i) {
            j->y += bias;
        }
    }

    float pxBottomSpace = fontSize * (lineHeight - 1);
    size_t rows         = maxRows;
    if (isShowMoreText() && maxRows != SIZE_T_MAX) rows++;

    if (maxRows == SIZE_T_MAX || rows >= lineContent.size()) {
        // 无限制最大行数 或 最大行数大于等于当前行数
        return height * (float)lineContent.size() + bias - pxBottomSpace;
    }

    // 限制最大行数
    return getLineY(maxRows) + fontSize;
}

float TextBox::getLineY(size_t line) {
    if (line >= lineContent.size()) return 0;
    if (lineContent[line].empty()) return 0;
    float y = lineContent[line][0]->y;
    for (auto& i : lineContent[line]) {
        y = minf(y, i->y);
    }
    return y;
}

void TextBox::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                   brls::FrameContext* ctx) {
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
        if (showMoreText && line == drawRow - 1 && lineContent.size() != drawRow) break;

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
                t->image->draw(vg, x + t->x, y + t->y, t->width, t->height, style, ctx);
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
    nvgFontSize(vg, this->fontSize);
    nvgFillColor(vg, a(linkColor));
    nvgText(vg, x, y + getLineY(maxRows), TEXTBOX_MORE, nullptr);
}

brls::View* TextBox::create() { return new TextBox(); }

TextBox::~TextBox() = default;

void TextBox::setMaxRows(size_t value) {
    this->maxRows = value;
    if (!this->richContent.empty()) this->invalidate();
}

size_t TextBox::getMaxRows() const { return this->maxRows; }

void TextBox::setShowMoreText(bool value) {
    this->showMoreText = value;
    this->invalidate();
}

bool TextBox::isShowMoreText() const { return this->showMoreText; }

/// RichTextImage

RichTextImage::RichTextImage(std::string url, float width, float height, bool autoLoad)
    : RichTextComponent(RichTextType::Image), url(std::move(url)), width(width), height(height) {
    image = new brls::Image();
    image->setWidth(width);
    image->setHeight(height);
    image->setCornerRadius(4);
    image->setScalingType(brls::ImageScalingType::FIT);

    if (autoLoad) ImageHelper::with(image)->load(this->url);
}

RichTextImage::~RichTextImage() {
    ImageHelper::clear(this->image);
    this->image->setParent(nullptr);
    if (!this->image->isPtrLocked()) {
        delete this->image;
    } else {
        this->image->freeView();
    }
}

/// RichTextGrow

RichTextBreak::RichTextBreak(): RichTextComponent(RichTextType::Break) {}