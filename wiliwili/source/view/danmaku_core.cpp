//
// Created by fang on 2023/1/11.
//

#include <pystring.h>

#include "view/danmaku_core.hpp"
#include "view/mpv_core.hpp"
#include "utils/config_helper.hpp"

DanmakuItem::DanmakuItem(std::string content, const char *attributes)
    : msg(std::move(content)) {
#ifdef OPENCC
    static bool ZH_T = brls::Application::getLocale() == brls::LOCALE_ZH_HANT ||
                       brls::Application::getLocale() == brls::LOCALE_ZH_TW;
    if (ZH_T && brls::Label::OPENCC_ON) msg = brls::Label::STConverter(msg);
#endif
    auto attrs = pystring::split(attributes, ",");
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
    danmakuMutex.unlock();
}

void DanmakuCore::loadDanmakuData(const std::vector<DanmakuItem> &data) {
    danmakuMutex.lock();
    this->danmakuData = data;
    if (!data.empty()) danmakuLoaded = true;
    std::sort(danmakuData.begin(), danmakuData.end());
    danmakuMutex.unlock();

    // 通过mpv来通知弹幕加载完成
    MPVCore::instance().getEvent()->fire(MpvEventEnum::DANMAKU_LOADED);
}

void DanmakuCore::addSingleDanmaku(const DanmakuItem &item) {
    danmakuMutex.lock();
    this->danmakuData.emplace_back(item);
    danmakuMutex.unlock();

    // 通过mpv来通知弹幕加载完成
    MPVCore::instance().getEvent()->fire(MpvEventEnum::DANMAKU_LOADED);
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
    lineHeight = DANMAKU_STYLE_FONTSIZE * 1.2;

    // 更新弹幕透明度
    for (auto &d : danmakuData) {
        d.color.a       = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01;
        d.borderColor.a = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.005;
    }

    // 重置弹幕每行的时间信息
    for (int k = 0; k < lineNum; k++) {
        scrollLines[k].first  = 0;
        scrollLines[k].second = 0;
        centerLines[k]        = 0;
    }
    danmakuMutex.unlock();
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

void DanmakuCore::drawDanmaku(NVGcontext *vg, float x, float y, float width,
                              float height, float alpha) {
    if (!DanmakuCore::DANMAKU_ON) return;
    if (!this->danmakuLoaded) return;

    float SECOND        = 0.12f * DANMAKU_STYLE_SPEED;
    float CENTER_SECOND = 0.04f * DANMAKU_STYLE_SPEED;
    if (danmakuData.empty()) {
        refresh();
        danmakuData = getDanmakuData();
    }

    // Enable scissoring
    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);

    nvgFontSize(vg, DANMAKU_STYLE_FONTSIZE);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, this->danmakuFont);
    nvgTextLineHeight(vg, 1);

    int LINES = height / lineHeight;
    LINES *= DANMAKU_STYLE_AREA * 0.01;

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
            if (MPVCore::instance().core_idle) {
                // 暂停状态弹幕也要暂停
                position    = i.speed * videoSpeed * (playbackTime - i.time);
                i.startTime = currentTime - (playbackTime - i.time) * 1e6;
            } else {
                position =
                    i.speed * videoSpeed * (currentTime - i.startTime) / 1e6;
            }

            // 根据时间或位置判断是否显示弹幕
            if (position > width + i.length || i.time + SECOND < playbackTime) {
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
            if (i.level < DANMAKU_FILTER_LEVEL) continue;

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
                    i.line                = k;
                    scrollLines[k].first  = i.time + i.length / i.speed;
                    scrollLines[k].second = i.time + SECOND;
                    i.canShow             = true;
                    i.startTime           = brls::getCPUTimeUsec();
                    if (playbackTime - i.time > 0.2)
                        i.startTime -= (playbackTime - i.time) * 1e6;
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