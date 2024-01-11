//
// Created by fang on 2022/8/22.
//

#include "view/custom_button.hpp"

CustomButton::CustomButton() { brls::Logger::debug("View CustomButton: create"); }

CustomButton::~CustomButton() { brls::Logger::debug("View CustomButton: delete"); }

brls::View* CustomButton::create() { return new CustomButton(); }

void CustomButton::onFocusLost() {
    this->focusEvent.fire(false);
    Box::onFocusLost();
}

void CustomButton::onFocusGained() {
    this->focusEvent.fire(true);
    Box::onFocusGained();
}

brls::Event<bool>* CustomButton::getFocusEvent() { return &this->focusEvent; }

brls::View* CustomButton::getNextFocus(brls::FocusDirection direction, brls::View* currentView) {
    brls::View* next = nullptr;
    if (this->customNavigation) {
        next = this->customNavigation(direction);
    }

    if (!next) next = Box::getNextFocus(direction, currentView);

    return next;
}

void CustomButton::setCustomNavigation(std::function<brls::View*(brls::FocusDirection)> navigation) {
    this->customNavigation = navigation;
}
