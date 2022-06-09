//
// Created by fang on 2022/5/16.
//

#pragma once

#include <borealis.hpp>


class VideoGrid : public Box {
public:

    VideoGrid(float itemPercentage=22, float itemHeight=220, float itemMargin=10):ItemWidthPercentage(itemPercentage),ItemHeight(itemHeight),ItemMargin(itemMargin){
        YGNodeStyleSetFlexDirection(this->ygNode, YGFlexDirection::YGFlexDirectionRow);
        YGNodeStyleSetFlexWrap(this->ygNode, YGWrap::YGWrapWrap);
        YGNodeStyleSetAlignContent(this->ygNode, YGAlign::YGAlignFlexStart);
    }

    void setWidth(float width){
        Box::setWidth(width);
        float viewWidth = this->getWidth();
        float itemWidth = viewWidth * this->ItemWidthPercentage + this->ItemMargin * 2;
        this->spanCount = viewWidth / itemWidth;
        Logger::error("setWidth: {}, spanCount: {}", viewWidth, spanCount);
    }

    void setWidthPercentage(float percentage)
    {
        Box::setWidthPercentage(percentage);
        float viewWidth = this->getWidth();
        float itemWidth = viewWidth * this->ItemWidthPercentage + this->ItemMargin * 2;
        this->spanCount = viewWidth / itemWidth;
        Logger::error("setWidth: {}, spanCount: {}", viewWidth, spanCount);
    }

    void invalidate() override {
        View::invalidate();
        Logger::error("Grid: {}/{}", getWidth(), getHeight());
    }

    void addView(View* view, size_t position) override {
        Logger::error("add View to: {}", position);
        view->setWidthPercentage(ItemWidthPercentage);
        view->setMargins(ItemMargin,ItemMargin,ItemMargin,ItemMargin);
        view->setHeight(ItemHeight);

        Box::addView(view, position);
    }

    View* getNextFocus(FocusDirection direction, View* currentView) override {
        auto parentUserData = (size_t*)currentView->getParentUserData();

        // Allow up and down when axis is ROW
        if ((this->getAxis() == Axis::ROW && direction != FocusDirection::LEFT && direction != FocusDirection::RIGHT)) {
            size_t row_offset = spanCount;
            if (direction == FocusDirection::UP) row_offset = -spanCount;
            View* row_currentFocus       = nullptr;
            size_t row_currentFocusIndex = *((size_t*)parentUserData) + row_offset;

            if (row_currentFocusIndex >= this->getChildren().size()) {
                row_currentFocusIndex -= *((size_t*)parentUserData) % spanCount;
            }

            while (!row_currentFocus && row_currentFocusIndex >= 0 && row_currentFocusIndex < this->getChildren().size())
            {
                row_currentFocus = this->getChildren()[row_currentFocusIndex]->getDefaultFocus();
                row_currentFocusIndex += row_offset;
            }
            if (row_currentFocus) {
                return row_currentFocus;
            }
        }

        if (this->getAxis() == Axis::ROW) {
            int position = *((size_t*)parentUserData) % spanCount;
            if((direction == FocusDirection::LEFT && position == 0) || (direction == FocusDirection::RIGHT && position == (spanCount-1))) {
                View* next = getParentNavigationDecision(this, nullptr, direction);
                if (!next && hasParent())
                    next = getParent()->getNextFocus(direction, this);
                return next;
            }
        }

        return Box::getNextFocus(direction, currentView);
    }

    static brls::View* create(){
        return new VideoGrid();
    }

private:
    int spanCount = 4; // 列数
    float ItemWidthPercentage = 20; //元素宽度百分比
    float ItemHeight = 300; //元素高度
    float ItemMargin = 0; //元素周围空白
//    BRLS_BIND(brls::RecyclerFrame, recycler, "grid/recycler");
};