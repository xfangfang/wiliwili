//
// Created by fang on 2022/7/9.
//

#include "view/grid_dropdown.hpp"


RecyclingGridItem* DataSourceDropdown::cellForRow(RecyclingGrid* recycler, size_t index) {
    //从缓存列表中取出 或者 新生成一个表单项
    GridRadioCell* item = (GridRadioCell*)recycler->dequeueReusableCell("Cell");

    auto r = this->data[index];
    item->title->setText(this->data[index]);
    item->setSelected(index == dropdown->getSelected());

    return item;
}

size_t DataSourceDropdown::getItemCount() {
    return this->data.size();
}

void DataSourceDropdown::onItemSelected(RecyclingGrid* recycler, size_t index) {
    this->dropdown->getSelectCallback()(index);
    brls::Application::popActivity(brls::TransitionAnimation::FADE);
}