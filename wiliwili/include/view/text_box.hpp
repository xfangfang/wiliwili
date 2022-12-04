//
// Created by fang on 2022/5/30.
//

#pragma once

#include <borealis.hpp>

class TextBox : public brls::Label {
public:
    TextBox();

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx);

    static brls::View* create();
};