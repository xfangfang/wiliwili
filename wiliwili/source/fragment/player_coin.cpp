//
// Created by fang on 2022/12/26.
//

#include <borealis/core/i18n.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>
#include <borealis/views/label.hpp>

#include "fragment/player_coin.hpp"
#include "view/animation_image.hpp"
#include "view/check_box.hpp"
#include "utils/vibration_helper.hpp"
#include "utils/string_helper.hpp"
#include "bilibili.h"

using namespace brls::literals;

PlayerCoin::PlayerCoin() {
#if defined(__PSV__) || defined(PS4)
    this->inflateFromXMLRes("xml/fragment/player_coin_psv.xml");
#else
    this->inflateFromXMLRes("xml/fragment/player_coin.xml");
#endif
    brls::Logger::debug("Fragment PlayerCoin: create");

    this->img1->registerClickAction([this](brls::View* v) -> bool {
        coinEvent.fire(1);
        VibrationHelper::instance().playCoin();
        this->dismiss();
        return true;
    });

    this->img2->registerClickAction([this](brls::View* v) -> bool {
        coinEvent.fire(2);
        VibrationHelper::instance().playCoin();
        this->dismiss();
        return true;
    });

    this->registerAction(
        "", brls::BUTTON_Y,
        [this](...) {
            this->checkBox->setChecked(!this->checkBox->getChecked());
            return true;
        },
        true);

    this->likeBox->registerClickAction([this](...) {
        this->checkBox->setChecked(!this->checkBox->getChecked());
        return true;
    });

    // support touch
    this->likeBox->addGestureRecognizer(new brls::TapGestureRecognizer(this->likeBox));
    this->img1->addGestureRecognizer(new brls::TapGestureRecognizer(this->img1));
    this->img2->addGestureRecognizer(new brls::TapGestureRecognizer(this->img2));

    this->getCoinExp();
}

void PlayerCoin::hideTwoCoin(bool value) {
    if (value) {
        this->img2->setVisibility(brls::Visibility::GONE);
    } else {
        this->img2->setVisibility(brls::Visibility::VISIBLE);
    }
}

bool PlayerCoin::likeAtTheSameTime() { return this->checkBox->getChecked(); }

brls::Event<int>* PlayerCoin::getSelectEvent() { return &this->coinEvent; }

void PlayerCoin::getCoinExp() {
    ASYNC_RETAIN
    BILI::get_coin_exp(
        [ASYNC_TOKEN](int value) {
            brls::sync([ASYNC_TOKEN, value]() {
                ASYNC_RELEASE
                this->exp = value;
                this->updateHintLabel();
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("{}", error);
        });
}

void PlayerCoin::updateHintLabel() {
    if (exp < 0) return;

    if (exp >= 50) {
        labelHint->setText("wiliwili/player/coin/exp_full"_i18n);
    } else {
        labelHint->setText(wiliwili::format("wiliwili/player/coin/exp"_i18n, this->coin * 10, exp));
    }
}

PlayerCoin::~PlayerCoin() { brls::Logger::debug("Fragment PlayerCoin: delete"); }

brls::View* PlayerCoin::create() { return new PlayerCoin(); }

brls::View* PlayerCoin::getDefaultFocus() {
    if (this->img2->getVisibility() == brls::Visibility::GONE) return (brls::View*)this->img1.getView();
    return (brls::View*)this->img2.getView();
}

void PlayerCoin::onChildFocusGained(brls::View* directChild, brls::View* focusedView) {
    Box::onChildFocusGained(directChild, focusedView);
    if (focusedView == this->img2.getView()) {
        this->labelNum->setText(wiliwili::format("wiliwili/player/coin/pcs"_i18n, 2));
        this->coin = 2;
    } else if (focusedView == this->img1.getView()) {
        this->labelNum->setText(wiliwili::format("wiliwili/player/coin/pcs"_i18n, 1));
        this->coin = 1;
    } else {
        return;
    }
    this->updateHintLabel();
}