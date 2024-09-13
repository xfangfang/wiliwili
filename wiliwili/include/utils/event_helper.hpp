//
// Created by fang on 2024/1/10.
//

#pragma once
#include <string>
#include <borealis/core/singleton.hpp>
#include <borealis/core/event.hpp>

typedef enum MpvEventEnum {
    /**
     * 视频播放的一系列状态
     * 播放器组件会订阅这些事件处理不同的逻辑
     */
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

    /**
     * 视频缓存速度更新时触发此事件
     * 播放器组件会订阅此事件，用于在网络加载卡顿时显示缓冲速度
     */
    CACHE_SPEED_CHANGE,

    /**
     * 视频播放速度调整时触发此事件
     * 弹幕组件和播放器组件的相关功能会订阅此事件
     */
    VIDEO_SPEED_CHANGE,

    /**
     * mpv内部音量调整时触发以下事件
     * DLNA 播放页面会订阅相关事件，并将音量状态同步到手机投屏端
     */
    VIDEO_VOLUME_CHANGE,
    VIDEO_MUTE,
    VIDEO_UNMUTE,

    /**
     * 每次播放视频前，触发此事件
     * 重置视频进度、视频比例、视频时长等信息
     * 弹幕组件、字幕组件和播放器组件的相关功能会订阅此事件
     */
    RESET,

    /**
     * 需要重置到播放器初识参数时，触发此事件（比如清空着色器设置）
     * 视频播放页会订阅此事件，用来在播放器重置后重新设置播放链接
     */
     RESTART,
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
