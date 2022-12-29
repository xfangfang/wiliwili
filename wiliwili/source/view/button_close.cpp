//
// Created by fang on 2022/12/27.
//

#include "view/button_close.hpp"

ButtonClose::ButtonClose() {
    this->inflateFromXMLRes("xml/views/button_close.xml");
    brls::Logger::debug("View ButtonClose: create");

    this->addGestureRecognizer(new brls::TapGestureRecognizer(this, [this](){
        this->dismiss();
    }));
}

ButtonClose::~ButtonClose() { brls::Logger::debug("View ButtonClose: delete"); }

brls::View* ButtonClose::create() { return new ButtonClose(); }