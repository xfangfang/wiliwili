#pragma once

#include "view/auto_tab_frame.hpp"

class AutoTabFrame;

class LocalTab : public AttachedView {
public:
    LocalTab();

    ~LocalTab();

    static View* create();

    void onCreate() override;
private:
};