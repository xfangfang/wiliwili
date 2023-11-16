//
// Created by fang on 2022/4/22.
//

#pragma once

#include <borealis.hpp>
#include "view/mpv_core.hpp"

class VideoProgressSlider;

class SVGImage;

class VideoProfile;

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

    void setBackupUrl(const std::string& url, int progress = 0,
                      const std::string& audio = "");

    void setUrl(const std::string& url, int progress,
                const std::vector<std::string>& audios);

    void setBackupUrl(const std::string& url, int progress,
                      const std::vector<std::string>& audios);

    void setUrl(const std::vector<EDLUrl>& edl_urls, int progress = 0);

    static std::string genExtraUrlParam(int progress, const std::string& audio);

    static std::string genExtraUrlParam(
        int progress, const std::vector<std::string>& audios = {});

    void resume();

    void pause();

    void stop();

    void togglePlay();

    void setSpeed(float speed);

    // 视频加载前重置此变量
    // 视频加载结束后，获取此值，若大于0则跳转进度
    void setLastPlayedPosition(int64_t p);
    [[nodiscard]] int64_t getLastPlayedPosition() const;

    /// OSD
    void showOSD(bool temp = true);

    void hideOSD();

    bool isOSDShown() const;

    void onOSDStateChanged(bool state);

    void toggleDanmaku();

    void toggleOSD();

    void showLoading();

    void hideLoading();

    void hideDanmakuButton();

    void hideDLNAButton();

    void hideVideoQualityButton();

    void hideVideoSpeedButton();

    void disableCloseOnEndOfFile();

    void disableBottomBar();

    void hideHistorySetting();

    void hideVideoRelatedSetting();

    void hideSubtitleSetting();

    void hideBottomLineSetting();

    void hideVideoProgressSlider();

    /// 隐藏左下角的播放时间
    void hideStatusLabel();

    /// 将OSD改为直播样式
    void setLiveMode();

    /// 设置播放时间 (左下角: 00:00:00/00:00:00 中左侧的值)
    void setStatusLabelLeft(const std::string& value);

    /// 设置视频时长 (左下角: 00:00:00/00:00:00 中右侧的值)
    void setStatusLabelRight(const std::string& value);

    /// 设置自定义的切换播放状态的按钮事件
    void setCustomToggleAction(std::function<void()> action);

    void setTitle(const std::string& title);

    std::string getTitle();

    void setQuality(const std::string& str);

    std::string getQuality();

    void setOnlineCount(const std::string& count);

    void setDuration(const std::string& value);

    /// 设置播放时间 (左下角: 00:00:00/00:00:00 中左侧的值)
    /// 当拖拽进度或直播时，此函数无效
    void setPlaybackTime(const std::string& value);

    // 手动设置osd右下角的全屏图标
    void setFullscreenIcon(bool fs);

    brls::View* getFullscreenIcon();

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

    // 当自动跳转下一集时不退出全屏
    inline static bool EXIT_FULLSCREEN_ON_END = true;

    // Bottom progress bar
    inline static bool BOTTOM_BAR = true;

private:
    bool allowFullscreen  = true;
    bool registerMPVEvent = false;
    bool showDanmaku      = true;
    // 和前面的 EXIT_FULLSCREEN_ON_END 共同控制，closeOnEndOfFile用来控制单个VideoView行为
    bool closeOnEndOfFile = true;
    // 播放设置中显示 上传历史记录
    bool showHistorySetting = true;
    // 播放设置中显示 播放结束行为相关的设置
    bool showVideoRelatedSetting = true;
    // 播放设置中显示 字幕选择
    bool showSubtitleSetting = true;
    // 播放设置中显示 底部进度条
    bool showBottomLineSetting = true;
    // 是否为直播样式
    bool isLiveMode = false;
    MPVEvent::Subscription eventSubscribeID;
    MPVCustomEvent::Subscription customEventSubscribeID;
    std::function<void()> customToggleAction = nullptr;
    brls::InputManager* input;
    NVGcolor bottomBarColor =
        brls::Application::getTheme().getColor("color/bilibili");

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
    BRLS_BIND(brls::Label, centerStatusLabel, "video/center/status");
    BRLS_BIND(brls::Label, rightStatusLabel, "video/right/status");
    BRLS_BIND(brls::Label, videoQuality, "video/quality");
    BRLS_BIND(brls::Label, videoSpeed, "video/speed");
    BRLS_BIND(brls::Label, speedHintLabel, "video/speed/hint/label");
    BRLS_BIND(brls::Box, speedHintBox, "video/speed/hint/box");
    BRLS_BIND(brls::Box, btnToggle, "video/osd/toggle");
    BRLS_BIND(SVGImage, btnToggleIcon, "video/osd/toggle/icon");
    BRLS_BIND(SVGImage, btnFullscreenIcon, "video/osd/fullscreen/icon");
    BRLS_BIND(SVGImage, btnDanmakuIcon, "video/osd/danmaku/icon");
    BRLS_BIND(SVGImage, btnVolumeIcon, "video/osd/danmaku/volume/icon");
    BRLS_BIND(SVGImage, btnDanmakuSettingIcon,
              "video/osd/danmaku/setting/icon");
    BRLS_BIND(SVGImage, btnSettingIcon, "video/osd/setting/icon");
    BRLS_BIND(SVGImage, btnCastIcon, "video/osd/cast/icon");
    BRLS_BIND(brls::Label, hintLabel, "video/osd/hint/label");
    BRLS_BIND(brls::Box, hintBox, "video/osd/hint/box");
    BRLS_BIND(VideoProfile, videoProfile, "video/profile");

    // OSD
    time_t osdLastShowTime     = 0;
    const time_t OSD_SHOW_TIME = 5;  //默认显示五秒
    OSDState osd_state         = OSDState::HIDDEN;
    bool is_osd_shown          = false;
    bool is_seeking            = false;
    int64_t seeking_range      = 0;
    size_t seeking_iter        = 0;
    time_t hintLastShowTime    = 0;
    int64_t lastPlayedPosition = POSITION_UNDEFINED;

    MPVCore* mpvCore;
    brls::Rect oldRect = brls::Rect(-1, -1, -1, -1);

    /**
     * 延迟 200ms 触发进度跳转到 seeking_range
     * seeking_range: 相对进度，单位秒
     */
    void requestSeeking();

    /**
     * 禁用系统锁屏
     */
    static void disableDimming(bool disable);
};
