//
// Created by fang on 2023/1/10.
//

// register this fragment in main.cpp
//#include "fragment/player_danmaku_setting.hpp"
//    brls::Application::registerXMLView("PlayerDanmakuSetting", PlayerDanmakuSetting::create);
// <brls:View xml=@res/xml/fragment/player_danmaku_setting.xml

#pragma once

#include <borealis.hpp>

class ButtonClose;
namespace brls {
class ScrollingFrame;
};
class PlayerDanmakuSetting : public brls::Box {
public:
    PlayerDanmakuSetting();

    bool isTranslucent() override;

    View* getDefaultFocus() override;

    ~PlayerDanmakuSetting() override;

    static View* create();

private:
    BRLS_BIND(brls::SelectorCell, cellLevel, "player/danmaku/filter/level");
    BRLS_BIND(brls::BooleanCell, cellScroll, "player/danmaku/filter/scroll");
    BRLS_BIND(brls::BooleanCell, cellTop, "player/danmaku/filter/top");
    BRLS_BIND(brls::BooleanCell, cellBottom, "player/danmaku/filter/bottom");
    BRLS_BIND(brls::BooleanCell, cellColor, "player/danmaku/filter/color");

    BRLS_BIND(brls::SelectorCell, cellArea, "player/danmaku/style/area");
    BRLS_BIND(brls::SelectorCell, cellAlpha, "player/danmaku/style/alpha");
    BRLS_BIND(brls::SelectorCell, cellSpeed, "player/danmaku/style/speed");
    BRLS_BIND(brls::SelectorCell, cellFontsize,
              "player/danmaku/style/fontsize");
    BRLS_BIND(brls::SelectorCell, cellLineHeight,
              "player/danmaku/style/lineHeight");

    BRLS_BIND(ButtonClose, closebtn, "button/close");
    BRLS_BIND(brls::ScrollingFrame, settings, "danmaku/settings");
};