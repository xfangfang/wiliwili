//
// Created by fang on 2023/3/6.
//

#include <borealis/core/thread.hpp>

#include "bilibili.h"
#include "view/subtitle_core.hpp"
#include "view/mpv_core.hpp"

static NVGcolor a(NVGcolor color, float alpha) {
    color.a *= alpha;
    return color;
}

SubtitleCore::SubtitleCore() {
    event_id = MPV_E->subscribe([this](MpvEventEnum e) {
        if (e == MpvEventEnum::RESET) {
            this->reset();
        }
    });
}

SubtitleCore::~SubtitleCore() { MPV_E->unsubscribe(event_id); }

void SubtitleCore::setSubtitleList(const bilibili::VideoPageResult& data) {
    videoPageData = data;
    for (auto& i : data.subtitles) {
        brls::Logger::debug("{}: {}", i.lan_doc, i.subtitle_url);
    }
}

void SubtitleCore::reset() {
    this->videoPageData.subtitles.clear();
    this->videoPageData.last_play_cid  = 0;
    this->videoPageData.last_play_time = 0;
    this->videoPageData.online_count   = 0;
    this->subtitleIndex                = 0;
    this->clearSubtitle();
}

[[nodiscard]] bool SubtitleCore::isAvailable() const { return !videoPageData.subtitles.empty(); }

void SubtitleCore::drawSubtitle(NVGcontext* vg, float x, float y, float width, float height, float alpha) {
    double playbackTime = MPVCore::instance().playback_time;
    for (size_t i = subtitleIndex; i < currentSubtitle.data.body.size(); i++) {
        auto& line = currentSubtitle.data.body[i];
        // 当前还不能播放字幕
        if (line.from >= playbackTime) break;
        // 当前字幕播放结束，应该播放下一条
        if (line.to < playbackTime) continue;

        float fontSize               = 26;
        float borderV                = 6;
        float borderH                = 16;
        float bottomSpace            = 26;
        float backgroundCornerRadius = 2;

        // 初始化
        nvgFontSize(vg, fontSize);
        nvgTextAlign(vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
        nvgFontFaceId(vg, this->subtitleFont);
        nvgTextLineHeight(vg, 1);
        if (line.length <= 0) {
            float bounds[4];
            nvgTextBounds(vg, 0, 0, line.content.c_str(), nullptr, bounds);
            line.length = bounds[2] - bounds[0];
        }

        // 绘制字幕背景
        nvgBeginPath(vg);
        nvgFillColor(vg, a(backgroundColor, alpha));
        nvgRoundedRect(vg, x + (width - line.length) / 2 - borderH, y + height - borderV - fontSize - bottomSpace,
                       line.length + borderH * 2, fontSize + borderV * 2, backgroundCornerRadius);
        nvgFill(vg);

        // 绘制字幕
        nvgFillColor(vg, a(fontColor, alpha));
        nvgText(vg, x + (width - line.length) / 2, y + height - fontSize - bottomSpace, line.content.c_str(), nullptr);

        // 绘制AI标记
        if (!currentSubtitle.data.genByAI) break;
        nvgFontSize(vg, 8);
        nvgFillColor(vg, a(nvgRGBA(255, 255, 255, 127), alpha));
        nvgText(vg, x + (width + line.length) / 2 + 4, y + height - borderV - fontSize - bottomSpace + 4, "AI",
                nullptr);
        break;
    }
}

bilibili::VideoPageResult SubtitleCore::getSubtitleList() { return videoPageData; }

void SubtitleCore::selectSubtitle(size_t index) {
    if (index < 0 || index >= videoPageData.subtitles.size()) return;
    auto& page     = videoPageData.subtitles[index];
    std::string id = page.id_str;
    if (page.data.body.empty()) {
        // 下载字幕
        ASYNC_RETAIN
        BILI::get_subtitle(
            page.subtitle_url,
            [ASYNC_TOKEN, id](const bilibili::SubtitleData& result) {
                brls::sync([ASYNC_TOKEN, id, result]() {
                    ASYNC_RELEASE
                    for (auto& i : videoPageData.subtitles) {
                        if (i.id_str == id) {
                            i.data = result;
                            onLoadSubtitle(i);
                            return;
                        }
                    }
                });
            },
            [ASYNC_TOKEN](BILI_ERR) {
                ASYNC_RELEASE
                brls::Logger::error("请求字幕失败: {}", error);
            });

    } else {
        onLoadSubtitle(page);
    }
}

void SubtitleCore::onLoadSubtitle(const bilibili::VideoPageSubtitle& page) {
    currentSubtitle = page;
    brls::Logger::info("select subtitle: {}", page.lan_doc);
}

void SubtitleCore::clearSubtitle() { currentSubtitle = bilibili::VideoPageSubtitle{}; }

[[nodiscard]] std::string SubtitleCore::getCurrentSubtitleId() const { return currentSubtitle.id_str; }