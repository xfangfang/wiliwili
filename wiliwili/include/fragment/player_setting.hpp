//
// Created by fang on 2023/3/4.
//

#pragma once

#include <borealis.hpp>

class ButtonClose;
class SelectorCell;
namespace brls {
class ScrollingFrame;
};

class PlayerSetting : public brls::Box {
public:
    PlayerSetting();

    bool isTranslucent() override;

    View* getDefaultFocus() override;

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    ~PlayerSetting() override;

    static View* create();

    void setupCustomShaders();

    void setupCommonSetting();

    void setupSubtitle();

    void hideHistoryCell();

    void hideVideoRelatedCells();

    void hideSubtitleCells();

    void hideBottomLineCells();

    void hideHighlightLineCells();

private:
    BRLS_BIND(ButtonClose, closebtn, "button/close");
    BRLS_BIND(brls::ScrollingFrame, settings, "player/settings");
    BRLS_BIND(brls::Box, cancel, "player/cancel");

    // subtitle setting
    BRLS_BIND(brls::Header, subtitleHeader, "setting/video/subtitle/header");
    BRLS_BIND(brls::BooleanCell, btnSubtitle, "setting/video/subtitle");
    BRLS_BIND(brls::Box, subtitleBox, "setting/video/subtitle/box");

    // shader setting
    BRLS_BIND(brls::Header, shaderHeader, "setting/shaders/header");
    BRLS_BIND(brls::Box, shaderBox, "setting/shaders/box");

    // common setting
    BRLS_BIND(brls::BooleanCell, btnFullscreen, "setting/fullscreen");
    BRLS_BIND(brls::BooleanCell, btnProgress, "setting/video/progress");
    BRLS_BIND(brls::BooleanCell, btnHistory, "setting/history/log");
    BRLS_BIND(brls::BooleanCell, btnAutoNextPart, "setting/auto/nextPart");
    BRLS_BIND(brls::BooleanCell, btnAutoNextRcmd, "setting/auto/nextRcmd");
    BRLS_BIND(brls::BooleanCell, btnExitFullscreen, "setting/auto/exit");
    BRLS_BIND(brls::BooleanCell, btnMirror, "setting/video/mirror");
    BRLS_BIND(SelectorCell, btnAspect, "setting/video/aspect");
    BRLS_BIND(brls::BooleanCell, btnHighlight, "setting/video/highlight");
    BRLS_BIND(brls::DetailCell, btnSleep, "setting/sleep");

    // 获取需要现实的倒计时关闭字符串
    static inline std::string getCountdown(size_t now);
};