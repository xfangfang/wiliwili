//
// Created by maye174 on 2023/11/13.
//

#pragma once

#include "api/live/extract_messages.hpp"
#include "live/dl_emoticon.hpp"

#include <chrono>
#include <cstddef>
#include <deque>
#include <map>
#include <mutex>
#include <memory>

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
    
    // 表情包映射
    std::shared_ptr<lmp> emoticons;

    void reset();
    void refresh();
    void add(const std::vector<LiveDanmakuItem> &dan_l);
    void draw(NVGcontext *vg, float x, float y, float width, float height, float alpha);
    
    // 设置表情包映射
    void setEmoticons(std::shared_ptr<lmp> emotes) { this->emoticons = emotes; }

    bool init_danmaku(NVGcontext *vg, LiveDanmakuItem &i, float width, int LINES, float SECOND, time_p now, int time);
    
    // 判断字符串是否是表情
    bool isEmoticon(const std::string& text) const;
    
    // 查找文本中所有表情的位置和长度
    std::vector<std::pair<size_t, size_t>> findEmoticons(const std::string& text) const;
    
    // 渲染表情
    void drawEmoticon(NVGcontext *vg, const std::string& name, float x, float y, float size, float alpha = 1.0f);
    
    // 清理表情缓存
    void clearEmoticonCache();
};