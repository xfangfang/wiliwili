//
// Created by fang on 2022/6/9.
//

#pragma once

#include "view/auto_tab_frame.hpp"

class HomeTab : AttachedView {
public:
    HomeTab();

    ~HomeTab();

    static View* create();

    void onCreate();

private:
    BRLS_BIND(AutoTabFrame, tabFrame, "home/tab/frame");
    BRLS_BIND(brls::Box, search, "home/search");
};