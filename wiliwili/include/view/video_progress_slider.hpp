//
// Created by fang on 2022/8/15.
//

// register this view in main.cpp
//#include "view/video_progress_slider.hpp"
//    brls::Application::registerXMLView("VideoProgressSlider", VideoProgressSlider::create);


#pragma once

#include <borealis.hpp>


class VideoProgressSlider : public brls::Box {

public:
    VideoProgressSlider();

    ~VideoProgressSlider();

    static brls::View* create();

    void onLayout() override;
    brls::View* getDefaultFocus() override;
    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style, brls::FrameContext* ctx) override;

    void setProgress(float progress);

    float getProgress()
    {
        return progress;
    }

    brls::Event<float>* getProgressEvent()
    {
        return &progressEvent;
    }

  private:
    brls::InputManager* input;
    brls::Rectangle* line;
    brls::Rectangle* lineEmpty;
    brls::Rectangle* pointer;

    brls::Event<float> progressEvent;

    float progress = 1;

    void buttonsProcessing();
    void updateUI();

private:


};