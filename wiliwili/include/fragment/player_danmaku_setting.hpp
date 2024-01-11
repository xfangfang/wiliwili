//
// Created by fang on 2023/1/10.
//

// register this fragment in main.cpp
//#include "fragment/player_danmaku_setting.hpp"
//    brls::Application::registerXMLView("PlayerDanmakuSetting", PlayerDanmakuSetting::create);
// <brls:View xml=@res/xml/fragment/player_danmaku_setting.xml

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

class ButtonClose;
class SelectorCell;
namespace brls {
class ScrollingFrame;
class BooleanCell;
};  // namespace brls
class PlayerDanmakuSetting : public brls::Box {
public:
    PlayerDanmakuSetting();

    bool isTranslucent() override;

    View* getDefaultFocus() override;

    ~PlayerDanmakuSetting() override;

    static View* create();

private:
    BRLS_BIND(SelectorCell, cellLevel, "player/danmaku/filter/level");
    BRLS_BIND(brls::BooleanCell, cellMask, "player/danmaku/filter/mask");
    BRLS_BIND(brls::BooleanCell, cellScroll, "player/danmaku/filter/scroll");
    BRLS_BIND(brls::BooleanCell, cellTop, "player/danmaku/filter/top");
    BRLS_BIND(brls::BooleanCell, cellBottom, "player/danmaku/filter/bottom");
    BRLS_BIND(brls::BooleanCell, cellColor, "player/danmaku/filter/color");
    BRLS_BIND(brls::BooleanCell, cellAdvanced, "player/danmaku/filter/advanced");

    BRLS_BIND(SelectorCell, cellArea, "player/danmaku/style/area");
    BRLS_BIND(SelectorCell, cellAlpha, "player/danmaku/style/alpha");
    BRLS_BIND(SelectorCell, cellSpeed, "player/danmaku/style/speed");
    BRLS_BIND(SelectorCell, cellFontsize, "player/danmaku/style/fontsize");
    BRLS_BIND(SelectorCell, cellLineHeight, "player/danmaku/style/lineHeight");
    BRLS_BIND(SelectorCell, cellBackground, "player/danmaku/style/background");

    BRLS_BIND(SelectorCell, cellRenderPerf, "player/danmaku/performance/render");

    BRLS_BIND(ButtonClose, closebtn, "button/close");
    BRLS_BIND(brls::ScrollingFrame, settings, "danmaku/settings");
    BRLS_BIND(brls::Box, cancel, "player/cancel");
};