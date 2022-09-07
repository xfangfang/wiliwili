//
// Created by fang on 2022/6/15.
//

// register this view in main.cpp
//#include "view/recycling_grid.hpp"
//    brls::Application::registerXMLView("RecyclingGrid", RecyclingGrid::create);
// <brls:View xml=@res/xml/views/recycling_grid.xml

#pragma once

#include <borealis.hpp>

class RecyclingGrid;

class RecyclingGridItem : public brls::Box
{
public:
    RecyclingGridItem();
    ~RecyclingGridItem();

    /*
     * Cell's position inside recycler frame
     */
    size_t getIndex() const;

    /*
     * DO NOT USE! FOR INTERNAL USAGE ONLY!
     */
    void setIndex(size_t value);

    /*
     * A string used to identify a cell that is reusable.
     */
    std::string reuseIdentifier;

    /*
     * Prepares a reusable cell for reuse by the recycler frame's data source.
     */
    virtual void prepareForReuse() { }

    /*
     * 表单项回收
     */
    virtual void cacheForReuse() { }

    static RecyclingGridItem* create();

private:
    size_t index;
};

class RecyclingGridDataSource
{
public:
    virtual ~RecyclingGridDataSource() {}

    /*
     * Tells the data source to return the number of items in a recycler frame.
     */
    virtual size_t getItemCount() { return 0; }

    /*
     * Asks the data source for a cell to insert in a particular location of the recycler frame.
     */
    virtual RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) { return nullptr; }

    /*
     * Asks the data source for the height to use for a row in a specified location.
     * Return -1 to use autoscaling.
     */
    virtual float heightForRow(RecyclingGrid* recycler, size_t index) { return -1; }

    /*
     * Tells the data source a row is selected.
     */
    virtual void onItemSelected(RecyclingGrid* recycler, size_t index) { }

    virtual void clearData() { }

};

class RecyclingGrid : public brls::ScrollingFrame {

public:
    RecyclingGrid();

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style, brls::FrameContext* ctx) override;

    void registerCell(std::string identifier, std::function<RecyclingGridItem*()> allocation);

    void setDefaultCellFocus(size_t index);

    size_t getDefaultCellFocus() const;

    void setDataSource(RecyclingGridDataSource* source);

    RecyclingGridDataSource* getDataSource() const;

    void showSkeleton(unsigned int num = 12);

    // 重新加载数据
    void reloadData();

    void notifyDataChanged();

    void clearData();

    void setEmpty(std::string msg="");

    void setError(std::string error="");

    void selectRowAt(size_t index, bool animated);

    //    计算从start元素的顶点到index（不包含index）元素顶点的距离
    float getHeightByCellIndex(int index, int start=0);

    View* getNextCellFocus(brls::FocusDirection direction, View* currentView);

    void onLayout() override;

    /// 当前数据总数量
    size_t getItemCount();

    /// 当前数据总行数
    size_t getRowCount();

    /// 导航到页面尾部时触发回调函数
    void onNextPage(const std::function<void()>& callback = nullptr);

    void setPadding(float padding) override;
    void setPadding(float top, float right, float bottom, float left) override;
    void setPaddingTop(float top) override;
    void setPaddingRight(float right) override;
    void setPaddingBottom(float bottom) override;
    void setPaddingLeft(float left) override;


    // 获取一个列表项组件
    // 如果缓存列表中存在就从中取出一个
    // 如果缓存列表为空则生成一个新的
    RecyclingGridItem* dequeueReusableCell(std::string identifier);

    ~RecyclingGrid();

    static View *create();

    /// 元素间距
    float estimatedRowSpace = 20;

    /// 默认行高(元素实际高度 = 默认行高 - 元素间隔)
    float estimatedRowHeight = 240;

    /// 列数
    int spanCount = 4;

    /// 预取的行数
    int preFetchLine = 1;

    /// 瀑布流模式，每一项高度不固定（仅在spanCount为1时可用）
    bool isFlowMode = false;

private:
    RecyclingGridDataSource* dataSource = nullptr;
    bool layouted                  = false;
    float oldWidth = -1;

    uint32_t visibleMin, visibleMax;
    size_t defaultCellFocus = 0;

    float paddingTop    = 0;
    float paddingRight  = 0;
    float paddingBottom = 0;
    float paddingLeft   = 0;

    std::function<void()> nextPageCallback = nullptr;

    brls::Box* contentBox;
    brls::Image* hintImage;
    brls::Label* hintLabel;
    brls::Rect renderedFrame;
    std::vector<float> cellHeightCache;
    std::map<std::string, std::vector<RecyclingGridItem*>*> queueMap;
    std::map<std::string, std::function<RecyclingGridItem*(void)>> allocationMap;

    //检查宽度是否有变化
    bool checkWidth();

    // 回收列表项
    void queueReusableCell(RecyclingGridItem* cell);

    void itemsRecyclingLoop();

    void addCellAt(size_t index, int downSide);

};

class RecyclingGridContentBox : public brls::Box
{
public:
    RecyclingGridContentBox(RecyclingGrid* recycler);
    brls::View* getNextFocus(brls::FocusDirection direction, brls::View* currentView) override;

private:
    RecyclingGrid* recycler;
};


class SkeletonCell: public RecyclingGridItem {
public:
    SkeletonCell();

    static RecyclingGridItem* create();

    void draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext *ctx) override;

private:
    NVGcolor background = brls::Application::getTheme()["color/grey_3"];
};