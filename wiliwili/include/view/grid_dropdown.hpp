//
// Created by fang on 2022/7/9.
//

// register this view in main.cpp
//#include "view/grid_dropdown.hpp"
//    brls::Application::registerXMLView("GridDropdown", GridDropdown::create);
// <brls:View xml=@res/xml/views/grid_dropdown.xml

#pragma once

#include <cmath>
#include <borealis.hpp>
#include <borealis/views/cells/cell_radio.hpp>
#include "view/recycling_grid.hpp"

typedef brls::Event<int> ValueSelectedEvent;

class GridDropdown;

class GridRadioCell : public RecyclingGridItem {
public:
    GridRadioCell() {
        this->inflateFromXMLRes("xml/views/grid_radio_cell.xml");
    }

    void setSelected(bool selected) {
        brls::Theme theme = brls::Application::getTheme();

        this->selected = selected;
        this->checkbox->setVisibility(selected ? brls::Visibility::VISIBLE
                                               : brls::Visibility::GONE);
        this->title->setTextColor(selected
                                      ? theme["brls/list/listItem_value_color"]
                                      : theme["brls/text"]);
    }

    bool getSelected() { return this->selected; }

    BRLS_BIND(brls::Label, title, "brls/rediocell/title");
    BRLS_BIND(brls::CheckBox, checkbox, "brls/rediocell/checkbox");

    static RecyclingGridItem* create() { return new GridRadioCell(); }

private:
    bool selected = false;
};

class DataSourceDropdown : public RecyclingGridDataSource {
public:
    DataSourceDropdown(std::vector<std::string> result, GridDropdown* view)
        : data(result), dropdown(view) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                  size_t index) override;

    size_t getItemCount() override;

    void onItemSelected(RecyclingGrid* recycler, size_t index) override;

    void clearData() override{
        data.clear();
    }

private:
    std::vector<std::string> data;
    GridDropdown* dropdown;
};

class GridDropdown : public brls::Box {
public:
    GridDropdown(std::string title, std::vector<std::string> values,
                 ValueSelectedEvent::Callback cb, int selected = 0)
        : values(values), cb(cb), selected(selected) {
        this->inflateFromXMLRes("xml/views/grid_dropdown.xml");
        this->title->setText(title);

        recycler->registerCell("Cell", []() {
            GridRadioCell* cell = new GridRadioCell();
            cell->setHeight(
                brls::Application::getStyle()["brls/dropdown/listItemHeight"]);
            cell->title->setFontSize(brls::Application::getStyle()
                                         ["brls/dropdown/listItemTextSize"]);
            return cell;
        });

        DataSourceDropdown* dataSource = new DataSourceDropdown(values, this);
        recycler->setDefaultCellFocus(selected);
        recycler->setDataSource(dataSource);

        brls::Style style = brls::Application::getStyle();

        float height =
            dataSource->getItemCount() * style["brls/dropdown/listItemHeight"] +
            header->getHeight() + style["brls/dropdown/listPadding"]  // top
            + style["brls/dropdown/listPadding"]                      // bottom
            ;

        content->setHeight(
            fmin(height, brls::Application::contentHeight * 0.73f));
    }

    void show(std::function<void(void)> cb, bool animate,
              float animationDuration) override {
        if (animate) {
            content->setTranslationY(30.0f);

            showOffset.stop();
            showOffset.reset(30.0f);
            showOffset.addStep(0, animationDuration,
                               brls::EasingFunction::quadraticOut);
            showOffset.setTickCallback([this] { this->offsetTick(); });
            showOffset.start();
        }

        Box::show(cb, animate, animationDuration);

        if (animate) {
            alpha.stop();
            alpha.reset(1);

            applet->alpha.stop();
            applet->alpha.reset(0);
            applet->alpha.addStep(1, animationDuration,
                                  brls::EasingFunction::quadraticOut);
            applet->alpha.start();
        }
    }
    void hide(std::function<void(void)> cb, bool animated,
              float animationDuration) override {
        if (animated) {
            alpha.stop();
            alpha.reset(0);

            applet->alpha.stop();
            applet->alpha.reset(1);
            applet->alpha.addStep(0, animationDuration,
                                  brls::EasingFunction::quadraticOut);
            applet->alpha.start();
        }

        Box::hide(cb, animated, animationDuration);
    }

    virtual View* getParentNavigationDecision(
        View* from, View* newFocus, brls::FocusDirection direction) override {
        View* result =
            Box::getParentNavigationDecision(from, newFocus, direction);

        RecyclingGridItem* cell = dynamic_cast<RecyclingGridItem*>(result);
        if (cell && cell != from) {
            cellFocusDidChangeEvent.fire(cell);
        }

        return result;
    }

    brls::Event<RecyclingGridItem*>* getCellFocusDidChangeEvent() {
        return &cellFocusDidChangeEvent;
    }

    bool isTranslucent() override {
        return true || brls::View::isTranslucent();
    }

    size_t getSelected() { return this->selected; }

    ValueSelectedEvent::Callback getSelectCallback() { return this->cb; }

protected:
    float getShowAnimationDuration(
        brls::TransitionAnimation animation) override {
        return View::getShowAnimationDuration(animation) / 2;
    }

private:
    BRLS_BIND(RecyclingGrid, recycler, "grid_dropdown/recycler");
    BRLS_BIND(brls::Box, header, "grid_dropdown/header");
    BRLS_BIND(brls::Label, title, "grid_dropdown/title_label");
    BRLS_BIND(brls::Box, content, "grid_dropdown/content");
    BRLS_BIND(brls::AppletFrame, applet, "grid_dropdown/applet");

    std::vector<std::string> values;
    ValueSelectedEvent::Callback cb;
    size_t selected;
    brls::Animatable showOffset = 0;
    brls::Event<RecyclingGridItem*> cellFocusDidChangeEvent;

    void offsetTick() { content->setTranslationY(showOffset); }
};