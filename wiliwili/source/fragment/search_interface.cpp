//
// Created by fang on 2024/2/3.
//

#include "activity/search_activity.hpp"
#include "fragment/search_interface.hpp"

SearchEventInterface::SearchEventInterface() {
    appEventSubscribeID =
        SEARCH_E->subscribe([this](const std::string& event, void* data) { this->onAppEvent(event, data); });
}

SearchEventInterface::~SearchEventInterface() { SEARCH_E->unsubscribe(appEventSubscribeID); }

void SearchEventInterface::onAppEvent(const std::string& event, void* data) {
    if (event != SEARCH_KEY) return;
    if (data == nullptr) return;
    this->requestSearch((const char*)data);
}

void SearchAttachedView::onCreate() {
    // 创建 tab 时，设置一个搜索关键字
    lastKey = SearchActivity::currentKey;
}

void SearchAttachedView::onHide() {
    tabShow = false;
    // 当 tab 从展示转为隐藏，清空等待搜索的关键字
    lastKey.clear();
}

void SearchAttachedView::onShow() {
    tabShow = true;
    // 当 tab 从隐藏转为展示，且存在等待搜索的关键字，则进行搜索
    if (!lastKey.empty()) {
        this->requestSearch(lastKey);
        lastKey.clear();
    }
}

void SearchAttachedView::onAppEvent(const std::string& event, void* data) {
    if (event != SEARCH_KEY) return;
    if (data == nullptr) return;
    if (!tabShow) {
        // 当隐藏的 tab 收到搜索事件时，仅记录搜索关键字，等待展示时再进行搜索
        lastKey = (const char*)data;
        return;
    }
    this->requestSearch((const char*)data);
}