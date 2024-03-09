//
// Created by fang on 2022/8/15.
//

// register this view in main.cpp
//#include "view/video_progress_slider.hpp"
//    brls::Application::registerXMLView("VideoProgressSlider", VideoProgressSlider::create);

#pragma once

#include <vector>
#include <borealis/core/box.hpp>

namespace brls {
class Rectangle;
}
class SVGImage;

class VideoProgressSlider : public brls::Box {
public:
    VideoProgressSlider();

    ~VideoProgressSlider() override;

    static brls::View* create();

    void onLayout() override;

    brls::View* getDefaultFocus() override;

    void onChildFocusLost(brls::View* directChild, brls::View* focusedView) override;

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    void setProgress(float progress);

    [[nodiscard]] float getProgress() const { return progress; }

    // Progress is manually dragged
    brls::Event<float>* getProgressEvent() { return &progressEvent; }

    // Manual dragging is over
    brls::Event<float>* getProgressSetEvent() { return &progressSetEvent; }

    // Add a chapter point
    void addClipPoint(float point);

    // Clear all the points
    void clearClipPoint();

    void setClipPoint(const std::vector<float>& data);

    const std::vector<float>& getClipPoint();

private:
    brls::InputManager* input;
    brls::Rectangle* line;
    brls::Rectangle* lineEmpty;
    SVGImage* pointerIcon;
    brls::Box* pointer;

    brls::Event<float> progressEvent;
    brls::Event<float> progressSetEvent;

    std::vector<float> clipPointList;

    float progress             = 1;
    bool pointerSelected       = false;
    // while pointer is selected ignore progress setting
    bool ignoreProgressSetting = false;
    // while pointer is selected, the last progress value set by setProgress()
    // is stored here to be restored when canceling the selection
    float lastProgress         = 1;

    void buttonsProcessing();
    void updateUI();
    bool cancelPointerChange();
};