//
// Created by fang on 2023/1/11.
//

#include <pystring.h>
#include <atomic>
#include <cstdlib>
#include <utility>

#include "view/danmaku_core.hpp"
#include "view/mpv_core.hpp"
#include "utils/config_helper.hpp"

struct DanmakuItem_node {
    DanmakuItem item;
    DanmakuItem_node *next;
};

DanmakuItem::DanmakuItem(std::string content, const char *attributes)
    : msg(std::move(content)) {
#ifdef OPENCC
    static bool ZH_T = brls::Application::getLocale() == brls::LOCALE_ZH_HANT ||
                       brls::Application::getLocale() == brls::LOCALE_ZH_TW;
    if (ZH_T && brls::Label::OPENCC_ON) msg = brls::Label::STConverter(msg);
#endif
    std::vector<std::string> attrs;
    pystring::split(attributes, attrs, ",");
    if (attrs.size() < 9) {
        brls::Logger::error("error decode danmaku: {} {}", msg, attributes);
        type = -1;
        return;
    }
    time      = atof(attrs[0].c_str());
    type      = atoi(attrs[1].c_str());
    fontSize  = atoi(attrs[2].c_str());
    fontColor = atoi(attrs[3].c_str());
    level     = atoi(attrs[8].c_str());
    is_live   = 0;

    int r          = (fontColor >> 16) & 0xff;
    int g          = (fontColor >> 8) & 0xff;
    int b          = fontColor & 0xff;
    isDefaultColor = (r & g & b) == 0xff;
    color          = nvgRGB(r, g, b);
    color.a        = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01;
    borderColor.a  = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.005;

    // 判断是否添加浅色边框
    if ((r * 299 + g * 587 + b * 114) < 60000) {
        borderColor   = nvgRGB(255, 255, 255);
        borderColor.a = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.5;
    }
}

DanmakuItem::DanmakuItem(const danmaku_t *dan) {
    msg = std::string(dan->dan);

#ifdef OPENCC
    static bool ZH_T = brls::Application::getLocale() == brls::LOCALE_ZH_HANT ||
                       brls::Application::getLocale() == brls::LOCALE_ZH_TW;
    if (ZH_T && brls::Label::OPENCC_ON) msg = brls::Label::STConverter(msg);
#endif

    canShow = false;

    time      = 0.0f;
    type      = dan->dan_type;
    fontSize  = dan->dan_size;
    fontColor = dan->dan_color;
    level     = dan->user_level;
    is_live   = 1;

    int r          = (fontColor >> 16) & 0xff;
    int g          = (fontColor >> 8) & 0xff;
    int b          = fontColor & 0xff;
    isDefaultColor = (r & g & b) == 0xff;
    color          = nvgRGB(r, g, b);
    color.a        = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01;
    borderColor.a  = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.005;

    // 判断是否添加浅色边框
    if ((r * 299 + g * 587 + b * 114) < 60000) {
        borderColor   = nvgRGB(255, 255, 255);
        borderColor.a = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.5;
    }
}

void DanmakuCore::reset() {
    danmakuMutex.lock();
    lineNum     = 20;
    scrollLines = std::vector<std::pair<float, float>>(20, {0, 0});
    centerLines = std::vector<float>(20, {0});
    this->danmakuData.clear();
    this->danmakuLoaded = false;
    danmakuIndex        = 0;
    videoSpeed          = MPVCore::instance().getSpeed();
    lineHeight     = DANMAKU_STYLE_FONTSIZE * DANMAKU_STYLE_LINE_HEIGHT * 0.01f;
    lineNumCurrent = 0;
    danmakuMutex.unlock();
}

void DanmakuCore::loadDanmakuData(const std::vector<DanmakuItem> &data) {
    danmakuMutex.lock();
    this->danmakuData = data;
    if (!data.empty()) danmakuLoaded = true;
    std::sort(danmakuData.begin(), danmakuData.end());
    danmakuMutex.unlock();

    // 通过mpv来通知弹幕加载完成
    MPVCore::instance().getCustomEvent()->fire("DANMAKU_LOADED", nullptr);
}

void DanmakuCore::addSingleDanmaku(const DanmakuItem &item) {
    danmakuMutex.lock();
    this->danmakuData.emplace_back(item);
    this->danmakuLoaded = true;
    danmakuMutex.unlock();

    // 通过mpv来通知弹幕加载完成
    MPVCore::instance().getCustomEvent()->fire("DANMAKU_LOADED", nullptr);
}

#define is_live_mode _is_live_mode.load(std::memory_order_acquire)
void DanmakuCore::addLiveDanmaku(DanmakuItem &&item) {
    if (!is_live_mode) return;
    auto t = tail.load(std::memory_order_acquire);
    if (t == nullptr) {
        head.store(new DanmakuItem_node{std::move(item), nullptr},
                   std::memory_order_release);
        tail.store(head.load(std::memory_order_acquire),
                   std::memory_order_release);
        return;
    }
    t->next = new DanmakuItem_node{std::move(item), nullptr};
    tail.store(t->next, std::memory_order_release);
}

void DanmakuCore::refresh() {
    danmakuMutex.lock();

    // 获取视频播放速度
    videoSpeed = MPVCore::instance().getSpeed();

    // 将当前屏幕第一条弹幕序号设为0
    danmakuIndex = 0;

    // 重置弹幕控制显示的信息
    for (auto &i : danmakuData) {
        i.showing = false;
        i.canShow = true;
    }

    // 重新设置最大显示的行数
    lineNum = brls::Application::windowHeight / DANMAKU_STYLE_FONTSIZE;
    while (scrollLines.size() < lineNum) {
        scrollLines.emplace_back(0, 0);
        centerLines.emplace_back(0);
    }

    // 重新设置行高
    lineHeight     = DANMAKU_STYLE_FONTSIZE * DANMAKU_STYLE_LINE_HEIGHT * 0.01f;
    lineNumCurrent = 0;

    // 更新弹幕透明度
    for (auto &d : danmakuData) {
        d.color.a       = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01;
        d.borderColor.a = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.005;
    }

    // 重置弹幕每行的时间信息
    for (size_t k = 0; k < lineNum; k++) {
        scrollLines[k].first  = 0;
        scrollLines[k].second = 0;
        centerLines[k]        = 0;
    }
    danmakuMutex.unlock();
}

void DanmakuCore::setSpeed(double speed) {
    double oldSpeed     = videoSpeed;
    videoSpeed          = speed;
    int64_t currentTime = brls::getCPUTimeUsec();
    double factor       = oldSpeed / speed;
    // 修改滚动弹幕的起始播放时间，满足修改后的时间在新速度下生成的位置不变。
    for (size_t j = this->danmakuIndex; j < this->danmakuData.size(); j++) {
        auto &i = this->danmakuData[j];
        if (i.type == 4 || i.type == 5) continue;
        if (!i.canShow) continue;
        i.startTime = currentTime - (currentTime - i.startTime) * factor;
    }
}

void DanmakuCore::save() {
    ProgramConfig::instance().setSettingItem(SettingItem::DANMAKU_ON,
                                             DANMAKU_ON, false);
    ProgramConfig::instance().setSettingItem(SettingItem::DANMAKU_FILTER_TOP,
                                             DANMAKU_FILTER_SHOW_TOP, false);
    ProgramConfig::instance().setSettingItem(SettingItem::DANMAKU_FILTER_BOTTOM,
                                             DANMAKU_FILTER_SHOW_BOTTOM, false);
    ProgramConfig::instance().setSettingItem(SettingItem::DANMAKU_FILTER_SCROLL,
                                             DANMAKU_FILTER_SHOW_SCROLL, false);
    ProgramConfig::instance().setSettingItem(SettingItem::DANMAKU_FILTER_COLOR,
                                             DANMAKU_FILTER_SHOW_COLOR, false);
    ProgramConfig::instance().setSettingItem(SettingItem::DANMAKU_FILTER_LEVEL,
                                             DANMAKU_FILTER_LEVEL, false);
    ProgramConfig::instance().setSettingItem(SettingItem::DANMAKU_STYLE_AREA,
                                             DANMAKU_STYLE_AREA, false);
    ProgramConfig::instance().setSettingItem(
        SettingItem::DANMAKU_STYLE_FONTSIZE, DANMAKU_STYLE_FONTSIZE, false);
    ProgramConfig::instance().setSettingItem(
        SettingItem::DANMAKU_STYLE_LINE_HEIGHT, DANMAKU_STYLE_LINE_HEIGHT,
        false);
    ProgramConfig::instance().setSettingItem(SettingItem::DANMAKU_STYLE_SPEED,
                                             DANMAKU_STYLE_SPEED, false);
    ProgramConfig::instance().setSettingItem(SettingItem::DANMAKU_STYLE_ALPHA,
                                             DANMAKU_STYLE_ALPHA, false);
    ProgramConfig::instance().save();
}

std::vector<DanmakuItem> DanmakuCore::getDanmakuData() {
    danmakuMutex.lock();
    std::vector<DanmakuItem> data = danmakuData;
    danmakuMutex.unlock();
    return data;
}

NVGcolor DanmakuCore::a(NVGcolor color, float alpha) {
    color.a *= alpha;
    return color;
}

void DanmakuCore::live_danmaku_reset() {
    auto h = head.load(std::memory_order_acquire);
    auto t = tail.load(std::memory_order_acquire);
    head.store(nullptr, std::memory_order_release);
    tail.store(nullptr, std::memory_order_release);
    danmakuMutex.lock();
    while (h != nullptr) {
        auto _p = h;
        h       = h->next;
        delete _p;
    }
    danmakuMutex.unlock();
}

void DanmakuCore::draw_live_danmaku(NVGcontext *vg, float x, float y,
                                    float width, float height, float alpha) {
    float SECOND        = 0.12f * DANMAKU_STYLE_SPEED;
    float CENTER_SECOND = 0.04f * DANMAKU_STYLE_SPEED;

    // Enable scissoring
    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);

    nvgFontSize(vg, DANMAKU_STYLE_FONTSIZE);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, this->danmakuFont);
    nvgTextLineHeight(vg, 1);

    int LINES = lineNumCurrent;
    if (LINES == 0) {
        LINES = height / lineHeight * DANMAKU_STYLE_AREA * 0.01;
    }

    float back_time = MPVCore::instance().getPlaybackTime();
    float _time     = 0.0f;
    float bounds[4];
    auto _head = head.load(std::memory_order_acquire);

    auto can_show = [&](DanmakuItem &i, const float time,
                        const float position) -> bool {
        if (!i.canShow) {
            i.time = back_time + _time;
            _time += 0.1f;
            if (_time >= 0.2f * LINES * 2) {
                _time = 0.0f;
            }
            return true;
        }

        if (time > CENTER_SECOND) {
            // 底部或顶部弹幕
            if (i.type == 4) {
                centerLines[LINES - i.line - 1] = 0;
                return false;
            } else if (i.type == 5) {
                centerLines[i.line] = 0;
                return false;
            }
        }
        // 1. 过滤显示的弹幕级别
        if (i.level < DANMAKU_FILTER_LEVEL_LIVE) return false;

        if (i.type == 4 && !DANMAKU_FILTER_SHOW_BOTTOM) return false;

        if (i.type == 5 && !DANMAKU_FILTER_SHOW_TOP) return false;

        if (i.type != 4 && i.type != 5 && !DANMAKU_FILTER_SHOW_SCROLL)
            return false;

        // 5. 过滤彩色弹幕
        if (!i.isDefaultColor && !DANMAKU_FILTER_SHOW_COLOR) return false;

        // 6. 过滤失效弹幕
        if (i.type < 0) return false;

        // 根据位置判断是否显示弹幕
        if (position > width + (i.length * i.length)) return false;

        return true;
    };

    danmakuMutex.lock();
    while (_head && is_live_mode) {
        DanmakuItem &i = _head->item;
        //滑动弹幕位置
        float position = i.speed * (back_time - i.time);

        // 溢出屏幕外或被过滤调不展示的弹幕
        if (!can_show(i, (back_time - i.time), position)) {
            auto _p = _head;
            if (_head == tail.load(std::memory_order_acquire)) {
                head.store(nullptr, std::memory_order_release);
                tail.store(nullptr, std::memory_order_release);
                _head = _head->next;
            } else {
                _head = _head->next;
                head.store(_head, std::memory_order_release);
            }
            delete _p;
            continue;
        };

        // 添加即将出现的弹幕
        if (!i.canShow) {
            /// 处理即将要显示的弹幕
            nvgTextBounds(vg, 0, 0, i.msg.c_str(), nullptr, bounds);
            i.length = bounds[2] - bounds[0];
            i.speed  = (width + i.length) / SECOND;
            for (int k = 0; k < LINES; ++k) {
                if (i.type == 4) {
                    //底部
                    if (centerLines[LINES - k - 1]) continue;

                    centerLines[LINES - k - 1] = 1;
                    i.line                     = LINES - k - 1;
                    i.canShow                  = true;
                    break;
                } else if (i.type == 5) {
                    //顶部
                    if (centerLines[k]) continue;

                    centerLines[k] = 1;
                    i.line         = k;
                    i.canShow      = true;
                    break;
                } else {
                    //滚动
                    if (i.time < scrollLines[k].first ||
                        i.time + width / i.speed < scrollLines[k].second)
                        continue;
                    // 一条弹幕完全展示的时间点，同一行的其他弹幕需要在这之后出现
                    scrollLines[k].first = i.time + i.length / i.speed;
                    // 一条弹幕展示结束的时间点，同一行的其他弹幕到达屏幕左侧的时间应该在这之后。
                    scrollLines[k].second = i.time + SECOND;
                    i.line                = k;
                    i.canShow             = true;
                    break;
                }
            }
            _head = _head->next;
            continue;
        }

        //居中弹幕
        if (i.type == 4 || i.type == 5) {
            // 画弹幕文字包边
            nvgFillColor(vg, a(i.borderColor, alpha));
            nvgText(vg, x + width / 2 - i.length / 2 + 1,
                    y + i.line * lineHeight + 6, i.msg.c_str(), nullptr);

            // 画弹幕文字
            nvgFillColor(vg, a(i.color, alpha));
            nvgText(vg, x + width / 2 - i.length / 2,
                    y + i.line * lineHeight + 5, i.msg.c_str(), nullptr);

            _head = _head->next;
            continue;
        }

        if (position < 0) {
            _head = _head->next;
            continue;
        }

        // 画弹幕文字包边
        nvgFillColor(vg, a(i.borderColor, alpha));
        nvgText(vg, x + width - position + 1, y + i.line * lineHeight + 6,
                i.msg.c_str(), nullptr);

        // 画弹幕文字
        nvgFillColor(vg, a(i.color, alpha));
        nvgText(vg, x + width - position, y + i.line * lineHeight + 5,
                i.msg.c_str(), nullptr);

        _head = _head->next;
    }
    danmakuMutex.unlock();
    nvgRestore(vg);
}

void DanmakuCore::drawDanmaku(NVGcontext *vg, float x, float y, float width,
                              float height, float alpha) {
    if (!DanmakuCore::DANMAKU_ON) return;
    if (is_live_mode) {
        draw_live_danmaku(vg, x, y, width, height, alpha);
        return;
    }
    if (!this->danmakuLoaded) return;
    if (danmakuData.empty()) return;

    float SECOND        = 0.12f * DANMAKU_STYLE_SPEED;
    float CENTER_SECOND = 0.04f * DANMAKU_STYLE_SPEED;

    // Enable scissoring
    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);

    nvgFontSize(vg, DANMAKU_STYLE_FONTSIZE);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, this->danmakuFont);
    nvgTextLineHeight(vg, 1);

    int LINES = lineNumCurrent;
    if (LINES == 0) {
        LINES = height / lineHeight * DANMAKU_STYLE_AREA * 0.01;
    }

    //取出需要的弹幕
    int64_t currentTime = brls::getCPUTimeUsec();
    double playbackTime = MPVCore::instance().playback_time;
    float bounds[4];
    for (size_t j = this->danmakuIndex; j < this->danmakuData.size(); j++) {
        auto &i = this->danmakuData[j];
        // 溢出屏幕外或被过滤调不展示的弹幕
        if (!i.canShow) continue;

        // 正在展示中的弹幕
        if (i.showing) {
            if (i.type == 4 || i.type == 5) {
                //居中弹幕
                // 根据时间判断是否显示弹幕
                if (i.time > playbackTime ||
                    i.time + CENTER_SECOND < playbackTime) {
                    i.canShow = false;
                    continue;
                }

                // 画弹幕文字包边
                nvgFillColor(vg, a(i.borderColor, alpha));
                nvgText(vg, x + width / 2 - i.length / 2 + 1,
                        y + i.line * lineHeight + 6, i.msg.c_str(), nullptr);

                // 画弹幕文字
                nvgFillColor(vg, a(i.color, alpha));
                nvgText(vg, x + width / 2 - i.length / 2,
                        y + i.line * lineHeight + 5, i.msg.c_str(), nullptr);

                continue;
            }
            //滑动弹幕
            float position = 0;
            if (!i.is_live && MPVCore::instance().isPaused()) {
                // 暂停状态弹幕也要暂停
                position = i.speed * (playbackTime - i.time);
                i.startTime =
                    currentTime - (playbackTime - i.time) / videoSpeed * 1e6;
            } else {
                position =
                    i.speed * (currentTime - i.startTime) * videoSpeed / 1e6;
            }

            // 根据位置判断是否显示弹幕
            if (position > width + i.length) {
                i.showing    = false;
                danmakuIndex = j + 1;
                continue;
            }

            // 画弹幕文字包边
            nvgFillColor(vg, a(i.borderColor, alpha));
            nvgText(vg, x + width - position + 1, y + i.line * lineHeight + 6,
                    i.msg.c_str(), nullptr);

            // 画弹幕文字
            nvgFillColor(vg, a(i.color, alpha));
            nvgText(vg, x + width - position, y + i.line * lineHeight + 5,
                    i.msg.c_str(), nullptr);
            continue;
        }

        // 添加即将出现的弹幕
        if (i.time < playbackTime) {
            // 排除已经应该暂停显示的弹幕
            if (i.type == 4 || i.type == 5) {
                // 底部或顶部弹幕
                if (i.time + CENTER_SECOND < playbackTime) {
                    continue;
                }
            } else if (i.time + SECOND < playbackTime) {
                // 滚动弹幕
                danmakuIndex = j + 1;
                continue;
            }

            /// 过滤弹幕
            i.canShow = false;
            // 1. 过滤显示的弹幕级别
            if (!i.is_live && i.level < DANMAKU_FILTER_LEVEL) continue;
            if (i.is_live && i.level < DANMAKU_FILTER_LEVEL_LIVE) continue;

            if (i.type == 4) {
                // 2. 过滤底部弹幕
                if (!DANMAKU_FILTER_SHOW_BOTTOM) continue;
            } else if (i.type == 5) {
                // 3. 过滤顶部弹幕
                if (!DANMAKU_FILTER_SHOW_TOP) continue;
            } else {  // 4. 过滤滚动弹幕
                if (!DANMAKU_FILTER_SHOW_SCROLL) continue;
            }

            // 5. 过滤彩色弹幕
            if (!i.isDefaultColor && !DANMAKU_FILTER_SHOW_COLOR) continue;

            // 6. 过滤失效弹幕
            if (i.type < 0) continue;

            /// 处理即将要显示的弹幕
            nvgTextBounds(vg, 0, 0, i.msg.c_str(), nullptr, bounds);
            i.length  = bounds[2] - bounds[0];
            i.speed   = (width + i.length) / SECOND;
            i.showing = true;
            for (int k = 0; k < LINES; k++) {
                if (i.type == 4) {
                    //底部
                    if (i.time < centerLines[LINES - k - 1]) continue;

                    i.line                     = LINES - k - 1;
                    centerLines[LINES - k - 1] = i.time + CENTER_SECOND;
                    i.canShow                  = true;
                    break;
                } else if (i.type == 5) {
                    //顶部
                    if (i.time < centerLines[k]) continue;

                    i.line         = k;
                    centerLines[k] = i.time + CENTER_SECOND;
                    i.canShow      = true;
                    break;
                } else {
                    //滚动
                    if (i.time < scrollLines[k].first ||
                        i.time + width / i.speed < scrollLines[k].second)
                        continue;
                    i.line = k;
                    // 一条弹幕完全展示的时间点，同一行的其他弹幕需要在这之后出现
                    scrollLines[k].first = i.time + i.length / i.speed;
                    // 一条弹幕展示结束的时间点，同一行的其他弹幕到达屏幕左侧的时间应该在这之后。
                    scrollLines[k].second = i.time + SECOND;
                    i.canShow             = true;
                    i.startTime           = currentTime;
                    // 如果当前时间点弹幕已经出现在屏幕上了，那么反向推算出弹幕开始的现实时间
                    if (playbackTime - i.time > 0.2)
                        i.startTime -=
                            (playbackTime - i.time) / videoSpeed * 1e6;
                    break;
                }
            }
            // 循环之外的弹幕因为 canShow 为 false 所以不会显示
        } else {
            // 当前没有需要显示或等待显示的弹幕，结束循环
            break;
        }
    }
    nvgRestore(vg);
}