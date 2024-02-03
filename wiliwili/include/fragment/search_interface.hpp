//
// Created by fang on 2024/2/3.
//

#pragma once

#include "view/auto_tab_frame.hpp"
#include "utils/event_helper.hpp"

#define SEARCH_KEY "SEARCH_KEY"

class SearchEventInterface {
public:
    SearchEventInterface();

    virtual ~SearchEventInterface();

    virtual void onAppEvent(const std::string& event, void* data);

    virtual void requestSearch(const std::string& key) = 0;

private:
    CustomEvent::Subscription appEventSubscribeID;
};

class SearchAttachedView : public AttachedView, public SearchEventInterface {
public:
    void onCreate() override;

    void onHide() override;

    void onShow() override;

    void onAppEvent(const std::string& event, void* data) override;
private:
    bool tabShow = false;
    std::string lastKey;
};