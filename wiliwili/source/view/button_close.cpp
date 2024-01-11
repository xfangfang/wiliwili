//
// Created by fang on 2022/12/27.
//

#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/views/label.hpp>

#include "view/button_close.hpp"

ButtonClose::ButtonClose() {
    this->inflateFromXMLRes("xml/views/button_close.xml");
    brls::Logger::debug("View ButtonClose: create");

    this->registerColorXMLAttribute("textColor", [this](NVGcolor value) { this->setTextColor(value); });

    this->registerClickAction([this](...) {
        this->dismiss();
        return true;
    });
    this->addGestureRecognizer(new brls::TapGestureRecognizer(this));
}

void ButtonClose::setTextColor(NVGcolor color) { this->label->setTextColor(color); }

ButtonClose::~ButtonClose() { brls::Logger::debug("View ButtonClose: delete"); }

brls::View* ButtonClose::create() { return new ButtonClose(); }