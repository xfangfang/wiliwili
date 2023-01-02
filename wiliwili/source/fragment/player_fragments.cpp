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
        this->setBackgroundColor(
            brls::Application::getTheme().getColor("color/pink_1"));
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

RecyclingGridItem* PlayerTabCell::create() { return new PlayerTabCell(); }
void PlayerTabCell::draw(NVGcontext* vg, float x, float y, float width,
                         float height, brls::Style style,
                         brls::FrameContext* ctx) {
    Box::draw(vg, x, y, width, height, style, ctx);
    if (this->selected) {
        brls::Time t = brls::getCPUTimeUsec() / 1000;
        // 半高
        int h1 = 20 * fabs((t % 700) / 700.0f - 0.5) + 4;
        int h2 = 20 * fabs(((t + 300) % 700) / 700.0f - 0.5) + 4;
        int h3 = 20 * fabs(((t + 500) % 700) / 700.0f - 0.5) + 4;
        nvgBeginPath(vg);
        nvgFillColor(vg, a(ctx->theme.getColor("color/bilibili")));
        nvgRect(vg, x + 20, y + (height - h1) / 2, 2, h1);
        nvgRect(vg, x + 25, y + (height - h2) / 2, 2, h2);
        nvgRect(vg, x + 30, y + (height - h3) / 2, 2, h3);
        nvgFill(vg);
    }
}

/// PlayerTabHeader

PlayerTabHeader::PlayerTabHeader() {
    this->inflateFromXMLRes("xml/views/season_header_cell.xml");
}

RecyclingGridItem* PlayerTabHeader::create() { return new PlayerTabHeader(); }