//
// Created by fang on 2022/4/22.
//

#pragma once

#include <borealis.hpp>
#include "view/mpv_core.hpp"

class VideoProgressSlider;

class SVGImage;

// https://github.com/mpv-player/mpv/blob/master/DOCS/edl-mpv.rst
class EDLUrl {
public:
    std::string url;
    float length = -1;  // second

    EDLUrl(std::string url, float length = -1) : url(url), length(length) {}
};

class VideoView : public brls::Box {
public:
    VideoView();

    ~VideoView() override;

    /// Video control
    void setUrl(std::string url, int progress = 0, std::string audio = "");

    void setUrl(const std::vector<EDLUrl>& edl_urls, int progress = 0);

    void resume();

    void pause();

    void stop();

    void togglePlay();

    /// OSD
    void showOSD(bool temp = true);

    void hideOSD();

    bool isOSDShown();

    void showLoading();

    void hideLoading();

    void hideDanmakuButton();

    void setTitle(std::string title);

    std::string getTitle();

    void setOnlineCount(std::string count);

    void setDuration(std::string value);

    void setPlaybackTime(std::string value);

    // 手动设置osd右下角的全屏图标
    void setFullscreenIcon(bool fs);

    // 手动刷新osd左下角的播放图标
    void refreshToggleIcon();

    // 手动刷新osd右下角的弹幕图标
    void refreshDanmakuIcon();

    void setProgress(float value);

    float getProgress();

    /// Misc
    static View* create();

    void invalidate() override;

    void onLayout() override;

    bool isFullscreen();

    void setFullScreen(bool fs);

    void setCloseOnEndOfFile(bool value);

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    View* getDefaultFocus() override;

    View* getNextFocus(brls::FocusDirection direction,
                       View* currentView) override;

    void registerMpvEvent();

    void unRegisterMpvEvent();

    void resetDanmakuPosition();

    void buttonProcessing();

private:
    bool allowFullscreen  = true;
    bool registerMPVEvent = false;
    MPVEvent::Subscription eventSubscribeID;
    brls::InputManager* input;

    ///OSD
    BRLS_BIND(brls::Label, videoTitleLabel, "video/osd/title");
    BRLS_BIND(brls::Label, videoOnlineCountLabel, "video/view/label/people");
    BRLS_BIND(brls::Box, osdTopBox, "video/osd/top/box");
    BRLS_BIND(brls::Box, osdBottomBox, "video/osd/bottom/box");
    BRLS_BIND(brls::Box, osdCenterBox, "video/osd/center/box");
    BRLS_BIND(brls::ProgressSpinner, osdSpinner, "video/osd/loading");
    BRLS_BIND(VideoProgressSlider, osdSlider, "video/osd/bottom/progress");
    BRLS_BIND(brls::Label, centerLabel, "video/osd/center/label");
    BRLS_BIND(brls::Label, leftStatusLabel, "video/left/status");
    BRLS_BIND(brls::Label, rightStatusLabel, "video/right/status");
    BRLS_BIND(brls::Box, btnToggle, "video/osd/toggle");
    BRLS_BIND(SVGImage, btnToggleIcon, "video/osd/toggle/icon");
    BRLS_BIND(SVGImage, btnFullscreenIcon, "video/osd/fullscreen/icon");
    BRLS_BIND(SVGImage, btnDanmakuIcon, "video/osd/danmaku/icon");

    time_t osdLastShowTime     = 0;
    const time_t OSD_SHOW_TIME = 5;  //默认显示五秒
    MPVCore* mpvCore;
    brls::Rect oldRect = brls::Rect(-1, -1, -1, -1);
    int danmakuFont    = brls::Application::getDefaultFont();
    std::vector<DanmakuItem> danmakuData;

    bool closeOnEndOfFile = true;  // 全屏时 播放结束自动取消全屏

    //DEBUG
    BRLS_BIND(brls::Box, videoLayerDebug, "video/layer/debug");
    BRLS_BIND(brls::Box, videoLayerDanmaku, "video/layer/danmaku");
};
