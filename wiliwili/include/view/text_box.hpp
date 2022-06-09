//
// Created by fang on 2022/5/30.
//

#pragma once

#include <borealis.hpp>

class TextBox: public brls::Label {
public:
    TextBox(){
        this->brls::Label::setAnimated(false);
    }

    void draw(NVGcontext* vg, float x, float y, float width, float height, Style style, FrameContext* ctx)
    {
        if (width == 0)
            return;

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


    static brls::View* create(){
        return new TextBox();
    }

};