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

#include <borealis.hpp>

class CustomLayoutTab : public brls::AbsoluteLayout
{
  public:
    CustomLayoutTab();

    brls::View* getDefaultFocus() override;
    brls::View* getNextFocus(brls::FocusDirection direction, brls::View* currentView) override;
    void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash);

  private:
    brls::NavigationMap navigationMap;

    brls::Button* firstButton;
    brls::Button* secondButton;
    brls::Button* thirdButton;
};
