//
// Created by fang on 2022/12/28.
//

#include "view/check_box.hpp"

BiliCheckBox::BiliCheckBox() {
    this->registerBoolXMLAttribute("checked", [this](bool value) { this->setChecked(value); });
}

void BiliCheckBox::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                    brls::FrameContext* ctx) {
    float corner_radius = getCornerRadius();
    float radius        = std::fmin(width, height) / 2;
    int thickness       = roundf(radius * 0.20f);
    if (!checked) {
        nvgBeginPath(vg);
        nvgStrokeColor(vg, a(ctx->theme["brls/text"]));
        nvgStrokeWidth(vg, thickness);
        nvgRoundedRect(vg, x, y, width, height, corner_radius);
        nvgStroke(vg);
        return;
    }

    float centerX = x + width / 2;
    float centerY = y + height / 2;

    // Background
    nvgFillColor(vg, a(ctx->theme["color/bilibili"]));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, width, height, corner_radius);
    nvgFill(vg);

    // Check mark
    nvgFillColor(vg, a(nvgRGB(255, 255, 255)));

    // Long stroke
    nvgSave(vg);
    nvgTranslate(vg, centerX, centerY);
    nvgRotate(vg, -NVG_PI / 4.0f);

    nvgBeginPath(vg);
    nvgRect(vg, -(radius * 0.55f), 0, radius * 1.3f, thickness);
    nvgFill(vg);
    nvgRestore(vg);

    // Short stroke
    nvgSave(vg);
    nvgTranslate(vg, centerX - (radius * 0.65f), centerY);
    nvgRotate(vg, NVG_PI / 4.0f);

    nvgBeginPath(vg);
    nvgRect(vg, 0, -(thickness / 2), radius * 0.53f, thickness);
    nvgFill(vg);

    nvgRestore(vg);
}

void BiliCheckBox::setChecked(bool value) { this->checked = value; }

bool BiliCheckBox::getChecked() { return this->checked; }

brls::View* BiliCheckBox::create() { return new BiliCheckBox(); }