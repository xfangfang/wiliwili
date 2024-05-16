//
// Created by fang on 2022/4/22.
//

#pragma once

#include <borealis/core/bind.hpp>
#include <borealis/core/box.hpp>
#include <borealis/core/application.hpp>

#include "utils/event_helper.hpp"

namespace brls {
class Label;
class ProgressSpinner;
}  // namespace brls
class MPVCore;
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

class VideoHighlightData {
public:
    int sec = 0;
    std::vector<float> data;
};

#define VIDEO_CANCEL_SEEKING 0
#define VIDEO_SEEK_IMMEDIATELY 0

class VideoView : public brls::Box {
public:
    VideoView();

    ~VideoView() override;

    /// Video control
    void setUrl(const std::string& url, int start = 0, int end = -1, const std::string& audio = "");

    void setBackupUrl(const std::string& url, int start = 0, int end = -1, const std::string& audio = "");

    // 多个音频文件添加多个音轨
    void setUrl(const std::string& url, int start, int end, const std::vector<std::string>& audios);

    // 将视频添加入播放列表，按列表序播放 （用于自动播放备份视频）
    void setBackupUrl(const std::string& url, int start, int end, const std::vector<std::string>& audios);

    // 将多个视频合并成同一个视频播放
    void setUrl(const std::vector<EDLUrl>& edl_urls, int start = 0, int end = -1);

    static std::string genExtraUrlParam(int start, int end, const std::string& audio);

    static std::string genExtraUrlParam(int start, int end, const std::vector<std::string>& audios = {});

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

    bool isOSDLock() const;

    void onOSDStateChanged(bool state);

    void toggleOSDLock();

    void toggleDanmaku();

    void toggleOSD();

    void showLoading();

    void hideLoading();

    void showCenterHint();

    void setCenterHintText(const std::string& text);

    void setCenterHintIcon(const std::string& svg);

    void hideCenterHint();

    void hideDanmakuButton();

    void hideDLNAButton();

    void hideVideoQualityButton();

    void hideVideoSpeedButton();

    void hideOSDLockButton();

    void disableCloseOnEndOfFile();

    void hideHistorySetting();

    void hideVideoRelatedSetting();

    void hideSubtitleSetting();

    void hideBottomLineSetting();

    void hideHighlightLineSetting();

    void hideSkipOpeningCreditsSetting();

    void hideVideoProgressSlider();

    /// 隐藏左下角的播放时间
    void hideStatusLabel();

    /// 将OSD改为直播样式
    void setLiveMode();

    // 使用 TV 客户端的控制逻辑
    // 1. 焦点可以选到控制条上，通过确定键选中并调整，再次点击确定完成进度调整（之前：无法通过方向键调整进度）
    // 2. osd 隐藏状态下，按下确定键显示 osd。（之前：确定键控制播放和暂停）
    // 3. 全屏时 osd 显示状态下按返回键隐藏 osd，隐藏后再按返回键退出全屏。（之前：返回键控制直接退出全屏）
    void setTvControlMode(bool state);

    bool getTvControlMode() const;

    /// 设置播放时间 (左下角: 00:00:00/00:00:00 中左侧的值)
    void setStatusLabelLeft(const std::string& value);

    /// 设置视频时长 (左下角: 00:00:00/00:00:00 中右侧的值)
    void setStatusLabelRight(const std::string& value);

    /// 设置自定义的切换播放状态的按钮事件
    void setCustomToggleAction(std::function<void()> action);

    /// 番剧自定义菜单信息
    void setBangumiCustomSetting(const std::string& title, unsigned int seasonId);

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

    // 设置高能进度条
    void setHighlightProgress(const VideoHighlightData& data);

    // 进度条上方显示提示文字
    void showHint(const std::string& value);

    void clearHint();

    /// Misc
    static View* create();

    void invalidate() override;

    void onLayout() override;

    bool isFullscreen();

    void setFullScreen(bool fs);

    /// 分集点击事件
    void setSeasonAction(brls::ActionListener action);

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    View* getDefaultFocus() override;

    void onChildFocusGained(View* directChild, View* focusedView) override;

    View* getNextFocus(brls::FocusDirection direction, View* currentView) override;

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
    inline static const std::string REPLAY         = "REPLAY";
    inline static const std::string CLIP_INFO      = "CLIP_INFO";
    inline static const std::string HIGHLIGHT_INFO = "HIGHLIGHT_INFO";
    inline static const std::string REAL_DURATION  = "REAL_DURATION";

    // 用于指定 lastPlayedPosition 的值
    // 若无历史记录，则为 -1，若不使用历史记录的值，则为 -2
    inline static const int64_t POSITION_UNDEFINED = -1;
    inline static const int64_t POSITION_DISCARD   = -2;

    // 当自动跳转下一集时不退出全屏
    inline static bool EXIT_FULLSCREEN_ON_END = true;

    // Bottom progress bar
    inline static bool BOTTOM_BAR = true;

    // Highlight progress bar
    inline static bool HIGHLIGHT_PROGRESS_BAR = false;

private:
    bool allowFullscreen  = true;
    bool registerMPVEvent = false;
    bool enableDanmaku    = true;
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
    // 播放设置中显示 高能进度条
    bool showHighlightLineSetting = true;
    // 播放设置中显示 跳过片头片尾
    bool showOpeningCreditsSetting = true;
    // 是否为直播样式
    bool isLiveMode = false;
    // 是否开启 TV 客户端的控制逻辑
    bool isTvControlMode = false;
    // 是否展示重播按钮
    bool showReplay = false;
    std::string bangumiTitle;
    unsigned int bangumiSeasonId = 0;
    MPVEvent::Subscription eventSubscribeID;
    CustomEvent::Subscription customEventSubscribeID;
    std::function<void()> customToggleAction = nullptr;
    brls::ActionListener seasonAction = nullptr;
    brls::InputManager* input;
    NVGcolor bottomBarColor = brls::Application::getTheme().getColor("color/bilibili");

    ///OSD
    BRLS_BIND(brls::Label, videoTitleLabel, "video/osd/title");
    BRLS_BIND(brls::Label, videoOnlineCountLabel, "video/view/label/people");
    BRLS_BIND(brls::Box, osdTopBox, "video/osd/top/box");
    BRLS_BIND(brls::Box, osdBottomBox, "video/osd/bottom/box");
    // 用于显示缓冲组件
    BRLS_BIND(brls::Box, osdCenterBox, "video/osd/center/box");
    BRLS_BIND(brls::ProgressSpinner, osdSpinner, "video/osd/loading");
    BRLS_BIND(brls::Label, centerLabel, "video/osd/center/label");
    // 用于通用的提示信息
    BRLS_BIND(brls::Box, osdCenterBox2, "video/osd/center/box2");
    BRLS_BIND(brls::Label, centerLabel2, "video/osd/center/label2");
    BRLS_BIND(SVGImage, centerIcon2, "video/osd/center/icon2");
    // 用于显示和控制视频时长
    BRLS_BIND(VideoProgressSlider, osdSlider, "video/osd/bottom/progress");
    BRLS_BIND(brls::Label, leftStatusLabel, "video/left/status");
    BRLS_BIND(brls::Label, centerStatusLabel, "video/center/status");
    BRLS_BIND(brls::Label, rightStatusLabel, "video/right/status");
    BRLS_BIND(brls::Label, videoQuality, "video/quality");
    BRLS_BIND(brls::Label, videoSpeed, "video/speed");
    BRLS_BIND(brls::Label, showEpisode, "show/episode");
    BRLS_BIND(brls::Label, speedHintLabel, "video/speed/hint/label");
    BRLS_BIND(brls::Box, speedHintBox, "video/speed/hint/box");
    BRLS_BIND(brls::Box, btnToggle, "video/osd/toggle");
    // 底部菜单键
    BRLS_BIND(brls::Box, iconBox, "video/osd/icon/box");
    BRLS_BIND(SVGImage, btnToggleIcon, "video/osd/toggle/icon");
    BRLS_BIND(SVGImage, btnFullscreenIcon, "video/osd/fullscreen/icon");
    BRLS_BIND(SVGImage, btnDanmakuIcon, "video/osd/danmaku/icon");
    BRLS_BIND(SVGImage, btnVolumeIcon, "video/osd/danmaku/volume/icon");
    BRLS_BIND(SVGImage, btnDanmakuSettingIcon, "video/osd/danmaku/setting/icon");
    BRLS_BIND(SVGImage, btnSettingIcon, "video/osd/setting/icon");
    BRLS_BIND(SVGImage, btnCastIcon, "video/osd/cast/icon");
    BRLS_BIND(brls::Label, hintLabel, "video/osd/hint/label");
    BRLS_BIND(brls::Box, hintBox, "video/osd/hint/box");
    BRLS_BIND(VideoProfile, videoProfile, "video/profile");
    BRLS_BIND(brls::Box, osdLockBox, "video/osd/lock/box");
    BRLS_BIND(SVGImage, osdLockIcon, "video/osd/lock/icon");

    // OSD
    time_t osdLastShowTime     = 0;
    const time_t OSD_SHOW_TIME = 5;  //默认显示五秒
    OSDState osd_state         = OSDState::HIDDEN;
    bool is_osd_shown          = false;
    bool is_osd_lock           = false;
    bool hide_lock_button      = false;
    // 区别于视频的时长，当 real_duration 大于 0 时，播放器进度条的总时长以此为准而不是以视频的实际时长为准
    // 用于正确显示预览视频的进度条，比如付费电影的预览
    int real_duration          = 0;
    time_t hintLastShowTime    = 0;
    int64_t lastPlayedPosition = POSITION_UNDEFINED;
    VideoHighlightData highlightData;  // 在播放器进度条上显示的标记点（用来展示片头片尾标记）

    MPVCore* mpvCore;
    brls::Rect oldRect = brls::Rect(-1, -1, -1, -1);

    /**
     * 预览视频进度跳转，经过 delay 毫秒后跳转到指定进度
     * @param seek 相对跳转的进度值 单位秒，当值为 0 时取消跳转请求
     * @param delay 请求的延迟触发时间，当值为 0 时立刻跳转
     */
    void requestSeeking(int seek, int delay = 400);

    bool is_seeking     = false;  // 是否正在请求跳转
    int seeking_range   = 0;      // 跳转的目标进度, 跳转结束后归零
    size_t seeking_iter = 0;      // 请求跳转的延迟函数 handle

    /**
     *  预览视频音量调节，实时调节
     * @param volume
     * @param delay 当值不为 0 时，自动处理音量UI的显示与隐藏
     */
    void requestVolume(int volume, int delay = 0);
    int volume_init    = 0;
    size_t volume_iter = 0;  // 音量UI关闭的延迟函数 handle

    /**
     * 预览应用背光调节，实时调节
     */
    void requestBrightness(float brightness);
    float brightness_init = 0.0f;

    /// 绘制高能进度条
    void drawHighlightProgress(NVGcontext* vg, float x, float y, float width, float alpha);

    void _setTvControlMode(bool state);

    float getRealDuration();
};
