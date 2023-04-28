//
// Created by fang on 2022/8/15.
//

// register this view in main.cpp
//#include "view/video_progress_slider.hpp"
//    brls::Application::registerXMLView("VideoProgressSlider", VideoProgressSlider::create);

#pragma once

#include <borealis.hpp>

class SVGImage;

class VideoProgressSlider : public brls::Box {
public:
    VideoProgressSlider();

    ~VideoProgressSlider() override;

    static brls::View* create();

    void onLayout() override;

    brls::View* getDefaultFocus() override;

    void setProgress(float progress);

    [[nodiscard]] float getProgress() const { return progress; }

    // Progress is manually dragged
    brls::Event<float>* getProgressEvent() { return &progressEvent; }

    // Manual dragging is over
    brls::Event<float>* getProgressSetEvent() { return &progressSetEvent; }

private:
    brls::InputManager* input;
    brls::Rectangle* line;
    brls::Rectangle* lineEmpty;
    SVGImage* pointerIcon;
    brls::Box* pointer;

    brls::Event<float> progressEvent;
    brls::Event<float> progressSetEvent;

    float progress = 1;

    void updateUI();
};