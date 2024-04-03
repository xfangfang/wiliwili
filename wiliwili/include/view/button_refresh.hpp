//
// Created by fang on 2023/4/19.
//

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/theme.hpp>
#include <borealis/core/time.hpp>
#include <borealis/core/touch/tap_gesture.hpp>

#include "view/svg_image.hpp"

/// RefreshButton

class ButtonRefresh : public brls::Box {
public:
    ButtonRefresh() {
        // Create Refresh button
        refreshIcon = new SVGImage();
        refreshIcon->setDimensions(32, 32);
        if (brls::Application::getThemeVariant() == brls::ThemeVariant::LIGHT) {
            refreshIcon->setImageFromSVGRes("svg/bpx-svg-sprite-replay-grey.svg");
        } else {
            refreshIcon->setImageFromSVGRes("svg/bpx-svg-sprite-replay.svg");
        }
        this->setCornerRadius(8);
        this->setBackgroundColor(brls::Application::getTheme().getColor("color/grey_1"));
        this->setDimensions(55, 55);
        this->setHideClickAnimation(true);
        this->setAlignItems(brls::AlignItems::CENTER);
        this->setJustifyContent(brls::JustifyContent::CENTER);
        this->addView(refreshIcon);
        brls::Box::registerClickAction([this](...) {
            this->startRotate();
            if (this->actionListener) this->actionListener(this);
            return true;
        });
        this->addGestureRecognizer(new brls::TapGestureRecognizer(this));
    }

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override {
        Box::draw(vg, x, y, width, height, style, ctx);

        nvgBeginPath(vg);
        nvgStrokeColor(vg, nvgRGBA(40, 40, 40, 40));
        nvgStrokeWidth(vg, 1);
        nvgRoundedRect(vg, x, y, width, height, getCornerRadius());
        nvgStroke(vg);
    }

    void startRotate() {
        angle.stop();
        angle.reset(0);
        angle.addStep(NVG_PI * 4, 600, brls::EasingFunction::quadraticOut);
        angle.setTickCallback([this] { this->refreshIcon->rotate(this->angle); });
        angle.start();
    }

    void registerClickAction(const brls::ActionListener& cb) { this->actionListener = cb; }

private:
    brls::ActionListener actionListener = nullptr;
    brls::Animatable scale              = 1;
    brls::Animatable angle              = 0;
    SVGImage* refreshIcon               = nullptr;
};