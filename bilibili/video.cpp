/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2020  natinusala

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "video.hpp"

using namespace brls::i18n::literals;

CustomLayoutTab::CustomLayoutTab()
{
    // Create views
    this->firstButton = new brls::Button(brls::ButtonStyle::REGULAR);
    this->firstButton->setLabel("custom_layout/first_button"_i18n);
    this->addView(this->firstButton);

    this->secondButton = new brls::Button(brls::ButtonStyle::REGULAR);
    this->secondButton->setLabel("custom_layout/second_button"_i18n);
    this->addView(this->secondButton);

    this->thirdButton = new brls::Button(brls::ButtonStyle::REGULAR);
    this->thirdButton->setLabel("custom_layout/third_button"_i18n);
    this->addView(this->thirdButton);

    // Populate custom navigation map
    this->navigationMap.add(
        this->firstButton,
        brls::FocusDirection::RIGHT,
        this->secondButton);

    this->navigationMap.add(
        this->secondButton,
        brls::FocusDirection::LEFT,
        this->firstButton);

    this->navigationMap.add(
        this->secondButton,
        brls::FocusDirection::DOWN,
        this->thirdButton);

    this->navigationMap.add(
        this->thirdButton,
        brls::FocusDirection::UP,
        this->secondButton);
}

#define BUTTON_WIDTH 300
#define BUTTON_HEIGHT 50
#define PADDING 75

void CustomLayoutTab::layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash)
{
    int x = this->getX();
    int y = this->getY();

    // Fully custom layout
    this->firstButton->setBoundaries(
        x + PADDING,
        y + PADDING,
        BUTTON_WIDTH,
        BUTTON_HEIGHT);

    this->secondButton->setBoundaries(
        x + PADDING * 2 + BUTTON_WIDTH,
        y + PADDING,
        BUTTON_WIDTH,
        BUTTON_HEIGHT);

    this->thirdButton->setBoundaries(
        x + PADDING * 2 + BUTTON_WIDTH,
        y + PADDING * 2 + BUTTON_HEIGHT,
        BUTTON_WIDTH,
        BUTTON_HEIGHT);
}

brls::View* CustomLayoutTab::getDefaultFocus()
{
    return this->firstButton->getDefaultFocus();
}

brls::View* CustomLayoutTab::getNextFocus(brls::FocusDirection direction, brls::View* currentView)
{
    return this->navigationMap.getNextFocus(direction, currentView);
}
