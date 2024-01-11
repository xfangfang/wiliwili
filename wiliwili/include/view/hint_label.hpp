//
// Created by fang on 2023/8/6.
//

// register this view in main.cpp
//#include "view/hint_label.hpp"
//    brls::Application::registerXMLView("HintLabel", HintLabel::create);
// <brls:View xml=@res/xml/views/hint_label.xml

#pragma once

#include <borealis/views/label.hpp>

class HintLabel : public brls::Label {
public:
    HintLabel();

    ~HintLabel() override;

    static View* create();
};