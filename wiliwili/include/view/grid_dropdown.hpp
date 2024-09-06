//
// Created by fang on 2022/7/9.
//

// register this view in main.cpp
//#include "view/grid_dropdown.hpp"
//    brls::Application::registerXMLView("GridDropdown", GridDropdown::create);
// <brls:View xml=@res/xml/views/grid_dropdown.xml

#pragma once

#include <cstdlib>
#include <cmath>
#include <utility>
#include <borealis/views/cells/cell_radio.hpp>

#include "view/recycling_grid.hpp"

typedef brls::Event<int> ValueSelectedEvent;

class EmptyDropdown : public brls::Box {
public:
    void show(std::function<void(void)> cb, bool animate, float animationDuration) override;

    void hide(std::function<void(void)> cb, bool animated, float animationDuration) override;

    void dismiss(std::function<void(void)> cb = [] {}) override;

    bool isTranslucent() override;

    brls::Box* getContentView();

protected:
    BRLS_BIND(brls::Box, header, "grid_dropdown/header");
    BRLS_BIND(brls::Label, title, "grid_dropdown/title_label");
    BRLS_BIND(brls::Box, content, "grid_dropdown/content");
    BRLS_BIND(brls::Box, cancel, "grid_dropdown/cancel");
    BRLS_BIND(brls::AppletFrame, applet, "grid_dropdown/applet");

    brls::Animatable showOffset = 0;
    void offsetTick() { content->setTranslationY(showOffset); }
};

//class GridDropdown;
class BaseDropdown;

class GridRadioCell : public RecyclingGridItem {
public:
    GridRadioCell();

    void setSelected(bool selected);

    bool getSelected();

    BRLS_BIND(brls::Label, title, "brls/rediocell/title");
    BRLS_BIND(brls::CheckBox, checkbox, "brls/rediocell/checkbox");

    static RecyclingGridItem* create();

private:
    bool selected = false;
};

class DataSourceDropdown : public RecyclingGridDataSource {
public:
    DataSourceDropdown(BaseDropdown* view) : dropdown(view) {}

    void onItemSelected(RecyclingGrid* recycler, size_t index) override;

protected:
    BaseDropdown* dropdown;
};

class TextDataSourceDropdown : public DataSourceDropdown {
public:
    TextDataSourceDropdown(std::vector<std::string> result, BaseDropdown* view)
        : DataSourceDropdown(view), data(std::move(result)) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override;

    size_t getItemCount() override;

    void clearData() override;

private:
    std::vector<std::string> data;
};


template <typename T>
class CommonDataSourceDropdown : public DataSourceDropdown {
public:
    typedef std::function<RecyclingGridItem*(RecyclingGrid*, T&)> CommonCellForRowCallback;

    CommonDataSourceDropdown(std::vector<T> result, BaseDropdown* view,
                             CommonCellForRowCallback cb)
        : DataSourceDropdown(view), data(std::move(result)), cellForRowFunc(cb) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        if (cellForRowFunc) return cellForRowFunc(recycler, data[index]);
        return nullptr;
    }

    size_t getItemCount() override { return data.size(); }

    void clearData() override { data.clear(); }

private:
    std::vector<T> data;
    CommonCellForRowCallback cellForRowFunc = nullptr;
};


/**
 * 带有进入退出动画的菜单，自带列表，默认提供了文本列表，也可以自定义列表内容
 */
class BaseDropdown : public EmptyDropdown {
public:
    BaseDropdown(const std::string& title, ValueSelectedEvent::Callback cb, size_t selected = 0);

    RecyclingGrid* getRecyclingList();

    void setDataSource(DataSourceDropdown* dataSource);

    virtual View* getParentNavigationDecision(View* from, View* newFocus, brls::FocusDirection direction) override;

    brls::Event<RecyclingGridItem*>* getCellFocusDidChangeEvent();

    size_t getSelected() const;

    ValueSelectedEvent::Callback getSelectCallback();

    static BaseDropdown* text(const std::string& title, const std::vector<std::string>& values,
                              ValueSelectedEvent::Callback cb, size_t selected = 0, const std::string& hint = "");

protected:
    BRLS_BIND(RecyclingGrid, recycler, "grid_dropdown/recycler");

    ValueSelectedEvent::Callback cb;
    size_t selected;
    brls::Event<RecyclingGridItem*> cellFocusDidChangeEvent;
};