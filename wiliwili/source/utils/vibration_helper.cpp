//
// Created by fang on 2022/12/27.
//

#include "utils/vibration_helper.hpp"

#ifdef __SWITCH__
#include <borealis/platforms/switch/switch_input.hpp>
#include <borealis/core/application.hpp>
#endif

VibrationData VibrationHelper::coinVibrationData = {
    {160, 1000, 0, 0.87}, {160, 1000, 0, 0.98}, {160, 1000, 0, 0.97}, {160, 1000, 0, 0.96}, {160, 1000, 0, 0.88},
    {160, 1250, 0, 0.98}, {160, 1250, 0, 0.30}, {160, 1250, 0, 0.95}, {160, 1250, 0, 0.93}, {160, 1250, 0, 0.92},
    {160, 1250, 0, 0.89}, {160, 1250, 0, 0.87}, {160, 1250, 0, 0.88}, {160, 1250, 0, 0.88}, {160, 1250, 0, 0.87},
    {160, 1250, 0, 0.16}, {160, 1250, 0, 0.70}, {160, 1250, 0, 0.72}, {160, 1250, 0, 0.65}, {160, 1250, 0, 0.62},
    {160, 1250, 0, 0.66}, {160, 1250, 0, 0.08}, {160, 1250, 0, 0.05}, {160, 1250, 0, 0.57}, {160, 1250, 0, 0.51},
    {160, 1250, 0, 0.41}, {160, 1250, 0, 0.35}, {160, 1250, 0, 0.40}, {160, 1250, 0, 0.39}, {160, 1250, 0, 0.33},
    {160, 1250, 0, 0.30}, {160, 1250, 0, 0.03}, {160, 1250, 0, 0.03}, {160, 1250, 0, 0.06}, {160, 1250, 0, 0.07},
    {160, 1250, 0, 0.08}, {160, 1250, 0, 0.08}, {160, 1250, 0, 0.09}, {160, 1250, 0, 0.07}, {160, 1250, 0, 0.04},
    {160, 1250, 0, 0.08}, {160, 1250, 0, 0.01}, {160, 1250, 0, 0.03}, {160, 1250, 0, 0.01}, {160, 1250, 0, 0.01},
    {160, 1250, 0, 0.01}, {160, 1250, 0, 0.01}, {160, 1250, 0, 0.04}, {160, 1250, 0, 0.01},
};

VibrationData VibrationHelper::waitVibrationData = {
    {140, 146.5, 0.14, 0.14},
    {160, 320, 0, 0},
    {160, 320, 0, 0},
    {160, 320, 0, 0},
};

VibrationHelper::~VibrationHelper() { this->stop(); }

void VibrationHelper::stop() {
    this->playing = false;
    if (this->playThread.joinable()) this->playThread.join();
}

void VibrationHelper::startVibrate(const VibrationData& data, bool loop) {
    if (!GAMEPAD_VIBRATION) return;
    this->stop();
    this->playing = true;
#ifdef __SWITCH__
    playThread = std::thread([&data, loop, this]() {
        auto manager = (brls::SwitchInputManager*)brls::Application::getPlatform()->getInputManager();
        do {
            for (auto& i : data) {
                if (!this->playing) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                if (!this->playing) break;
                manager->sendRumbleRaw(i[0], i[1], i[2], i[3]);
            }
        } while (this->playing && loop);
        manager->sendRumbleRaw(160, 320, 0, 0);
    });
#endif
}

void VibrationHelper::playCoin() { startVibrate(coinVibrationData); }

void VibrationHelper::playWait() { startVibrate(waitVibrationData, true); }
