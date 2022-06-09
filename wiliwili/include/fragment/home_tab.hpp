//
// Created by fang on 2022/6/9.
//

#pragma once

#include <borealis.hpp>

class HomeTab : brls::Box {
public:
    HomeTab();

    ~HomeTab();

    static View* create(){
        return new HomeTab();
    }
private:
};