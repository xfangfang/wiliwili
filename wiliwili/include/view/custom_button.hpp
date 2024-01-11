//
// Created by fang on 2022/8/22.
//

// register this view in main.cpp
//#include "view/custom_button.hpp"
//    brls::Application::registerXMLView("CustomButton", CustomButton::create);
// <brls:View xml=@res/xml/views/custom_button.xml

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/event.hpp>

class CustomButton : public brls::Box {
public:
    CustomButton();

    ~CustomButton() override;

    static View *create();

    void onFocusGained() override;

    void onFocusLost() override;

    brls::Event<bool> *getFocusEvent();

    View *getNextFocus(brls::FocusDirection direction, View *currentView) override;

    void setCustomNavigation(std::function<brls::View *(brls::FocusDirection)> navigation);

private:
    brls::Event<bool> focusEvent;
    std::function<brls::View *(brls::FocusDirection)> customNavigation = nullptr;
};