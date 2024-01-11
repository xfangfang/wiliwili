//
// Created by maye174 on 2023/11/13.
//

#include "view/live_core.hpp"
#include "view/danmaku_core.hpp"

#include <chrono>
#include <cstddef>

#include "nanovg.h"

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
    this->now.clear();
    this->next_mutex.lock();
    while (!this->next.empty()) {
        this->next.pop_front();
    }
    this->next_mutex.unlock();
}

void LiveDanmakuCore::add(const std::vector<LiveDanmakuItem> &dan_l) {
    for (const auto &i : dan_l) {
        if (i.danmaku->dan_type == 4 && !DanmakuCore::DANMAKU_FILTER_SHOW_BOTTOM)
            continue;
        else if (i.danmaku->dan_type == 5 && !DanmakuCore::DANMAKU_FILTER_SHOW_TOP)
            continue;
        else if (i.danmaku->dan_type != 4 && i.danmaku->dan_type != 5 && !DanmakuCore::DANMAKU_FILTER_SHOW_SCROLL)
            continue;
        if (i.danmaku->user_level < DANMAKU_FILTER_LEVEL_LIVE) continue;
        if (i.danmaku->dan_color != 0xffffff && !DanmakuCore::DANMAKU_FILTER_SHOW_COLOR) continue;
        this->next_mutex.lock();
        this->next.emplace_front(std::move(i));
        this->next_mutex.unlock();
    }
}

void LiveDanmakuCore::draw(NVGcontext *vg, float x, float y, float width, float height, float alpha) {
    if (!DanmakuCore::DANMAKU_ON) return;

    int r, g, b;
    float SECOND        = 0.12f * DanmakuCore::DANMAKU_STYLE_SPEED;
    float CENTER_SECOND = 0.04f * DanmakuCore::DANMAKU_STYLE_SPEED;
    line_height         = DanmakuCore::DANMAKU_STYLE_FONTSIZE * DanmakuCore::DANMAKU_STYLE_LINE_HEIGHT * 0.01f;
    size_t LINES        = height / this->line_height * DanmakuCore::DANMAKU_STYLE_AREA * 0.01;
    if (LINES < 1) LINES = 1;

    if (this->scroll_lines.size() < LINES) {
        this->scroll_lines.resize(LINES);
        this->center_lines.resize(LINES, 0);
    }

    // Enable scissoring
    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);

    nvgFontSize(vg, DanmakuCore::DANMAKU_STYLE_FONTSIZE);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, DanmakuCore::DANMAKU_FONT);
    nvgTextLineHeight(vg, 1);

    // 弹幕渲染质量
    if (DanmakuCore::DANMAKU_RENDER_QUALITY < 100) {
        nvgFontQuality(vg, 0.01f * DanmakuCore::DANMAKU_RENDER_QUALITY);
    }

    auto _now = std::chrono::system_clock::now();

    size_t _time = 0;
    this->next_mutex.lock();
    while (!this->next.empty() && init_danmaku(vg, this->next.front(), width, LINES, SECOND, _now, _time)) {
        const auto &i = next.front();
        if (this->now.find(i.danmaku->dan_color) == this->now.end())
            this->now.emplace(i.danmaku->dan_color, std::deque<LiveDanmakuItem>{});
        this->now[i.danmaku->dan_color].emplace_back(std::move(i));
        this->next.pop_front();
        _time += 80;
        if (_time > 80 * LINES) _time = 0;
    }
    while (this->next.size() > 100) {
        this->next.pop_back();
    }
    this->next_mutex.unlock();

    for (const auto &[i, v] : this->now) {
        r                     = (i >> 16) & 0xff;
        g                     = (i >> 8) & 0xff;
        b                     = i & 0xff;
        NVGcolor color        = nvgRGB(r, g, b);
        color.a               = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01 * alpha;
        NVGcolor border_color = nvgRGBA(0, 0, 0, DanmakuCore::DANMAKU_STYLE_ALPHA * 1.28 * alpha);
        if ((r * 299 + g * 587 + b * 114) < 60000) {
            border_color = nvgRGBA(255, 255, 255, DanmakuCore::DANMAKU_STYLE_ALPHA * 1.28 * alpha);
        }

        if (DanmakuCore::DANMAKU_STYLE_FONT != DanmakuFontStyle::DANMAKU_FONT_PURE) {
            float dx, dy;
            dx = dy = DanmakuCore::DANMAKU_STYLE_FONT == DanmakuFontStyle::DANMAKU_FONT_INCLINE;
            nvgFontBlur(vg, DanmakuCore::DANMAKU_STYLE_FONT == DanmakuFontStyle::DANMAKU_FONT_SHADOW);
            nvgFontDilate(vg, DanmakuCore::DANMAKU_STYLE_FONT == DanmakuFontStyle::DANMAKU_FONT_STROKE);

            nvgFillColor(vg, border_color);
            for (const auto &j : v) {
                float position = j.speed * std::chrono::duration<float>(_now - j.time).count();
                if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                    nvgText(vg, x + width / 2 - j.length / 2 + dx, y + j.line * line_height + 5 + dy, j.danmaku->dan,
                            nullptr);
                } else if (position > 0) {
                    nvgText(vg, x + width - position + dx, y + j.line * line_height + 5 + dy, j.danmaku->dan, nullptr);
                }
            }
            nvgFontBlur(vg, 0.0f);
            nvgFontDilate(vg, 0.0f);
        }

        nvgFillColor(vg, color);
        for (const auto &j : v) {
            float position = j.speed * std::chrono::duration<float>(_now - j.time).count();
            if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                nvgText(vg, x + width / 2 - j.length / 2, y + j.line * line_height + 5, j.danmaku->dan, nullptr);
            } else if (position > 0) {
                nvgText(vg, x + width - position, y + j.line * line_height + 5, j.danmaku->dan, nullptr);
            }
        }
    }
    nvgRestore(vg);

    for (auto &[i, v] : this->now) {
        while (!v.empty()) {
            const auto &j  = v.front();
            float position = j.speed * std::chrono::duration<float>(_now - j.time).count();
            if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                if (j.time + std::chrono::milliseconds(size_t(CENTER_SECOND * 1000.0f)) < _now) {
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

bool LiveDanmakuCore::init_danmaku(NVGcontext *vg, LiveDanmakuItem &i, float width, int LINES, float SECOND, time_p now,
                                   int time) {
    float bounds[4];
    if (!i.length) {
        nvgTextBounds(vg, 0, 0, i.danmaku->dan, nullptr, bounds);
        i.length = bounds[2] - bounds[0];
        if (!i.length) i.length = 1;
    }
    i.speed = (width + i.length) / SECOND;
    i.time  = now + std::chrono::milliseconds(time);

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
                   i.time + std::chrono::milliseconds(size_t(width / i.speed * 1000.0f)) > scroll_lines[k].second) {
            //滚动
            // 一条弹幕末尾出现的时间点
            scroll_lines[k].first = i.time + std::chrono::milliseconds(size_t(i.length / i.speed * 1000.0f));
            // 一条弹幕完全消失的时间点
            scroll_lines[k].second = i.time + std::chrono::milliseconds(size_t(SECOND * 1000.0f));
            i.line                 = k;
            return true;
        }
    }
    return false;
}