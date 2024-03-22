//
// Created by fang on 2022/6/15.
//

// register this view in main.cpp
//#include "view/recycling_grid.hpp"
//    brls::Application::registerXMLView("RecyclingGrid", RecyclingGrid::create);
// <brls:View xml=@res/xml/views/recycling_grid.xml

#pragma once

#include <map>
#include <borealis/core/application.hpp>
#include <borealis/core/bind.hpp>
#include <borealis/views/scrolling_frame.hpp>

namespace brls {
class Label;
class Image;
}  // namespace brls
class RecyclingGrid;
class ButtonRefresh;

class RecyclingGridItem : public brls::Box {
public:
    RecyclingGridItem();
    ~RecyclingGridItem() override;

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
    virtual void prepareForReuse() {}

    /*
     * 表单项回收
     */
    virtual void cacheForReuse() {}

private:
    size_t index;
};

class RecyclingGridDataSource {
public:
    virtual ~RecyclingGridDataSource() = default;

    /*
     * Tells the data source to return the number of items in a recycler frame.
     */
    virtual size_t getItemCount() { return 0; }

    /*
     * Asks the data source for a cell to insert in a particular location of the recycler frame.
     */
    virtual RecyclingGridItem* cellForRow(RecyclingGrid* recycler,
                                          size_t index) {
        return nullptr;
    }

    /*
     * Asks the data source for the height to use for a row in a specified location.
     * Return -1 to use autoscaling.
     */
    virtual float heightForRow(RecyclingGrid* recycler, size_t index) {
        return -1;
    }

    /*
     * Tells the data source a row is selected.
     */
    virtual void onItemSelected(RecyclingGrid* recycler, size_t index) {}

    virtual void clearData() = 0;
};

class RecyclingGridContentBox;

class RecyclingGrid : public brls::ScrollingFrame {
public:
    RecyclingGrid();

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    void registerCell(std::string identifier,
                      std::function<RecyclingGridItem*()> allocation);

    void setDefaultCellFocus(size_t index);

    size_t getDefaultCellFocus() const;

    void setDataSource(RecyclingGridDataSource* source);

    RecyclingGridDataSource* getDataSource() const;

    void showSkeleton(unsigned int num = 30);

    void refresh();

    void setRefreshAction(const std::function<void()>& event);

    // 重新加载数据
    void reloadData();

    void notifyDataChanged();

    /// 获取当前指定索引数据所在的item指针
    ///（注意，因为是循环使用列表项的，所以此指针只能在获取时刻在主线程内使用）
    RecyclingGridItem* getGridItemByIndex(size_t index);

    std::vector<RecyclingGridItem*>& getGridItems();

    void clearData();

    void setEmpty(std::string msg = "");

    void setError(std::string error = "");

    void selectRowAt(size_t index, bool animated);

    //    计算从start元素的顶点到index（不包含index）元素顶点的距离
    float getHeightByCellIndex(size_t index, size_t start = 0);

    View* getNextCellFocus(brls::FocusDirection direction, View* currentView);

    void forceRequestNextPage();

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

    void setPaddingRightPercentage(float right);
    void setPaddingLeftPercentage(float left);

    float getPaddingLeft();
    float getPaddingRight();

    // 获取一个列表项组件
    // 如果缓存列表中存在就从中取出一个
    // 如果缓存列表为空则生成一个新的
    RecyclingGridItem* dequeueReusableCell(std::string identifier);

    brls::View* getDefaultFocus() override;

    ~RecyclingGrid() override;

    static View* create();

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
    bool layouted                       = false;
    float oldWidth                      = -1;

    bool requestNextPage = false;
    // true表示正在请求下一页，此时不会再次触发下一页请求
    // 数据为空时不请求下一页，因为有些时候首页和下一页请求的内容或方式不同
    // 当列表元素有变动时（添加或修改数据源，会重置为false，这是将允许请求下一页）

    uint32_t visibleMin, visibleMax;
    size_t defaultCellFocus = 0;

    float paddingTop    = 0;
    float paddingRight  = 0;
    float paddingBottom = 0;
    float paddingLeft   = 0;

    bool paddingPercentage = false;

    std::function<void()> nextPageCallback = nullptr;
    std::function<void()> refreshAction    = nullptr;

    RecyclingGridContentBox* contentBox = nullptr;
    brls::Image* hintImage;
    brls::Label* hintLabel;
    ButtonRefresh* refreshButton;
    brls::Rect renderedFrame;
    std::vector<float> cellHeightCache;
    std::map<std::string, std::vector<RecyclingGridItem*>*> queueMap;
    std::map<std::string, std::function<RecyclingGridItem*(void)>>
        allocationMap;

    //检查宽度是否有变化
    bool checkWidth();

    // 回收列表项
    void queueReusableCell(RecyclingGridItem* cell);

    void itemsRecyclingLoop();

    /**
     * 在指定位置添加一个列表项
     * 内部更新 renderedFrame 的值，假设有一个每一项都绘制的超长列表，renderedFrame 的 y 表示当前截取绘制的顶部坐标，height 表示当前绘制的高度
     * 当添加一个列表项时，renderedFrame 的 height 增加一项的高度（注意，只在每行的第一个列表项添加时才更新列表项的高度）
     * @param index 指定的位置
     * @param downSide 是向下添加还是向上添加，当向上添加时 将 renderedFrame 的 y 减去当前列表项的高度。（y 的值只在向上添加或移除时候改变）
     */
    void addCellAt(size_t index, bool downSide);
};

class RecyclingGridContentBox : public brls::Box {
public:
    RecyclingGridContentBox(RecyclingGrid* recycler);
    brls::View* getNextFocus(brls::FocusDirection direction,
                             brls::View* currentView) override;

private:
    RecyclingGrid* recycler;
};

class SkeletonCell : public RecyclingGridItem {
public:
    SkeletonCell();

    static RecyclingGridItem* create();

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

private:
    NVGcolor background = brls::Application::getTheme()["color/grey_3"];
};