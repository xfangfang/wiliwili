//
// Created by fang on 2023/1/11.
//

#include <borealis/core/logger.hpp>

#include <pystring.h>
#include <cstdlib>
#include <utility>
#include <lunasvg.h>

#include "view/danmaku_core.hpp"
#include "utils/config_helper.hpp"
#include "utils/string_helper.hpp"

// include ntohl / ntohll
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#if defined(__linux__)
#include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/endian.h>
#elif defined(__OpenBSD__)
#include <sys/types.h>
#endif
#endif
#ifdef __SWITCH__
static inline uint64_t ntohll(uint64_t netlonglong) {
    return __builtin_bswap64(netlonglong);
}
#elif defined(__WINRT__)
#elif defined(betoh64)
#define ntohll betoh64
#elif defined(be64toh)
#define ntohll be64toh
#elif !defined(ntohll)
static inline uint64_t ntohll(uint64_t value) {
    if (ntohl(1) == 1) {
        // The system is big endian, no conversion is needed
        return value;
    } else {
        // The system is little endian, convert from network byte order (big endian) to host byte order
        const uint32_t high_part = ntohl(static_cast<uint32_t>(value >> 32));
        const uint32_t low_part =
            ntohl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));
        return (static_cast<uint64_t>(low_part) << 32) | high_part;
    }
}
#endif

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
        borderColor.a = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.005;
    }
}

DanmakuCore::DanmakuCore() {
    event_id = MPV_E->subscribe([this](MpvEventEnum e) {
        if (e == MpvEventEnum::LOADING_END) {
            this->refresh();
        } else if (e == MpvEventEnum::RESET) {
            this->reset();
        } else if (e == MpvEventEnum::VIDEO_SPEED_CHANGE) {
            this->setSpeed(MPVCore::instance().getSpeed());
        }
    });

    // 退出前清空遮罩纹理
    brls::Application::getExitDoneEvent()->subscribe([this]() {
        if (maskTex != 0) {
            nvgDeleteImage(brls::Application::getNVGContext(), maskTex);
            maskTex = 0;
        }
    });
}

DanmakuCore::~DanmakuCore() { MPV_E->unsubscribe(event_id); }

void DanmakuCore::reset() {
    danmakuMutex.lock();
    lineNum     = 20;
    scrollLines = std::vector<std::pair<float, float>>(20, {0, 0});
    centerLines = std::vector<float>(20, {0});
    this->danmakuData.clear();
    this->danmakuLoaded = false;
    danmakuIndex        = 0;
    maskIndex           = 0;
    maskSliceIndex      = 0;
    videoSpeed          = MPVCore::instance().getSpeed();
    lineHeight     = DANMAKU_STYLE_FONTSIZE * DANMAKU_STYLE_LINE_HEIGHT * 0.01f;
    lineNumCurrent = 0;
    maskData.clear();
    if (maskTex != 0) {
        nvgDeleteImage(brls::Application::getNVGContext(), maskTex);
        maskTex = 0;
    }
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

void DanmakuCore::loadMaskData(const WebMask &data) { maskData = data; }

void DanmakuCore::refresh() {
    danmakuMutex.lock();

    // 获取视频播放速度
    videoSpeed = MPVCore::instance().getSpeed();

    // 将当前屏幕第一条弹幕序号设为0
    danmakuIndex = 0;

    // 将遮罩序号设为0
    maskSliceIndex = 0;
    maskIndex      = 0;

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

// Uncomment this line to show mask in a more obvious way.
//#define DEBUG_MASK

void DanmakuCore::draw(NVGcontext *vg, float x, float y, float width,
                       float height, float alpha) {
    if (!DanmakuCore::DANMAKU_ON) return;
    if (!this->danmakuLoaded) return;
    if (danmakuData.empty()) return;

    int64_t currentTime = brls::getCPUTimeUsec();
    double playbackTime = MPVCore::instance().playback_time;
    float SECOND        = 0.12f * DANMAKU_STYLE_SPEED;
    float CENTER_SECOND = 0.04f * DANMAKU_STYLE_SPEED;

    // Enable scissoring
    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);

    // 设置遮罩
#ifdef BOREALIS_USE_OPENGL
    if (!maskData.sliceData.empty()) {
        // 先根据时间选择分片
        while (maskSliceIndex < maskData.sliceData.size()) {
            auto &slice = maskData.sliceData[maskSliceIndex + 1];
            if (slice.time > playbackTime * 1000) break;
            maskSliceIndex++;
            maskIndex = 0;
        }

        // 在分片内选择对应时间的svg
        // todo: 优化为异步加载 (包括下载和解压分片)
        if (maskSliceIndex >= maskData.sliceData.size()) goto skip_mask;
        auto &slice = maskData.getSlice(maskSliceIndex);
        while (maskIndex < slice.svgData.size()) {
            auto &svg = slice.svgData[maskIndex];
            if (svg.showTime > playbackTime * 1000) break;
            maskIndex++;
        }
        if (maskIndex == 0) maskIndex = 1;

        // 设置 svg
        if (maskIndex > slice.svgData.size()) goto skip_mask;
        auto &svg = slice.svgData[maskIndex - 1];
        // 给图片添加一圈边框（避免图片边沿为透明时自动扩展了透明色导致非视频区域无法显示弹幕）
        // 注：返回的 svg 底部固定留有 2像素 透明，不是很清楚具体作用，这里选择绘制一个2像素宽的空心矩形来覆盖
        const std::string border =
            R"xml(<rect x="0" y="0" width="100%" height="100%" fill="none" stroke="#000" stroke-width="2"/></svg>)xml";
        auto maskDocument = lunasvg::Document::loadFromData(
            pystring::slice(svg.svg, 0, pystring::rindex(svg.svg, "</svg>")) +
            border);
        if (maskDocument == nullptr) goto skip_mask;
        auto bitmap        = maskDocument->renderToBitmap(maskDocument->width(),
                                                          maskDocument->height());
        uint32_t maskWidth = bitmap.width();
        uint32_t maskHeight = bitmap.height();
        if (maskTex != 0) {
            nvgUpdateImage(vg, maskTex, bitmap.data());
        } else {
            maskTex = nvgCreateImageRGBA(vg, (int)maskWidth, (int)maskHeight,
                                         NVG_IMAGE_NEAREST, bitmap.data());
        }

        // 设置遮罩
        nvgBeginPath(vg);
        float drawHeight = height, drawWidth = width;
        float drawX = x, drawY = y;
        if (maskWidth * height > maskHeight * width) {
            drawHeight = maskHeight * width / maskWidth;
            drawY      = y + (height - drawHeight) / 2;
        } else {
            drawWidth = maskWidth * height / maskHeight;
            drawX     = x + (width - drawWidth) / 2;
        }
        auto paint = nvgImagePattern(vg, drawX, drawY, drawWidth, drawHeight, 0,
                                     maskTex, alpha);
        nvgRect(vg, x, y, width, height);
        nvgFillPaint(vg, paint);
#if defined(DEBUG_MASK)
        nvgFill(vg);
#else
        nvgStencil(vg);
#endif
    }
#endif /* BOREALIS_USE_OPENGL */
skip_mask:

    // 设置基础字体
    nvgFontSize(vg, DANMAKU_STYLE_FONTSIZE);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, this->danmakuFont);
    nvgTextLineHeight(vg, 1);

    int LINES = lineNumCurrent;
    if (LINES == 0) {
        LINES = height / lineHeight * DANMAKU_STYLE_AREA * 0.01;
    }

    //取出需要的弹幕
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
            if (MPVCore::instance().isPaused()) {
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

    // 清空遮罩
#if !defined(DEBUG_MASK) && defined(BOREALIS_USE_OPENGL)
    if (maskTex > 0) {
        nvgBeginPath(vg);
        nvgRect(vg, x, y, width, height);
        nvgStencilClear(vg);
    }
#endif
    nvgRestore(vg);
}

void WebMask::parse(const std::string &text) {
    rawData = text;

    // 检查头部
    std::memcpy(&version, rawData.data() + 4, sizeof(int32_t));
    std::memcpy(&check, rawData.data() + 8, sizeof(int32_t));
    std::memcpy(&length, rawData.data() + 12, sizeof(int32_t));
    version = ntohl(version);
    check   = ntohl(check);
    length  = ntohl(length);

    // 获取所有分片信息
    sliceData.reserve(length + 1);
    int64_t time, offset, currentOffset = 16;
    for (size_t i = 0; i < length; i++) {
        std::memcpy(&time, rawData.data() + currentOffset, sizeof(int64_t));
        std::memcpy(&offset, rawData.data() + currentOffset + 8,
                    sizeof(int64_t));
        time   = ntohll(time);
        offset = ntohll(offset);
        sliceData.emplace_back(time, offset, 0);
        if (i != 0) sliceData[i - 1].offsetEnd = offset;
        if (i == length - 1) sliceData[i].offsetEnd = rawData.size();
        currentOffset += 16;
    }
    // 再添加一个尾部分片方便计算
    sliceData.emplace_back(-1, -1, -1);
}

void WebMask::clear() {
    this->rawData.clear();
    this->sliceData.clear();
}

const MaskSlice &WebMask::getSlice(size_t index) {
    auto &slice = sliceData[index];
    if (!slice.svgData.empty()) return slice;

    // 解压分片数据
    std::string data;
    try {
        data = wiliwili::decompressGzipData(rawData.substr(
            slice.offsetStart, slice.offsetEnd - slice.offsetStart));
    } catch (const std::runtime_error &e) {
        brls::Logger::error("web mask decompress error: {}", e.what());
        return slice;
    }

    // 获取svg数据
    size_t sliceOffset = 0;
    uint32_t sliceLength, sliceTime;
    while (sliceOffset < data.size()) {
        std::memcpy(&sliceLength, data.data() + sliceOffset, sizeof(int32_t));
        std::memcpy(&sliceTime, data.data() + sliceOffset + 8, sizeof(int32_t));
        sliceLength = ntohl(sliceLength);
        sliceTime   = ntohl(sliceTime);
        sliceOffset += 12;
        auto base64 = pystring::replace(
            pystring::split(data.substr(sliceOffset, sliceLength), ",", 1)[1],
            "\n", "");
        std::string svg;
        wiliwili::base64Decode(base64, svg);
        sliceOffset += sliceLength;

        slice.svgData.emplace_back(svg, sliceTime);
    }
    return slice;
}