//
// Created by fang on 2024/1/8.
//

#include <borealis/core/thread.hpp>
#include <borealis/core/view.hpp>
#include <borealis/core/application.hpp>

#include "utils/gesture_helper.hpp"

// Delta from touch starting point to current, when
// touch will be recognized as pan movement
#define MAX_DELTA_MOVEMENT 24

// Time in ms to recognize long press
#define LONG_TIME_MS 250  // 250ms
#define LONG_TIME_US (LONG_TIME_MS * 1000)

OsdGestureRecognizer::OsdGestureRecognizer(const OsdGestureEvent::Callback& respond) {
    tapEvent.subscribe(respond);
    this->startTime = 0;
    this->endTime   = -1 - LONG_TIME_US;
}

/// 触摸被打断时保证触发对应的完结事件
static inline OsdGestureType updateGestureType(OsdGestureType osdGestureType) {
    if (osdGestureType == OsdGestureType::LONG_PRESS_START) {
        return OsdGestureType::LONG_PRESS_CANCEL;
    } else if (osdGestureType == OsdGestureType::LEFT_VERTICAL_PAN_START ||
               osdGestureType == OsdGestureType::LEFT_VERTICAL_PAN_UPDATE) {
        return OsdGestureType::LEFT_VERTICAL_PAN_CANCEL;
    } else if (osdGestureType == OsdGestureType::RIGHT_VERTICAL_PAN_START ||
               osdGestureType == OsdGestureType::RIGHT_VERTICAL_PAN_UPDATE) {
        return OsdGestureType::RIGHT_VERTICAL_PAN_CANCEL;
    } else if (osdGestureType == OsdGestureType::HORIZONTAL_PAN_START ||
               osdGestureType == OsdGestureType::HORIZONTAL_PAN_UPDATE) {
        return OsdGestureType::HORIZONTAL_PAN_CANCEL;
    }
    return osdGestureType;
}

brls::GestureState OsdGestureRecognizer::recognitionLoop(brls::TouchState touch, brls::MouseState mouse,
                                                         brls::View* view, brls::Sound* soundToPlay) {
    brls::TouchPhase phase = touch.phase;
    brls::Point position   = touch.position;

    if (phase == brls::TouchPhase::NONE) {
        position = mouse.position;
        phase    = mouse.leftButton;
    }

    if (!enabled || phase == brls::TouchPhase::NONE) return brls::GestureState::FAILED;

    // If not first touch frame and state is
    // INTERRUPTED or FAILED, stop recognition
    if (phase != brls::TouchPhase::START) {
        if (this->state == brls::GestureState::INTERRUPTED || this->state == brls::GestureState::FAILED) {
            if (this->state != lastState) {
                // 触摸被打断时保证触发对应的完结事件
                this->osdGestureType = updateGestureType(this->osdGestureType);
                this->tapEvent.fire(getCurrentStatus());
            }

            lastState = this->state;
            return this->state;
        }
    }

    switch (phase) {
        case brls::TouchPhase::START:
            brls::Application::giveFocus(view);
            this->startTime = brls::getCPUTimeUsec();
            if (this->startTime - this->endTime < LONG_TIME_US) {
                brls::cancelDelay(iter);
                this->osdGestureType = OsdGestureType::DOUBLE_TAP_START;
            } else {
                this->osdGestureType = OsdGestureType::NONE;
            }

            this->state    = brls::GestureState::UNSURE;
            this->position = position;
            this->tapEvent.fire(getCurrentStatus());
            break;
        case brls::TouchPhase::STAY: {
            auto frame = view->getFrame();
            // Check if touch is out view's bounds
            // if true, FAIL recognition
            if (!frame.pointInside(position)) {
                // 触摸被打断时保证触发对应的完结事件
                this->state          = brls::GestureState::FAILED;
                this->osdGestureType = updateGestureType(this->osdGestureType);
            } else if (this->osdGestureType == OsdGestureType::NONE) {
                // 到目前还没有识别出具体的触摸类型
                this->delta = position - this->position;
                if (fabs(this->delta.x) > MAX_DELTA_MOVEMENT) {
                    // 识别到水平滑动
                    this->osdGestureType = OsdGestureType::HORIZONTAL_PAN_START;
                } else if (fabs(this->delta.y) > MAX_DELTA_MOVEMENT) {
                    // 识别到垂直滑动
                    if (position.x < frame.getMidX()) {
                        this->osdGestureType = OsdGestureType::LEFT_VERTICAL_PAN_START;
                    } else {
                        this->osdGestureType = OsdGestureType::RIGHT_VERTICAL_PAN_START;
                    }
                } else if (brls::getCPUTimeUsec() - this->startTime > LONG_TIME_US) {
                    // 识别到长按
                    this->osdGestureType = OsdGestureType::LONG_PRESS_START;
                } else {
                    // 没有识别到触摸类型，不触发回调函数
                    break;
                }
            } else if (this->osdGestureType == OsdGestureType::HORIZONTAL_PAN_START ||
                       this->osdGestureType == OsdGestureType::HORIZONTAL_PAN_UPDATE) {
                // 更新水平滑动的状态
                this->delta          = position - this->position;
                this->deltaX         = delta.x / frame.size.width;
                this->osdGestureType = OsdGestureType::HORIZONTAL_PAN_UPDATE;
            } else if (this->osdGestureType == OsdGestureType::LEFT_VERTICAL_PAN_START ||
                       this->osdGestureType == OsdGestureType::LEFT_VERTICAL_PAN_UPDATE) {
                // 更新左侧垂直滑动的状态
                this->delta          = position - this->position;
                this->deltaY         = -delta.y / frame.size.height;
                this->osdGestureType = OsdGestureType::LEFT_VERTICAL_PAN_UPDATE;
            } else if (this->osdGestureType == OsdGestureType::RIGHT_VERTICAL_PAN_START ||
                       this->osdGestureType == OsdGestureType::RIGHT_VERTICAL_PAN_UPDATE) {
                // 更新右侧垂直滑动的状态
                this->delta          = position - this->position;
                this->deltaY         = -delta.y / frame.size.height;
                this->osdGestureType = OsdGestureType::RIGHT_VERTICAL_PAN_UPDATE;
            } else {
                break;
            }
            this->tapEvent.fire(getCurrentStatus());
            break;
        }

        case brls::TouchPhase::END: {
            this->endTime = brls::getCPUTimeUsec();
            this->state   = brls::GestureState::END;

            switch (this->osdGestureType) {
                case OsdGestureType::DOUBLE_TAP_START:
                    this->osdGestureType = OsdGestureType::DOUBLE_TAP_END;
                    this->tapEvent.fire(getCurrentStatus());
                    break;
                case OsdGestureType::LONG_PRESS_START:
                    this->osdGestureType = OsdGestureType::LONG_PRESS_END;
                    this->tapEvent.fire(getCurrentStatus());
                    break;
                case OsdGestureType::HORIZONTAL_PAN_START:
                case OsdGestureType::HORIZONTAL_PAN_UPDATE:
                    // 避免拖拽结束后立刻触摸被误认为是双击事件
                    this->endTime        = 0;
                    this->osdGestureType = OsdGestureType::HORIZONTAL_PAN_END;
                    this->tapEvent.fire(getCurrentStatus());
                    break;
                case OsdGestureType::LEFT_VERTICAL_PAN_START:
                case OsdGestureType::LEFT_VERTICAL_PAN_UPDATE:
                    this->endTime        = 0;
                    this->osdGestureType = OsdGestureType::LEFT_VERTICAL_PAN_END;
                    this->tapEvent.fire(getCurrentStatus());
                    break;
                case OsdGestureType::RIGHT_VERTICAL_PAN_START:
                case OsdGestureType::RIGHT_VERTICAL_PAN_UPDATE:
                    this->endTime        = 0;
                    this->osdGestureType = OsdGestureType::RIGHT_VERTICAL_PAN_END;
                    this->tapEvent.fire(getCurrentStatus());
                    break;
                default:
                    this->osdGestureType = OsdGestureType::TAP;
                    brls::cancelDelay(iter);
                    ASYNC_RETAIN
                    iter = brls::delay(LONG_TIME_MS, [ASYNC_TOKEN]() {
                        ASYNC_RELEASE
                        // 延时触发点击事件
                        this->tapEvent.fire(getCurrentStatus());
                    });
                    break;
            }
            break;
        }
        case brls::TouchPhase::NONE:
            this->state = brls::GestureState::FAILED;
            break;
    }

    lastState = this->state;
    return this->state;
}

OsdGestureStatus OsdGestureRecognizer::getCurrentStatus() {
    return OsdGestureStatus{
        .osdGestureType = this->osdGestureType,
        .state          = this->state,
        .position       = this->position,
        .deltaX         = this->deltaX,
        .deltaY         = this->deltaY,
    };
}