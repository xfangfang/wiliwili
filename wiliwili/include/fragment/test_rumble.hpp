//
// Created by fang on 2022/12/24.
//

// register this fragment in main.cpp
//#include "fragment/test_rumble.hpp"
//    brls::Application::registerXMLView("TestRumble", TestRumble::create);
// <brls:View xml=@res/xml/fragment/test_rumble.xml

#pragma once

#include <thread>
#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

#include "view/video_progress_slider.hpp"

namespace brls {
class Label;
class Button;
}  // namespace brls

class TestRumble : public brls::Box {
public:
    TestRumble();

    ~TestRumble();

    static View* create();

    void startPCM(const std::vector<std::vector<float>>& data);
    void stop();
    void setRumble(float lowFreq, float highFreq, float lowAmp, float highAmp);

    View* getDefaultFocus() override;

private:
    bool playing = false;
    std::thread playThread;
    static std::vector<std::vector<float>> demo1, demo2, demo3;

    float freq_l = 0, amp_l = 0.5;
    float freq_h = 0, amp_h = 0.5;

    BRLS_BIND(VideoProgressSlider, progress1, "p1");
    BRLS_BIND(VideoProgressSlider, progress2, "p2");
    BRLS_BIND(brls::Label, label1, "l1");
    BRLS_BIND(brls::Label, label2, "l2");

    BRLS_BIND(VideoProgressSlider, progress3, "p3");
    BRLS_BIND(VideoProgressSlider, progress4, "p4");
    BRLS_BIND(brls::Label, label3, "l3");
    BRLS_BIND(brls::Label, label4, "l4");

    BRLS_BIND(brls::Button, btn_stop, "btn");
    BRLS_BIND(brls::Button, btn_pcm1, "btn_pcm1");
    BRLS_BIND(brls::Button, btn_pcm2, "btn_pcm2");

    void updateLabel();
};