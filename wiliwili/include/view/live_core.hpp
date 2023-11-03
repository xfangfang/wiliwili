//
// Created by maye174 on 2023/11/13.
//

#pragma once

#include "api/live/extract_messages.hpp"

#include <queue>

#include "nanovg.h"
#include <borealis.hpp>
#include <borealis/core/singleton.hpp>

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
    float time;
    float length = 0;
    float speed  = 0;
    int line     = 0;
};

class LiveDanmakuCore : public brls::Singleton<LiveDanmakuCore> {
public:
    //0-60
    static inline int DANMAKU_FILTER_LEVEL_LIVE = 0;
    static inline bool DANMAKU_ON               = true;

    std::vector<std::pair<float, float>> scroll_lines;
    std::vector<int> center_lines;

    float line_height;

    int danmaku_font = brls::Application::getDefaultFont();

    std::queue<LiveDanmakuItem> danmaku_que;
    std::mutex que_mutex;

    std::map<int, std::deque<LiveDanmakuItem>> danmaku_data;

    void reset();
    void add(const std::vector<LiveDanmakuItem> &dan_l);
    void draw(NVGcontext *vg, float x, float y, float width, float height,
              float alpha);

    bool init_danmaku(NVGcontext *vg, LiveDanmakuItem &i, float width,
                      int LINES, float SECOND, float time);
};