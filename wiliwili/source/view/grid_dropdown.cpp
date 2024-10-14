//
// Created by fang on 2022/7/9.
//

#include <borealis/views/applet_frame.hpp>
#include <borealis/core/touch/tap_gesture.hpp>

#include "view/grid_dropdown.hpp"
#include "view/svg_image.hpp"

/// EmptyDropDown

void EmptyDropdown::show(std::function<void(void)> cb, bool animate, float animationDuration) {
    if (animate) {
        content->setTranslationY(30.0f);

        showOffset.stop();
        showOffset.reset(30.0f);
        showOffset.addStep(0, animationDuration, brls::EasingFunction::quadraticOut);
        showOffset.setTickCallback([this] { this->offsetTick(); });
        showOffset.start();
    }

    Box::show(cb, animate, animationDuration);

    if (animate) {
        alpha.stop();
        alpha.reset(1);

        applet->alpha.stop();
        applet->alpha.reset(0);
        applet->alpha.addStep(1, animationDuration, brls::EasingFunction::quadraticOut);
        applet->alpha.start();
    }
}

void EmptyDropdown::hide(std::function<void(void)> cb, bool animated, float animationDuration) {
    if (animated) {
        alpha.stop();
        alpha.reset(0);

        applet->alpha.stop();
        applet->alpha.reset(1);
        applet->alpha.addStep(0, animationDuration, brls::EasingFunction::quadraticOut);
        applet->alpha.start();
    }

    Box::hide(cb, animated, animationDuration);
}

void EmptyDropdown::dismiss(std::function<void(void)> cb) { this->applet->dismiss(cb); }

bool EmptyDropdown::isTranslucent() { return true; }

brls::Box* EmptyDropdown::getContentView() { return content; }

/// GridRadioCell
GridRadioCell::GridRadioCell() { this->inflateFromXMLRes("xml/views/grid_radio_cell.xml"); }

void GridRadioCell::setSelected(bool selected) {
    brls::Theme theme = brls::Application::getTheme();

    this->selected = selected;
    this->checkbox->setVisibility(selected ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
    this->title->setTextColor(selected ? theme["brls/list/listItem_value_color"] : theme["brls/text"]);
}

bool GridRadioCell::getSelected() { return this->selected; }

RecyclingGridItem* GridRadioCell::create() { return new GridRadioCell(); }

/// DataSourceDropdown

void DataSourceDropdown::onItemSelected(RecyclingGrid* recycler, size_t index) {
    brls::Application::popActivity(brls::TransitionAnimation::FADE,
                                   [this, index]() { this->dropdown->getSelectCallback()(index); });
}

/// TextDropdown

RecyclingGridItem* TextDataSourceDropdown::cellForRow(RecyclingGrid* recycler, size_t index) {
    GridRadioCell* item = (GridRadioCell*)recycler->dequeueReusableCell("Cell");

    auto r = this->data[index];
    item->title->setText(this->data[index]);
    item->setSelected(index == dropdown->getSelected());
    return item;
}

size_t TextDataSourceDropdown::getItemCount() { return this->data.size(); }

void TextDataSourceDropdown::clearData() { this->data.clear(); }

/// BaseDropdown

BaseDropdown::BaseDropdown(const std::string& title, ValueSelectedEvent::Callback cb, size_t selected)
    : cb(std::move(cb)), selected(selected) {
    this->inflateFromXMLRes("xml/views/grid_dropdown.xml");
    this->title->setText(title);

    this->cancel->registerClickAction([this](...) {
        this->applet->dismiss();
        return true;
    });
    this->cancel->addGestureRecognizer(new brls::TapGestureRecognizer(this->cancel));

    this->setWidth(brls::Application::ORIGINAL_WINDOW_WIDTH);
}

RecyclingGrid* BaseDropdown::getRecyclingList() { return recycler; }

void BaseDropdown::setDataSource(DataSourceDropdown* dataSource) {
    // 当设置的选中项为 -1 时，表示不选中任何项，但是焦点位于第一项
    recycler->setDefaultCellFocus(selected == -1 ? 0 : selected);
    recycler->setDataSource(dataSource);

    brls::Style style = brls::Application::getStyle();

    float height = dataSource->getItemCount() * style["brls/dropdown/listItemHeight"] + header->getHeight() +
                   style["brls/dropdown/listPadding"]    // top
                   + style["brls/dropdown/listPadding"]  // bottom
        ;

    content->setHeight(fmin(height, brls::Application::contentHeight * 0.73f));
}

brls::View* BaseDropdown::getParentNavigationDecision(View* from, View* newFocus, brls::FocusDirection direction) {
    View* result = Box::getParentNavigationDecision(from, newFocus, direction);

    auto* cell = dynamic_cast<RecyclingGridItem*>(result);
    if (cell && cell != from) {
        cellFocusDidChangeEvent.fire(cell);
    }

    return result;
}

brls::Event<RecyclingGridItem*>* BaseDropdown::getCellFocusDidChangeEvent() { return &cellFocusDidChangeEvent; }

size_t BaseDropdown::getSelected() const { return this->selected; }

ValueSelectedEvent::Callback BaseDropdown::getSelectCallback() { return this->cb; }

BaseDropdown* BaseDropdown::text(const std::string& title, const std::vector<std::string>& values,
                                 ValueSelectedEvent::Callback cb, size_t selected, const std::string& hint) {
    auto* dropdown = new BaseDropdown(title, std::move(cb), selected);
    dropdown->getRecyclingList()->registerCell("Cell", []() {
        auto* cell = new GridRadioCell();
        cell->setHeight(brls::Application::getStyle()["brls/dropdown/listItemHeight"]);
        cell->title->setFontSize(brls::Application::getStyle()["brls/dropdown/listItemTextSize"]);
        return cell;
    });
    if (!hint.empty()) {
        auto box = new brls::Box();
        box->setMargins(20, 10, 10, 30);
        box->setAlignItems(brls::AlignItems::CENTER);
        auto icon = new SVGImage();
        icon->setDimensions(12, 13);
        icon->setMarginRight(10);
        icon->setImageFromSVGRes("svg/ico-sprite-info.svg");
        auto text = new brls::Label();
        text->setFontSize(18);
        text->setTextColor(brls::Application::getTheme().getColor("font/grey"));
        text->setText(hint);
        box->addView(icon);
        box->addView(text);
        dropdown->getContentView()->addView(box);
    }
    dropdown->setDataSource(new TextDataSourceDropdown(values, dropdown));
    brls::Application::pushActivity(new brls::Activity(dropdown));
    return dropdown;
}