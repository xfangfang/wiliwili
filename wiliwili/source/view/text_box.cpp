//
// Created by fang on 2022/12/4.
//

#include "view/text_box.hpp"

TextBox::TextBox() { this->brls::Label::setAnimated(false); }

void TextBox::draw(NVGcontext* vg, float x, float y, float width, float height,
                   brls::Style style, brls::FrameContext* ctx) {
    if (width == 0) return;

    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);
    nvgFontSize(vg, this->getFontSize());
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, this->getFont());
    nvgTextLineHeight(vg, this->getLineHeight());
    nvgFillColor(vg, a(this->getTextColor()));
    nvgTextBox(vg, x, y, width, this->getFullText().c_str(), nullptr);

    nvgRestore(vg);
}

brls::View* TextBox::create() { return new TextBox(); }