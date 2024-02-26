//
// Created by fang on 2023/7/10.
//

#pragma once

#include <borealis/views/cells/cell_selector.hpp>
#include "view/grid_dropdown.hpp"

class BiliSelectorCell : public brls::SelectorCell {
public:
    BiliSelectorCell() {
        detail->setTextColor(brls::Application::getTheme()["brls/list/listItem_value_color"]);

        this->registerClickAction([this](View* view) {
            BaseDropdown::text(
                this->title->getFullText(), data, [this](int selected) { this->setSelection(selected); }, selection);
            return true;
        });
    }

    static View* create() { return new BiliSelectorCell(); }
};