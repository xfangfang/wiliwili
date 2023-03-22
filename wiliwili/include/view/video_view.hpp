//
// Created by fang on 2022/4/22.
//

#pragma once

#include <borealis.hpp>
#include "view/mpv_core.hpp"
#include "view/danmaku_core.hpp"

class VideoProgressSlider;

class SVGImage;

// https://github.com/mpv-player/mpv/blob/master/DOCS/edl-mpv.rst
class EDLUrl {
public:
    std::string url;
    float length = -1;  // second

    EDLUrl(std::string url, float length = -1) : url(url), length(length) {}
};

enum class OSDState {
    HIDDEN    = 0,
    SHOWN     = 1,
    ALWAYS_ON = 2,
};

class VideoView : public brls::Box {
public:
    VideoView();

    ~VideoView() override;

    /// Video control
    void setUrl(const std::string& url, int progress = 0,
                const std::string& audio = "");

    void setUrl(const std::vector<EDLUrl>& edl_urls, int progress = 0);

    void resume();

    void pause();

    void stop();

    void togglePlay();

    void setSpeed(float speed);

    // 视频加载前重置此变量
    // 视频加载结束后，获取此值，若大于0则跳转进度
    void setLastPlayedPosition(int64_t p);
    int64_t getLastPlayedPosition() const;

    /// OSD
    void showOSD(bool temp = true);

    void hideOSD();

    bool isOSDShown();

    void onOSDStateChanged(bool state);

    void toggleDanmaku();

    void toggleOSD();

    void showLoading();

    void hideLoading();

    void hideDanmakuButton();

    void setTitle(const std::string& title);

    std::string getTitle();

    void setQuality(const std::string& str);

    std::string getQuality();

    void setOnlineCount(const std::string& count);

    void setDuration(const std::string& value);

    void setPlaybackTime(const std::string& value);

    // 手动设置osd右下角的全屏图标
    void setFullscreenIcon(bool fs);

    // 手动刷新osd左下角的播放图标
    void refreshToggleIcon();

    // 手动刷新osd右下角的弹幕图标
    void refreshDanmakuIcon();

    void setProgress(float value);

    float getProgress();

    // 进度条上方显示提示文字
    void showHint(const std::string& value);

    void clearHint();

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

    void onChildFocusGained(View* directChild, View* focusedView) override;

    View* getNextFocus(brls::FocusDirection direction,
                       View* currentView) override;

    void registerMpvEvent();

    void unRegisterMpvEvent();

    void buttonProcessing();

    // 用于 VideoView 可以接收的自定义事件
    inline static const std::string QUALITY_CHANGE = "QUALITY_CHANGE";
    inline static const std::string SET_ONLINE_NUM = "SET_ONLINE_NUM";
    inline static const std::string SET_TITLE      = "SET_TITLE";
    inline static const std::string SET_QUALITY    = "SET_QUALITY";
    inline static const std::string HINT           = "HINT";
    inline static const std::string LAST_TIME      = "LAST_TIME";

    // 用于指定 lastPlayedPosition 的值
    // 若无历史记录，则为 -1，若不使用历史记录的值，则为 -2
    inline static const int64_t POSITION_UNDEFINED = -1;
    inline static const int64_t POSITION_DISCARD   = -2;

private:
    bool allowFullscreen  = true;
    bool registerMPVEvent = false;
    MPVEvent::Subscription eventSubscribeID;
    MPVCustomEvent::Subscription customEventSubscribeID;
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
    BRLS_BIND(brls::Label, videoQuality, "video/quality");
    BRLS_BIND(brls::Box, btnToggle, "video/osd/toggle");
    BRLS_BIND(SVGImage, btnToggleIcon, "video/osd/toggle/icon");
    BRLS_BIND(SVGImage, btnFullscreenIcon, "video/osd/fullscreen/icon");
    BRLS_BIND(SVGImage, btnDanmakuIcon, "video/osd/danmaku/icon");
    BRLS_BIND(SVGImage, btnDanmakuSettingIcon,
              "video/osd/danmaku/setting/icon");
    BRLS_BIND(SVGImage, btnSettingIcon, "video/osd/setting/icon");
    BRLS_BIND(brls::Label, hintLabel, "video/osd/hint/label");
    BRLS_BIND(brls::Box, hintBox, "video/osd/hint/box");

    // OSD
    time_t osdLastShowTime     = 0;
    const time_t OSD_SHOW_TIME = 5;  //默认显示五秒
    OSDState osd_state         = OSDState::HIDDEN;
    bool is_osd_shown          = false;
    time_t hintLastShowTime    = 0;
    int64_t lastPlayedPosition = POSITION_UNDEFINED;

    MPVCore* mpvCore;
    brls::Rect oldRect = brls::Rect(-1, -1, -1, -1);

    bool closeOnEndOfFile = true;  // 全屏时 播放结束自动取消全屏

    //DEBUG
    BRLS_BIND(brls::Box, videoLayerDebug, "video/layer/debug");
    BRLS_BIND(brls::Box, videoLayerDanmaku, "video/layer/danmaku");
};
