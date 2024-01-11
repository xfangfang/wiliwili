//
// Created by fang on 2022/6/9.
//

// register this view in main.cpp
// #include "view/up_user_small.hpp"
// brls::Application::registerXMLView("UpUserSmall", UpUserSmall::create);
#pragma once

#include <borealis/core/box.hpp>

class UpUserSmall : public brls::Box {
public:
    UpUserSmall();

    ~UpUserSmall() override;

    static View *create() { return new UpUserSmall(); }
};