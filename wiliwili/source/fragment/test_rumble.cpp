//
// Created by fang on 2022/12/24.
//

#include <borealis/core/thread.hpp>
#include <borealis/views/button.hpp>

#include "fragment/test_rumble.hpp"
#ifdef __SWITCH__
#include <borealis/platforms/switch/switch_input.hpp>
#include <borealis/core/application.hpp>
#endif

std::vector<std::vector<float>> TestRumble::demo1 = {
    {160, 1000.00, 0, 0.87}, {160, 1000.00, 0, 0.98}, {160, 1000.00, 0, 0.97}, {160, 1000.00, 0, 0.96},
    {160, 1000.00, 0, 0.88}, {160, 1357.11, 0, 0.98}, {160, 1388.77, 0, 0.30}, {160, 1357.11, 0, 0.95},
    {160, 1357.11, 0, 0.93}, {160, 1357.11, 0, 0.92}, {160, 1357.11, 0, 0.89}, {160, 1357.11, 0, 0.87},
    {160, 1357.11, 0, 0.88}, {160, 1357.11, 0, 0.88}, {160, 1357.11, 0, 0.87}, {160, 1388.77, 0, 0.16},
    {160, 1357.11, 0, 0.70}, {160, 1357.11, 0, 0.72}, {160, 1357.11, 0, 0.65}, {160, 1357.11, 0, 0.62},
    {160, 1357.11, 0, 0.66}, {160, 1388.77, 0, 0.08}, {160, 1388.77, 0, 0.05}, {160, 1357.11, 0, 0.57},
    {160, 1357.11, 0, 0.51}, {160, 1357.11, 0, 0.41}, {160, 1357.11, 0, 0.35}, {160, 1357.11, 0, 0.40},
    {160, 1357.11, 0, 0.39}, {160, 1357.11, 0, 0.33}, {160, 1357.11, 0, 0.30}, {160, 1388.77, 0, 0.03},
    {160, 1388.77, 0, 0.03}, {160, 1357.11, 0, 0.06}, {160, 1357.11, 0, 0.07}, {160, 1357.11, 0, 0.08},
    {160, 1357.11, 0, 0.08}, {160, 1357.11, 0, 0.09}, {160, 1357.11, 0, 0.07}, {160, 1357.11, 0, 0.04},
    {160, 1357.11, 0, 0.08}, {160, 1388.77, 0, 0.01}, {160, 1388.77, 0, 0.03}, {160, 1388.77, 0, 0.01},
    {160, 1388.77, 0, 0.01}, {160, 1492.93, 0, 0.01}, {160, 1492.93, 0, 0.01}, {160, 1357.11, 0, 0.04},
    {160, 1492.93, 0, 0.01},
};

std::vector<std::vector<float>> TestRumble::demo2 = {
    {160, 500.00, 0, 0.09},  {160, 687.50, 0, 0.17},  {160, 625.00, 0, 0.38},  {160, 625.00, 0, 0.30},
    {160, 437.50, 0, 0.77},  {160, 437.50, 0, 0.68},  {160, 375.00, 0, 0.40},  {160, 562.50, 0, 0.25},
    {160, 500.00, 0, 0.06},  {160, 562.50, 0, 0.42},  {160, 562.50, 0, 0.77},  {160, 625.00, 0, 0.49},
    {160, 1250.00, 0, 0.52}, {160, 937.50, 0, 0.58},  {160, 937.50, 0, 0.77},  {160, 1250.00, 0, 0.77},
    {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77},
    {160, 1250.00, 0, 0.75}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.57}, {160, 1250.00, 0, 0.62},
    {160, 1250.00, 0, 0.59}, {160, 1250.00, 0, 0.68}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77},
    {160, 1250.00, 0, 0.77}, {160, 875.00, 0, 0.61},  {160, 562.50, 0, 0.62},  {160, 562.50, 0, 0.63},
    {160, 562.50, 0, 0.61},  {160, 562.50, 0, 0.62},  {160, 562.50, 0, 0.53},  {160, 500.00, 0, 0.41},
    {160, 500.00, 0, 0.62},  {160, 500.00, 0, 0.63},  {160, 500.00, 0, 0.39},  {160, 500.00, 0, 0.33},
    {160, 437.50, 0, 0.45},  {160, 687.50, 0, 0.62},  {160, 1197.04, 0, 0.48}, {160, 1197.04, 0, 0.67},
    {160, 1197.04, 0, 0.51}, {160, 1250.00, 0, 0.47}, {160, 1250.00, 0, 0.69}, {160, 1250.00, 0, 0.73},
    {160, 1250.00, 0, 0.66}, {160, 1250.00, 0, 0.68}, {160, 1250.00, 0, 0.70}, {160, 1250.00, 0, 0.67},
    {160, 1110.79, 0, 0.74}, {160, 1110.79, 0, 0.71}, {160, 1110.79, 0, 0.66}, {160, 1110.79, 0, 0.70},
    {160, 1110.79, 0, 0.71}, {160, 1110.79, 0, 0.56}, {160, 1250.00, 0, 0.47}, {160, 1250.00, 0, 0.45},
    {160, 1110.79, 0, 0.77}, {160, 1110.79, 0, 0.77}, {160, 1110.79, 0, 0.77}, {160, 1250.00, 0, 0.61},
    {160, 1250.00, 0, 0.38}, {160, 1110.79, 0, 0.77}, {160, 1110.79, 0, 0.77}, {160, 1250.00, 0, 0.77},
    {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77},
    {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77},
    {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77},
    {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.62},
    {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.69}, {160, 1250.00, 0, 0.65},
    {160, 1250.00, 0, 0.61}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77},
    {160, 1250.00, 0, 0.77}, {160, 1250.00, 0, 0.77},
};

TestRumble::TestRumble() {
    this->inflateFromXMLRes("xml/fragment/test_rumble.xml");

    // [40, 626]
    progress1->getProgressEvent()->subscribe([this](float value) {
        value *= 1252;
        this->label1->setText("freq: " + std::to_string(value));
        freq_l = value;
        setRumble(freq_l, freq_h, amp_l, amp_h);
    });
    progress1->setFocusable(false);

    // [0, 1]
    progress2->setProgress(0.5);
    progress2->getProgressEvent()->subscribe([this](float value) {
        this->label2->setText("amp: " + std::to_string(value));
        amp_l = value;
        setRumble(freq_l, freq_h, amp_l, amp_h);
    });
    progress2->setFocusable(false);

    // [80, 1252]
    progress3->getProgressEvent()->subscribe([this](float value) {
        value *= 1252;
        this->label3->setText("freq: " + std::to_string(value));
        freq_h = value;
        setRumble(freq_l, freq_h, amp_l, amp_h);
    });
    progress3->setFocusable(false);

    // [0, 1]
    progress4->setProgress(0.5);
    progress4->getProgressEvent()->subscribe([this](float value) {
        this->label4->setText("amp: " + std::to_string(value));
        amp_h = value;
        setRumble(freq_l, freq_h, amp_l, amp_h);
    });
    progress4->setFocusable(false);

    this->updateLabel();

    btn_stop->registerClickAction([this](...) -> bool {
        stop();
        setRumble(160, 320, 0, 0);
        return true;
    });

    btn_pcm1->registerClickAction([this](...) -> bool {
        startPCM(demo1);
        return true;
    });

    btn_pcm2->registerClickAction([this](...) -> bool {
        startPCM(demo2);
        return true;
    });
}

void TestRumble::stop() {
    this->playing = false;
    if (this->playThread.joinable()) this->playThread.join();
#ifdef __SWITCH__
    ((brls::SwitchInputManager*)brls::Application::getPlatform()->getInputManager())->sendRumbleRaw(160, 320, 0, 0);
#endif
}

void TestRumble::startPCM(const std::vector<std::vector<float>>& data) {
    stop();
    this->playing    = true;
    this->playThread = std::thread([this, data]() {
        for (auto& i : data) {
            if (!this->playing) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            if (!this->playing) return;
            setRumble(i[0], i[1], i[2], i[3]);
        }
        setRumble(160, 320, 0, 0);
    });
}

void TestRumble::setRumble(float lowFreq, float highFreq, float lowAmp, float highAmp) {
    if (lowFreq < 0) lowFreq = 0;
    if (lowFreq > 1250) lowFreq = 1250;
    if (highFreq < 0) highFreq = 0;
    if (highFreq > 1250) highFreq = 1250;
    if (lowAmp < 0) lowAmp = 0;
    if (lowAmp > 1) lowAmp = 1;
    if (highAmp < 0) highAmp = 0;
    if (highAmp > 1) highAmp = 1;

#ifdef __SWITCH__
    auto manager = (brls::SwitchInputManager*)brls::Application::getPlatform()->getInputManager();
    manager->sendRumbleRaw(lowFreq, highFreq, lowAmp, highAmp);
#else
    brls::Logger::debug("setRumble lf{} la{} hf{} ha{}", lowFreq, lowAmp, highFreq, highAmp);
#endif

    freq_l = lowFreq;
    freq_h = highFreq;
    amp_l  = lowAmp;
    amp_h  = highAmp;
    this->updateLabel();
}

void TestRumble::updateLabel() {
    brls::sync([this]() {
        this->label1->setText("freq: " + std::to_string(freq_l));
        this->label2->setText("amp: " + std::to_string(amp_l));
        this->label3->setText("freq: " + std::to_string(freq_h));
        this->label4->setText("amp: " + std::to_string(amp_h));

        this->progress1->setProgress(freq_l / 1252.0f);
        this->progress2->setProgress(amp_l);
        this->progress3->setProgress(freq_h / 1252.0f);
        this->progress4->setProgress(amp_h);
    });
}

TestRumble::~TestRumble() {
    brls::Logger::debug("Fragment TestRumble: delete");
    stop();
}

brls::View* TestRumble::create() { return new TestRumble(); }

brls::View* TestRumble::getDefaultFocus() { return this->btn_pcm1; }
