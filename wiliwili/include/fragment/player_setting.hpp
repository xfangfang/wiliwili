//
// Created by fang on 2023/3/4.
//

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

class ButtonClose;
class BiliSelectorCell;
enum class SettingItem;
namespace brls {
class ScrollingFrame;
class Header;
class BooleanCell;
class DetailCell;
class SliderCell;
class RadioCell;
class InputNumericCell;
};  // namespace brls

class PlayerSetting : public brls::Box {
public:
    PlayerSetting();

    bool isTranslucent() override;

    View* getDefaultFocus() override;

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

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

    void hideSkipOpeningCreditsSetting();

    void setBangumiCustomSetting(const std::string& title, unsigned int seasonId);

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
    BRLS_BIND(brls::BooleanCell, btnAlwaysOnTop, "setting/alwaysontop");
    BRLS_BIND(brls::BooleanCell, btnProgress, "setting/video/progress");
    BRLS_BIND(brls::BooleanCell, btnHistory, "setting/history/log");
    BRLS_BIND(brls::DetailCell, btnPlayStrategy, "setting/play/strategy");
    BRLS_BIND(brls::BooleanCell, btnExitFullscreen, "setting/auto/exit");
    BRLS_BIND(brls::BooleanCell, btnMirror, "setting/video/mirror");
    BRLS_BIND(BiliSelectorCell, btnAspect, "setting/video/aspect");
    BRLS_BIND(brls::BooleanCell, btnHighlight, "setting/video/highlight");
    BRLS_BIND(brls::DetailCell, btnSleep, "setting/sleep");
    BRLS_BIND(brls::BooleanCell, btnSkip, "setting/auto/skip");

    // equalizer setting
    BRLS_BIND(brls::RadioCell, btnEqualizerReset, "setting/equalizer/reset");
    BRLS_BIND(brls::SliderCell, btnEqualizerBrightness, "setting/equalizer/brightness");
    BRLS_BIND(brls::SliderCell, btnEqualizerContrast, "setting/equalizer/contrast");
    BRLS_BIND(brls::SliderCell, btnEqualizerSaturation, "setting/equalizer/saturation");
    BRLS_BIND(brls::SliderCell, btnEqualizerGamma, "setting/equalizer/gamma");
    BRLS_BIND(brls::SliderCell, btnEqualizerHue, "setting/equalizer/hue");

    // bangumi custom setting
    BRLS_BIND(brls::Header, bangumiHeader, "setting/video/custom/header");
    BRLS_BIND(brls::Box, bangumiBox, "setting/video/custom/box");
    BRLS_BIND(BiliSelectorCell, btnCustomAspect, "setting/video/custom/aspect");
    BRLS_BIND(brls::BooleanCell, btnClip, "setting/video/custom/clip");
    BRLS_BIND(brls::InputNumericCell, btnClipStart, "setting/video/custom/clip/start");
    BRLS_BIND(brls::InputNumericCell, btnClipEnd, "setting/video/custom/clip/end");

    unsigned int seasonId{};

    // 更新倒计时
    void updateCountdown(size_t now);

    void setupEqualizerSetting(brls::SliderCell* cell, const std::string& title, SettingItem item, int initValue);

    void registerHideBackground(brls::View* view);
};