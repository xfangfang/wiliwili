//
// Created by fang on 2022/6/9.
//

#pragma once

#include "view/auto_tab_frame.hpp"

class CustomButton;
class HomeTab : AttachedView {
public:
    HomeTab();

    ~HomeTab() override;

    static void openSearch();

    static View* create();

    void onCreate() override;

private:
    BRLS_BIND(AutoTabFrame, tabFrame, "home/tab/frame");
    BRLS_BIND(CustomButton, search, "home/search");
};