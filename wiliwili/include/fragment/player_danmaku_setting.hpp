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
class BiliSelectorCell;
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
    BRLS_BIND(BiliSelectorCell, cellLevel, "player/danmaku/filter/level");
    BRLS_BIND(brls::BooleanCell, cellMask, "player/danmaku/filter/mask");
    BRLS_BIND(brls::BooleanCell, cellScroll, "player/danmaku/filter/scroll");
    BRLS_BIND(brls::BooleanCell, cellTop, "player/danmaku/filter/top");
    BRLS_BIND(brls::BooleanCell, cellBottom, "player/danmaku/filter/bottom");
    BRLS_BIND(brls::BooleanCell, cellColor, "player/danmaku/filter/color");
    BRLS_BIND(brls::BooleanCell, cellAdvanced, "player/danmaku/filter/advanced");

    BRLS_BIND(BiliSelectorCell, cellArea, "player/danmaku/style/area");
    BRLS_BIND(BiliSelectorCell, cellAlpha, "player/danmaku/style/alpha");
    BRLS_BIND(BiliSelectorCell, cellSpeed, "player/danmaku/style/speed");
    BRLS_BIND(BiliSelectorCell, cellFontsize, "player/danmaku/style/fontsize");
    BRLS_BIND(BiliSelectorCell, cellLineHeight, "player/danmaku/style/lineHeight");
    BRLS_BIND(BiliSelectorCell, cellBackground, "player/danmaku/style/background");

    BRLS_BIND(BiliSelectorCell, cellRenderPerf, "player/danmaku/performance/render");

    BRLS_BIND(ButtonClose, closebtn, "button/close");
    BRLS_BIND(brls::ScrollingFrame, settings, "danmaku/settings");
    BRLS_BIND(brls::Box, cancel, "player/cancel");
};