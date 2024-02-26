//
// Created by fang on 2022/12/26.
//

// register this fragment in main.cpp
//#include "fragment/player_coin.hpp"
//    brls::Application::registerXMLView("PlayerCoin", PlayerCoin::create);
// <brls:View xml=@res/xml/fragment/player_coin.xml

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

namespace brls {
class Label;
}
class AnimationImage;
class BiliCheckBox;

class PlayerCoin : public brls::Box {
public:
    PlayerCoin();

    ~PlayerCoin() override;

    void hideTwoCoin(bool value = true);

    brls::Event<int>* getSelectEvent();

    bool likeAtTheSameTime();

    void onChildFocusGained(View* directChild, View* focusedView) override;

    View* getDefaultFocus() override;

    /// 获取投币经验值
    void getCoinExp();

    void updateHintLabel();

    static View* create();

private:
    BRLS_BIND(brls::Label, labelNum, "coin/num");
    BRLS_BIND(brls::Label, labelHint, "coin/hint");
    BRLS_BIND(AnimationImage, img1, "coin/img/1");
    BRLS_BIND(AnimationImage, img2, "coin/img/2");
    BRLS_BIND(BiliCheckBox, checkBox, "coin/checkbox");
    BRLS_BIND(brls::Box, likeBox, "coin/like");

    int coin = 2;
    int exp  = -1;

    brls::Event<int> coinEvent;
};