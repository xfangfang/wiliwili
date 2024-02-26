//
// Created by fang on 2022/12/28.
//

// register this view in main.cpp
//#include "view/check_box.hpp"
//    brls::Application::registerXMLView("CheckBox", CheckBox::create);
// <brls:View xml=@res/xml/views/check_box.xml

#pragma once

#include <borealis/core/view.hpp>

class BiliCheckBox : public brls::View {
public:
    BiliCheckBox();
    virtual void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                      brls::FrameContext* ctx) override;

    void setChecked(bool value);

    bool getChecked();

    static View* create();

protected:
    bool checked = false;
};