//
// Created by fang on 2022/12/27.
//

// register this view in main.cpp
//#include "view/button_close.hpp"
//    brls::Application::registerXMLView("ButtonClose", ButtonClose::create);
// <brls:View xml=@res/xml/views/button_close.xml

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

namespace brls {
class Label;
}

class ButtonClose : public brls::Box {
public:
    ButtonClose();

    void setTextColor(NVGcolor color);

    ~ButtonClose() override;

    static View* create();

private:
    BRLS_BIND(brls::Label, label, "close/button/text");
};