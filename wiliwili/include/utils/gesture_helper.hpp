//
// Created by fang on 2024/1/8.
//

#pragma once

#include <borealis/core/gesture.hpp>

#include "presenter/presenter.h"

enum class OsdGestureType {
    TAP,
    DOUBLE_TAP_START,
    DOUBLE_TAP_END,
    LONG_PRESS_START,
    LONG_PRESS_END,
    LONG_PRESS_CANCEL,
    HORIZONTAL_PAN_START,
    HORIZONTAL_PAN_UPDATE,
    HORIZONTAL_PAN_END,
    HORIZONTAL_PAN_CANCEL,
    LEFT_VERTICAL_PAN_START,
    LEFT_VERTICAL_PAN_UPDATE,
    LEFT_VERTICAL_PAN_END,
    LEFT_VERTICAL_PAN_CANCEL,
    RIGHT_VERTICAL_PAN_START,
    RIGHT_VERTICAL_PAN_UPDATE,
    RIGHT_VERTICAL_PAN_END,
    RIGHT_VERTICAL_PAN_CANCEL,
    NONE,
};

struct OsdGestureStatus {
    OsdGestureType osdGestureType;  // Gesture type
    brls::GestureState state;       // Gesture state
    brls::Point position;           // Current position
    float deltaX, deltaY;
};
typedef brls::Event<OsdGestureStatus> OsdGestureEvent;

class OsdGestureRecognizer : public brls::GestureRecognizer, public Presenter {
public:
    explicit OsdGestureRecognizer(const OsdGestureEvent::Callback& respond);

    brls::GestureState recognitionLoop(brls::TouchState touch, brls::MouseState mouse, brls::View* view,
                                       brls::Sound* soundToPlay) override;

    // Get current state of recognizer
    OsdGestureStatus getCurrentStatus();

    // Get tap gesture event
    [[nodiscard]] OsdGestureEvent getTapGestureEvent() const { return tapEvent; }

private:
    OsdGestureEvent tapEvent;
    brls::Point position{};
    brls::Point delta{};
    float deltaX{}, deltaY{};
    int64_t startTime{};
    int64_t endTime{};
    size_t iter{};
    OsdGestureType osdGestureType{OsdGestureType::NONE};
    brls::GestureState lastState{brls::GestureState::END};
};