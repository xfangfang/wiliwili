//
// Created by fang on 2022/12/27.
//

#pragma once

#include <vector>
#include <thread>

#include "borealis/core/singleton.hpp"

typedef std::vector<std::vector<float>> VibrationData;

class VibrationHelper : public brls::Singleton<VibrationHelper> {
public:
    VibrationHelper() = default;

    ~VibrationHelper();

    void stop();

    void startVibrate(const VibrationData& data, bool loop = false);

    void playCoin();

    void playWait();

    static VibrationData coinVibrationData;
    static VibrationData waitVibrationData;

    static inline bool GAMEPAD_VIBRATION = true;

private:
    bool playing = false;
    std::thread playThread;
};
