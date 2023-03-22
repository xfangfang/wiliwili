//
// Created by fang on 2023/3/4.
//

#pragma once

#include <borealis.hpp>

class ButtonClose;
namespace brls {
class ScrollingFrame;
};

class PlayerSetting : public brls::Box {
public:
    PlayerSetting();

    bool isTranslucent() override;

    View* getDefaultFocus() override;

    ~PlayerSetting() override;

    static View* create();

    void setupCustomShaders();

    void setupCommonSetting();

    void setupSubtitle();

private:
    BRLS_BIND(ButtonClose, closebtn, "button/close");
    BRLS_BIND(brls::ScrollingFrame, settings, "player/settings");

    // subtitle setting
    BRLS_BIND(brls::Header, subtitleHeader, "setting/video/subtitle/header");
    BRLS_BIND(brls::BooleanCell, btnSubtitle, "setting/video/subtitle");
    BRLS_BIND(brls::Box, subtitleBox, "setting/video/subtitle/box");

    // shader setting
    BRLS_BIND(brls::Header, shaderHeader, "setting/shaders/header");
    BRLS_BIND(brls::Box, shaderBox, "setting/shaders/box");

    // common setting
    BRLS_BIND(brls::BooleanCell, btnProgress, "setting/video/progress");
    BRLS_BIND(brls::BooleanCell, btnHistory, "setting/history/log");
    BRLS_BIND(brls::BooleanCell, btnAutoNextPart, "setting/auto/nextPart");
    BRLS_BIND(brls::BooleanCell, btnAutoNextRcmd, "setting/auto/nextRcmd");
    BRLS_BIND(brls::BooleanCell, btnExitFullscreen, "setting/auto/exit");
};