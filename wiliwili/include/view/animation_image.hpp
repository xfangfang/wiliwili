//
// Created by fang on 2022/12/27.
//

// register this view in main.cpp
//#include "view/animationImage.hpp"
//    brls::Application::registerXMLView("AnimationImage", AnimationImage::create);
// <brls:View xml=@res/xml/views/animationImage.xml

#pragma once

#include <borealis/core/box.hpp>

class AnimationImage : public brls::Box {
public:
    AnimationImage();

    ~AnimationImage();

    void setFrame(size_t value);

    void setImage(const std::string& value);

    void setFrameTime(size_t value);

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    void onLayout() override;

    static View* create();

protected:
    size_t frame             = 1;
    size_t current_frame     = 0;
    size_t last_refresh_time = 0;
    size_t last_x            = 0;
    size_t texture           = 0;
    size_t frame_time        = 41666;  // 1/24 s
    NVGpaint paint;

    void refreshImage();
};