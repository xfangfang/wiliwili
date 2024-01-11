//
// Created by maye174 on 2023/11/13.
//

#pragma once

#include "api/live/extract_messages.hpp"

#include <chrono>
#include <cstddef>
#include <deque>
#include <map>
#include <mutex>

#include <nanovg.h>
#include <borealis/core/singleton.hpp>

using time_p = std::chrono::time_point<std::chrono::system_clock>;
class LiveDanmakuItem {
public:
    LiveDanmakuItem(danmaku_t *danmaku);
    LiveDanmakuItem(const LiveDanmakuItem &item);
    LiveDanmakuItem(LiveDanmakuItem &&item);
    ~LiveDanmakuItem() {
        if (!danmaku) return;
        danmaku_t_free(danmaku);
        free(danmaku);
    }
    danmaku_t *danmaku;
    time_p time;
    size_t line  = 0;
    float length = 0;
    float speed  = 0;
};

class LiveDanmakuCore : public brls::Singleton<LiveDanmakuCore> {
public:
    //0-60
    static inline int DANMAKU_FILTER_LEVEL_LIVE = 0;

    std::vector<std::pair<time_p, time_p>> scroll_lines;
    std::vector<int> center_lines;

    float line_height;

    std::deque<LiveDanmakuItem> next;
    std::mutex next_mutex;

    std::map<int, std::deque<LiveDanmakuItem>> now;

    void reset();
    void add(const std::vector<LiveDanmakuItem> &dan_l);
    void draw(NVGcontext *vg, float x, float y, float width, float height, float alpha);

    bool init_danmaku(NVGcontext *vg, LiveDanmakuItem &i, float width, int LINES, float SECOND, time_p now, int time);
};