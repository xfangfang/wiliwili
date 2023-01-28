//
// Created by fang on 2022/5/30.
//

#pragma once

#include <borealis.hpp>

class TextBox : public brls::Label {
public:
    TextBox();

    void setMaxRows(int value);

    int getMaxRows() const;

    void setShowMoreText(bool value);

    bool isShowMoreText() const;

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    static brls::View* create();

protected:
    int maxRows       = -1;
    bool showMoreText = false;
};