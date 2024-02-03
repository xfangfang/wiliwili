//
// Created by fang on 2024/1/10.
//

#pragma once
#include <string>
#include <borealis/core/singleton.hpp>
#include <borealis/core/event.hpp>

typedef enum MpvEventEnum {
    MPV_LOADED,
    MPV_PAUSE,
    MPV_RESUME,
    MPV_IDLE,
    MPV_STOP,
    MPV_FILE_ERROR,
    LOADING_START,
    LOADING_END,
    UPDATE_DURATION,
    UPDATE_PROGRESS,
    START_FILE,
    END_OF_FILE,
    CACHE_SPEED_CHANGE,
    VIDEO_SPEED_CHANGE,
    VIDEO_VOLUME_CHANGE,
    VIDEO_MUTE,
    VIDEO_UNMUTE,
    RESET,
} MpvEventEnum;

typedef brls::Event<MpvEventEnum> MPVEvent;
typedef brls::Event<std::string, void *> CustomEvent;

class EventHelper : public brls::Singleton<EventHelper> {
public:
    MPVEvent *getMpvEvent();

    /**
     * 可以用于共享自定义事件
     * 传递内容为: string类型的事件名与一个任意类型的指针
     */
    CustomEvent *getCustomEvent();

    /**
     * 专门用于搜索页面的事件
     * 传递内容为: string类型的事件名与一个任意类型的指针
     */
    CustomEvent *getSearchEvent();

    // 自定义的事件，传递内容为: string类型的事件名与一个任意类型的指针
    CustomEvent customEvent;
    CustomEvent searchEvent;
};

#define MPV_E EventHelper::instance().getMpvEvent()
#define APP_E EventHelper::instance().getCustomEvent()
#define SEARCH_E EventHelper::instance().getSearchEvent()
