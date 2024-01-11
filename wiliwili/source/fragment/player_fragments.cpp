//
// Created by fang on 2023/1/1.
//

#include "fragment/player_fragments.hpp"

/// PlayerTabCell

PlayerTabCell::PlayerTabCell() {
    this->inflateFromXMLRes("xml/views/season_item_cell.xml");
    this->setHideHighlightBackground(true);
}

void PlayerTabCell::setSelected(bool value) {
    this->selected = value;
    if (value) {
        this->title->setMarginLeft(40);
        this->setBackgroundColor(brls::Application::getTheme().getColor("color/pink_1"));
    } else {
        this->title->setMarginLeft(16);
        this->setBackgroundColor(RGBA(0, 0, 0, 0));
    }
}

bool PlayerTabCell::getSelected() { return this->selected; }

void PlayerTabCell::setBadge(std::string value, std::string color) {
    if (value.empty()) {
        this->badgeBox->setVisibility(brls::Visibility::GONE);
        return;
    }

    unsigned char r, g, b;
    int result = sscanf(color.c_str(), "#%02hhx%02hhx%02hhx", &r, &g, &b);
    if (result == 3) {
        this->badge->setText(value);
        this->badgeBox->setVisibility(brls::Visibility::VISIBLE);
        this->badgeBox->setBackgroundColor(nvgRGB(r, g, b));
    } else {
        this->badgeBox->setVisibility(brls::Visibility::GONE);
    }
}

void PlayerTabCell::setBadge(std::string value, NVGcolor color, NVGcolor textColor) {
    if (value.empty()) {
        this->badgeBox->setVisibility(brls::Visibility::GONE);
        return;
    }

    this->badge->setText(value);
    this->badge->setTextColor(textColor);
    this->badgeBox->setVisibility(brls::Visibility::VISIBLE);
    this->badgeBox->setBackgroundColor(color);
}

RecyclingGridItem* PlayerTabCell::create() { return new PlayerTabCell(); }
void PlayerTabCell::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                         brls::FrameContext* ctx) {
    Box::draw(vg, x, y, width, height, style, ctx);
    if (this->selected) {
        // h1-3 周期 666ms，最大幅度 666*0.015 ≈ 10
        int h1 = ((brls::getCPUTimeUsec() >> 10) % 666) * 0.015;
        int h2 = (h1 + 3) % 10;
        int h3 = (h1 + 7) % 10;
        if (h1 > 5) h1 = 10 - h1;
        if (h2 > 5) h2 = 10 - h2;
        if (h3 > 5) h3 = 10 - h3;

        float base_y = y + height / 2 - 2;
        nvgBeginPath(vg);
        nvgFillColor(vg, a(ctx->theme.getColor("color/bilibili")));
        nvgRect(vg, x + 20, base_y - h1, 2, h1 + h1 + 4);
        nvgRect(vg, x + 25, base_y - h2, 2, h2 + h2 + 4);
        nvgRect(vg, x + 30, base_y - h3, 2, h3 + h3 + 4);
        nvgFill(vg);
    }
}

/// PlayerTabHeader

PlayerTabHeader::PlayerTabHeader() {
    this->inflateFromXMLRes("xml/views/season_header_cell.xml");
    this->setFocusable(false);
}

RecyclingGridItem* PlayerTabHeader::create() { return new PlayerTabHeader(); }