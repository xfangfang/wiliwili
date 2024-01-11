//
// Created by fang on 2023/1/1.
//

#pragma once

#include "view/recycling_grid.hpp"
#include "view/auto_tab_frame.hpp"

class PlayerTabCell : public RecyclingGridItem {
public:
    PlayerTabCell();

    void setSelected(bool value);

    bool getSelected();

    /**
     * 右侧的标识，一般用来展示会员信息
     * @param value     标识内容，为空则隐藏
     * @param color     标识背景色，为空则设为透明色
     */
    void setBadge(std::string value, std::string color);

    /**
     * 右侧的标识，一般用来展示会员信息
     * @param value     标识内容，为空则隐藏
     * @param color     标识背景色
     * @param textColor 标识颜色
     */
    void setBadge(std::string value, NVGcolor color, NVGcolor textColor);

    static RecyclingGridItem* create();

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    BRLS_BIND(brls::Label, title, "player/tab/cell/title");
    BRLS_BIND(brls::Box, badgeBox, "player/tab/cell/badgeBox");
    BRLS_BIND(brls::Label, badge, "player/tab/cell/badge");

private:
    bool selected = false;
};

class PlayerTabHeader : public RecyclingGridItem {
public:
    PlayerTabHeader();

    static RecyclingGridItem* create();

    BRLS_BIND(brls::Label, title, "player/tab/title");
};

template <class T>
class PlayerTabDataSource : public RecyclingGridDataSource {
public:
    typedef std::function<RecyclingGridItem*(RecyclingGrid*, PlayerTabDataSource*, T&)> Cell4RowFunc;
    typedef std::function<float(RecyclingGrid*, PlayerTabDataSource*, T&)> Height4RowFunc;
    typedef brls::Event<RecyclingGrid*, PlayerTabDataSource<T>*, size_t, T> SelectEvent;

    /**
     *
     * @param result 数据内容
     * @param cb cell选中的回调
     * @param fun 用来设置cell内容的回调
     * @param heightFun 用来设置cell高度的回调
     * @param defaultIndex
     */
    PlayerTabDataSource(std::vector<T> result, SelectEvent* cb, Cell4RowFunc fun, Height4RowFunc heightFun = nullptr,
                        size_t defaultIndex = 0)
        : data(result), selectEvent(cb), cell4Row(fun), height4Row(heightFun), currentIndex(defaultIndex) {}

    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override {
        return cell4Row(recycler, this, data[index]);
    }

    float heightForRow(RecyclingGrid* recycler, size_t index) override {
        if (!height4Row) return -1;
        return height4Row(recycler, this, data[index]);
    }

    size_t getItemCount() override { return data.size(); }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override {
        if (selectEvent) selectEvent->fire(recycler, this, index, data[index]);
    }

    std::vector<T>& getData() { return this->data; }

    void clearData() override { this->data.clear(); }

    size_t getCurrentIndex() { return currentIndex; }

    void setCurrentIndex(size_t index) { this->currentIndex = index; }

private:
    std::vector<T> data;
    SelectEvent* selectEvent;
    Cell4RowFunc cell4Row;
    Height4RowFunc height4Row;
    size_t currentIndex;
};

template <class T>
class BasePlayerTabFragment : public AttachedView {
public:
    typedef std::function<RecyclingGridItem*(RecyclingGrid*, PlayerTabDataSource<T>*, T&)> Cell4RowFunc;
    typedef std::function<float(RecyclingGrid*, PlayerTabDataSource<T>*, T&)> Height4RowFunc;
    typedef brls::Event<RecyclingGrid*, PlayerTabDataSource<T>*, size_t, T> SelectEvent;
    typedef std::function<void(RecyclingGrid*, PlayerTabDataSource<T>*)> InitFun;

    /**
     *
     * @param result 列表数据
     * @param fun 设置单个列表项的回调函数
     * @param heightFun 列表项高度自定义回调函数
     * @param init 构造函数中调用，用来自定义初始化列表
     * @param defaultIndex 列表默认选中序号
     */
    BasePlayerTabFragment(std::vector<T> result, Cell4RowFunc fun, InitFun init = nullptr, size_t defaultIndex = 0,
                          Height4RowFunc heightFun = nullptr) {
        this->setMarginTop(12);
        this->setMarginBottom(12);

        grid = new RecyclingGrid();
        grid->setPadding(0, 30, 0, 20);
        grid->setGrow(1);
        grid->applyXMLAttribute("spanCount", "1");
        grid->applyXMLAttribute("itemSpace", "0");
        grid->applyXMLAttribute("itemHeight", "50");
        grid->registerCell("Cell", []() { return PlayerTabCell::create(); });
        grid->registerCell("Header", []() { return PlayerTabHeader::create(); });
        grid->setDefaultCellFocus(defaultIndex);
        if (heightFun) grid->applyXMLAttribute("flowMode", "true");
        ds = new PlayerTabDataSource<T>(result, &callback, fun, heightFun, defaultIndex);
        grid->setDataSource(ds);
        this->addView(grid);

        if (init) init(grid, ds);
    }

    SelectEvent* getSelectEvent() { return &callback; }

private:
    SelectEvent callback;
    RecyclingGrid* grid;
    PlayerTabDataSource<T>* ds;
};