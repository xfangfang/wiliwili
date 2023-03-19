//
// Created by fang on 2022/12/4.
//

#include "view/text_box.hpp"

const char* TEXTBOX_MORE = "更多";

static YGSize textBoxMeasureFunc(YGNodeRef node, float width,
                                 YGMeasureMode widthMode, float height,
                                 YGMeasureMode heightMode) {
    auto* textBox        = (TextBox*)YGNodeGetContext(node);
    std::string fullText = textBox->getFullText();

    YGSize size = {
        .width  = width,
        .height = height,
    };

    if (fullText.empty() || isnan(width)) return size;

    NVGcontext* vg = brls::Application::getNVGContext();
    nvgFontSize(vg, textBox->getFontSize());
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, textBox->getFont());
    nvgTextLineHeight(vg, textBox->getLineHeight());

    int maxRows = textBox->getMaxRows();
    if (maxRows <= 0) {
        float bounds[4];
        nvgTextBoxBounds(vg, 0, 0, width, fullText.c_str(), nullptr, bounds);
        size.height = bounds[3] - bounds[1];
        return size;
    }

    std::unique_ptr<NVGtextRow, std::function<void(NVGtextRow*)>> rows(new NVGtextRow[maxRows + 1], [](NVGtextRow* p){delete []p;});
    int numberOfRows = nvgTextBreakLines(vg, fullText.c_str(), nullptr, width,
                                         rows.get(), maxRows + 1);
    if (numberOfRows > maxRows && !textBox->isShowMoreText()) {
        numberOfRows = maxRows;
    }
    // 以最后一行 baseline 为底
    size.height =
        (numberOfRows - 1) * textBox->getFontSize() * textBox->getLineHeight() +
        textBox->getFontSize();
    return size;
}

TextBox::TextBox() {
    this->brls::Label::setAnimated(false);

    this->registerFloatXMLAttribute(
        "maxRows", [this](float value) { this->setMaxRows((int)value); });

    this->registerBoolXMLAttribute(
        "showMore", [this](bool value) { this->setShowMoreText(value); });

    YGNodeSetMeasureFunc(this->ygNode, textBoxMeasureFunc);
}

void TextBox::draw(NVGcontext* vg, float x, float y, float width, float height,
                   brls::Style style, brls::FrameContext* ctx) {
    if (width == 0) return;

    nvgFontSize(vg, this->fontSize);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, this->font);
    nvgTextLineHeight(vg, this->lineHeight);
    nvgFillColor(vg, a(this->textColor));

    // 显示全部文字
    if (maxRows <= 0) {
        nvgTextBox(vg, x, y, width, fullText.c_str(), nullptr);
        return;
    }

    std::unique_ptr<NVGtextRow, std::function<void(NVGtextRow*)>> rows(new NVGtextRow[maxRows + 2], [](NVGtextRow* p){delete []p;});
    const char* end  = nullptr;
    int numberOfRows = nvgTextBreakLines(vg, fullText.c_str(), nullptr, width,
                                         rows.get(), maxRows + 2);

    // 显示全部文字
    if (numberOfRows <= maxRows) {
        nvgTextBox(vg, x, y, width, fullText.c_str(), nullptr);
        return;
    }

    // 当显示 "更多" 提示 且只超出一行时，可以借用提示所在的行
    if (numberOfRows == maxRows + 1 && showMoreText) {
        nvgTextBox(vg, x, y, width, fullText.c_str(), nullptr);
        return;
    }

    // 行数过多，只显示部分内容
    end = rows.get()[maxRows - 1].end;
    nvgTextBox(vg, x, y, width, fullText.c_str(), end);
    if (!showMoreText) return;

    // 显示 "更多" 提示
    static auto linkColor = ctx->theme.getColor("color/link");
    nvgFontSize(vg, 18);
    nvgFillColor(vg, a(linkColor));
    nvgText(vg, x, y + maxRows * this->getFontSize() * this->getLineHeight(),
            TEXTBOX_MORE, nullptr);
}

brls::View* TextBox::create() { return new TextBox(); }

void TextBox::setMaxRows(int value) {
    this->maxRows = value;
    this->invalidate();
}

int TextBox::getMaxRows() const { return this->maxRows; }

void TextBox::setShowMoreText(bool value) {
    this->showMoreText = value;
    this->invalidate();
}
bool TextBox::isShowMoreText() const { return this->showMoreText; }
