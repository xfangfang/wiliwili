//
// Created by maye174 on 2023/11/13.
//

#include "view/live_core.hpp"
#include "nanovg.h"
#include "view/danmaku_core.hpp"
#include "view/mpv_core.hpp"

LiveDanmakuItem::LiveDanmakuItem(danmaku_t *dan) { this->danmaku = dan; }

LiveDanmakuItem::LiveDanmakuItem(const LiveDanmakuItem &item) {
    this->danmaku = danmaku_t_copy(item.danmaku);
    this->time    = item.time;
    this->length  = item.length;
    this->speed   = item.speed;
    this->line    = item.line;
}

LiveDanmakuItem::LiveDanmakuItem(LiveDanmakuItem &&item) {
    this->danmaku = item.danmaku;
    this->time    = item.time;
    this->length  = item.length;
    this->speed   = item.speed;
    this->line    = item.line;
    item.danmaku  = nullptr;
}

void LiveDanmakuCore::reset() {
    this->scroll_lines.clear();
    this->center_lines.clear();
    this->danmaku_data.clear();
    while (!this->danmaku_que.empty()) {
        this->que_mutex.lock();
        this->danmaku_que.pop();
        this->que_mutex.unlock();
    }
}

void LiveDanmakuCore::add(const std::vector<LiveDanmakuItem> &dan_l) {
    for (const auto &i : dan_l) {
        if (i.danmaku->dan_type == 4 &&
            !DanmakuCore::DANMAKU_FILTER_SHOW_BOTTOM)
            continue;
        else if (i.danmaku->dan_type == 5 &&
                 !DanmakuCore::DANMAKU_FILTER_SHOW_TOP)
            continue;
        else if (i.danmaku->dan_type != 4 && i.danmaku->dan_type != 5 &&
                 !DanmakuCore::DANMAKU_FILTER_SHOW_SCROLL)
            continue;
        if (i.danmaku->user_level < DANMAKU_FILTER_LEVEL_LIVE) continue;
        if (i.danmaku->dan_color != 0xffffff &&
            !DanmakuCore::DANMAKU_FILTER_SHOW_COLOR)
            continue;
        this->que_mutex.lock();
        this->danmaku_que.emplace(std::move(i));
        this->que_mutex.unlock();
    }
}

void LiveDanmakuCore::draw(NVGcontext *vg, float x, float y, float width,
                           float height, float alpha) {
    if (!LiveDanmakuCore::DANMAKU_ON) return;

    int r, g, b;
    float SECOND        = 0.12f * DanmakuCore::DANMAKU_STYLE_SPEED;
    float CENTER_SECOND = 0.04f * DanmakuCore::DANMAKU_STYLE_SPEED;
    line_height         = DanmakuCore::DANMAKU_STYLE_FONTSIZE *
                  DanmakuCore::DANMAKU_STYLE_LINE_HEIGHT * 0.01f;
    int LINES =
        height / this->line_height * DanmakuCore::DANMAKU_STYLE_AREA * 0.01;
    if (LINES < 1) LINES = 1;

    if (this->scroll_lines.size() < LINES) {
        this->scroll_lines.resize(LINES, {0.0f, 0.0f});
        this->center_lines.resize(LINES, 0);
    }

    // Enable scissoring
    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);

    nvgFontSize(vg, DanmakuCore::DANMAKU_STYLE_FONTSIZE);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, danmaku_font);
    nvgTextLineHeight(vg, 1);

    float _time = 0.0f;
    this->que_mutex.lock();
    while (!this->danmaku_que.empty() &&
           init_danmaku(vg, this->danmaku_que.front(), width, LINES, SECOND,
                        _time)) {
        const auto &i = danmaku_que.front();
        if (this->danmaku_data.find(i.danmaku->dan_color) ==
            this->danmaku_data.end())
            this->danmaku_data.emplace(i.danmaku->dan_color,
                                       std::deque<LiveDanmakuItem>{});
        this->danmaku_data[i.danmaku->dan_color].emplace_back(std::move(i));
        this->danmaku_que.pop();
        _time += 0.1f;
        if (_time > 0.1f * LINES) _time = 0.0f;
    }
    this->que_mutex.unlock();

    for (const auto &[i, v] : this->danmaku_data) {
        r                     = (i >> 16) & 0xff;
        g                     = (i >> 8) & 0xff;
        b                     = i & 0xff;
        NVGcolor color        = nvgRGB(r, g, b);
        color.a               = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01 * alpha;
        NVGcolor border_color = nvgRGBA(0, 0, 0, 160);
        border_color.a        = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.005;
        if ((r * 299 + g * 587 + b * 114) < 60000) {
            border_color   = nvgRGB(255, 255, 255);
            border_color.a = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.5;
        }
        nvgFillColor(vg, color);

        for (const auto &j : v) {
            float position =
                j.speed * (MPVCore::instance().getPlaybackTime() - j.time);
            if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                nvgText(vg, x + width / 2 - j.length / 2,
                        y + j.line * line_height + 5, j.danmaku->dan, nullptr);
            } else if (position > 0) {
                nvgText(vg, x + width - position, y + j.line * line_height + 5,
                        j.danmaku->dan, nullptr);
            }
        }

        nvgFillColor(vg, border_color);

        for (const auto &j : v) {
            float position =
                j.speed * (MPVCore::instance().getPlaybackTime() - j.time);
            if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                nvgText(vg, x + width / 2 - j.length / 2 - 1,
                        y + j.line * line_height + 6, j.danmaku->dan, nullptr);
            } else if (position > 0) {
                nvgText(vg, x + width - position - 1,
                        y + j.line * line_height + 6, j.danmaku->dan, nullptr);
            }
        }
    }
    nvgRestore(vg);

    for (auto &[i, v] : this->danmaku_data) {
        while (!v.empty()) {
            const auto &j = v.front();
            float position =
                j.speed * (MPVCore::instance().getPlaybackTime() - j.time);
            if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                if (j.time + CENTER_SECOND <
                    MPVCore::instance().getPlaybackTime()) {
                    center_lines[j.line] = 0;
                    v.pop_front();
                } else {
                    break;
                }
            } else if (position > width + j.length) {
                v.pop_front();
            } else {
                break;
            }
        }
    }
}

bool LiveDanmakuCore::init_danmaku(NVGcontext *vg, LiveDanmakuItem &i,
                                   float width, int LINES, float SECOND,
                                   float time) {
    float bounds[4];
    if (!i.length) {
        nvgTextBounds(vg, 0, 0, i.danmaku->dan, nullptr, bounds);
        i.length = bounds[2] - bounds[0];
    }
    i.speed = (width + i.length) / SECOND;
    i.time  = MPVCore::instance().getPlaybackTime() + time;

    for (int k = 0; k < LINES; ++k) {
        if (i.danmaku->dan_type == 4 && !center_lines[LINES - k - 1]) {
            //底部
            center_lines[LINES - k - 1] = 1;
            i.line                      = LINES - k - 1;
            return true;
        } else if (i.danmaku->dan_type == 5 && !center_lines[k]) {
            //顶部
            center_lines[k] = 1;
            i.line          = k;
            return true;
        } else if (i.time > scroll_lines[k].first &&
                   i.time + width / i.speed > scroll_lines[k].second) {
            //滚动
            // 一条弹幕完全展示的时间点，同一行的其他弹幕需要在这之后出现
            scroll_lines[k].first = i.time + i.length / i.speed;
            // 一条弹幕展示结束的时间点，同一行的其他弹幕到达屏幕左侧的时间应该在这之后。
            scroll_lines[k].second = i.time + SECOND;
            i.line                 = k;
            return true;
        }
    }
    return false;
}