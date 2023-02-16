//
// Created by fang on 2022/4/23.
//

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "view/video_progress_slider.hpp"
#include "view/svg_image.hpp"
#include "utils/number_helper.hpp"
#include "activity/player_activity.hpp"
#include "fragment/player_danmaku_setting.hpp"

using namespace brls;

VideoView::VideoView() {
    mpvCore = &MPVCore::instance();
    this->inflateFromXMLRes("xml/views/video_view.xml");
    this->setHideHighlightBackground(true);
    this->setHideClickAnimation(true);

    input = brls::Application::getPlatform()->getInputManager();

    this->registerBoolXMLAttribute("allowFullscreen", [this](bool value) {
        this->allowFullscreen = value;
        if (!value) {
            this->btnFullscreenIcon->getParent()->setVisibility(
                brls::Visibility::GONE);
            this->registerAction(
                "cancel", brls::ControllerButton::BUTTON_B,
                [this](brls::View* view) -> bool {
                    this->dismiss();
                    return true;
                },
                true);
        }
    });

    this->registerAction(
        "\uE08F", brls::ControllerButton::BUTTON_LB,
        [this](brls::View* view) -> bool {
            mpvCore->command_str("seek -10");
            return true;
        },
        false, true);

    this->registerAction(
        "\uE08E", brls::ControllerButton::BUTTON_RB,
        [this](brls::View* view) -> bool {
            ControllerState state;
            input->updateUnifiedControllerState(&state);
            if (state.buttons[BUTTON_Y]) {
                mpvCore->command_str("seek -10");
            } else {
                mpvCore->command_str("seek +10");
            }
            return true;
        },
        false, true);

    this->registerAction(
        "toggleOSD", brls::ControllerButton::BUTTON_Y,
        [this](brls::View* view) -> bool {
            this->toggleOSD();
            return true;
        },
        true);

    this->registerAction(
        "toggleDanmaku", brls::ControllerButton::BUTTON_X,
        [this](brls::View* view) -> bool {
            this->toggleDanmaku();
            return true;
        },
        true);

    this->registerMpvEvent();

    osdSlider->getProgressSetEvent()->subscribe([this](float progress) {
        brls::Logger::verbose("Set progress: {}", progress);
        this->showOSD(true);
        mpvCore->command_str(
            fmt::format("seek {} absolute-percent", progress * 100).c_str());
    });

    osdSlider->getProgressEvent()->subscribe([this](float progress) {
        this->showOSD(false);
        leftStatusLabel->setText(
            wiliwili::sec2Time(mpvCore->duration * progress));
    });

    /// 点击屏幕其他位置切换OSD
    this->addGestureRecognizer(
        new brls::TapGestureRecognizer(this, [this]() { this->toggleOSD(); }));

    /// 播放/暂停 按钮
    this->btnToggle->addGestureRecognizer(new brls::TapGestureRecognizer(
        this->btnToggle,
        [this]() {
            if (mpvCore->isPaused()) {
                mpvCore->resume();
            } else {
                mpvCore->pause();
            }
        },
        brls::TapGestureConfig(false, brls::SOUND_NONE, brls::SOUND_NONE,
                               brls::SOUND_NONE)));

    /// 清晰度按钮
    this->videoQuality->registerClickAction([this](...) {
        mpvCore->getEvent()->fire(MpvEventEnum::QUALITY_CHANGE_REQUEST);
        return true;
    });
    this->videoQuality->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->videoQuality));
    this->registerAction(
        "wiliwili/player/quality"_i18n, brls::ControllerButton::BUTTON_START,
        [this](brls::View* view) -> bool {
            mpvCore->getEvent()->fire(MpvEventEnum::QUALITY_CHANGE_REQUEST);
            return true;
        });

    /// 全屏按钮
    this->btnFullscreenIcon->getParent()->registerClickAction([this](...) {
        if (this->isFullscreen()) {
            this->setFullScreen(false);
        } else {
            this->setFullScreen(true);
        }
        return true;
    });
    this->btnFullscreenIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnFullscreenIcon->getParent()));

    /// 弹幕切换按钮
    this->btnDanmakuIcon->getParent()->registerClickAction([this](...) {
        this->toggleDanmaku();
        return true;
    });
    this->btnDanmakuIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnDanmakuIcon->getParent()));

    /// 弹幕设置按钮
    this->btnDanmakuSettingIcon->getParent()->registerClickAction([](...) {
        auto setting = new PlayerDanmakuSetting();
        brls::Application::pushActivity(new Activity(setting));
        // 手动将焦点赋给设置页面
        brls::sync([setting]() { brls::Application::giveFocus(setting); });
        return true;
    });
    this->btnDanmakuSettingIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(
            this->btnDanmakuSettingIcon->getParent()));

    this->refreshDanmakuIcon();

    this->registerAction(
        "cancel", brls::ControllerButton::BUTTON_B,
        [this](brls::View* view) -> bool {
            if (this->isFullscreen()) {
                this->setFullScreen(false);
            } else {
                this->dismiss();
            }
            return true;
        },
        true);

    this->registerAction("wiliwili/player/fs"_i18n,
                         brls::ControllerButton::BUTTON_A,
                         [this](brls::View* view) {
                             if (this->isFullscreen()) {
                                 //全屏状态下切换播放状态
                                 this->togglePlay();
                                 this->showOSD(true);
                             } else {
                                 //非全屏状态点击视频组件进入全屏
                                 this->setFullScreen(true);
                             }
                             return true;
                         });
}

VideoView::~VideoView() {
    brls::Logger::debug("trying delete VideoView...");
    this->unRegisterMpvEvent();
    brls::Logger::debug("Delete VideoView done");
}

void VideoView::draw(NVGcontext* vg, float x, float y, float width,
                     float height, Style style, FrameContext* ctx) {
    if (!mpvCore->isValid()) return;

    mpvCore->openglDraw(this->getFrame(), this->getAlpha());

    // draw danmaku
    DanmakuCore::instance().drawDanmaku(vg, x, y, width, height, getAlpha());

    // draw osd
    if (wiliwili::unix_time() < this->osdLastShowTime) {
        if (!is_osd_shown) {
            is_osd_shown = true;
            this->onOSDStateChanged(true);
        }
        osdBottomBox->frame(ctx);
        osdTopBox->frame(ctx);
    } else {
        if (is_osd_shown) {
            is_osd_shown = false;
            this->onOSDStateChanged(false);
        }
    }

    // hot key
    this->buttonProcessing();

    osdCenterBox->frame(ctx);
}

void VideoView::invalidate() { View::invalidate(); }

void VideoView::onLayout() {
    brls::View::onLayout();

    brls::Rect rect = getFrame();

    if (oldRect.getWidth() == -1) {
        //初始化
        this->oldRect = rect;
    }

    if (!(rect == oldRect)) {
        brls::Logger::debug("Video view: {} size: {} / {} scale: {}",
                            (int64_t)this, rect.getWidth(), rect.getHeight(),
                            Application::windowScale);
        this->mpvCore->setFrameSize(rect);
    }
    oldRect = rect;
}

void VideoView::setUrl(std::string url, int progress, std::string audio) {
    brls::Logger::debug("set video url: {}", url);

    if (progress < 0) progress = 0;
    std::string extra = "referrer=\"https://www.bilibili.com\"";
    if (progress > 0) {
        extra += fmt::format(",start={}", progress);
        brls::Logger::debug("set video progress: {}", progress);
    }
    if (!audio.empty()) {
        extra += fmt::format(",audio-file=\"{}\"", audio);
        brls::Logger::debug("set audio: {}", audio);
    }
    brls::Logger::debug("Extra options: {}", extra);

    mpvCore->setUrl(url, extra);
}

void VideoView::setUrl(const std::vector<EDLUrl>& edl_urls, int progress) {
    std::string url = "edl://";
    std::vector<std::string> urls;
    bool delay_open = true;
    for (auto& i : edl_urls) {
        if (i.length < 0) {
            delay_open = false;
            break;
        }
    }
    for (auto& i : edl_urls) {
        if (!delay_open) {
            urls.emplace_back(fmt::format("%{}%{}", i.url.size(), i.url));
            continue;
        }
        urls.emplace_back(
            "!delay_open,media_type=video;!delay_open,media_type=audio;" +
            fmt::format("%{}%{},length={}", i.url.size(), i.url, i.length));
    }
    url += pystring::join(";", urls);
    this->setUrl(url, progress);
}

void VideoView::resume() { mpvCore->resume(); }

void VideoView::pause() { mpvCore->pause(); }

void VideoView::stop() { mpvCore->stop(); }

void VideoView::togglePlay() {
    if (this->mpvCore->isPaused()) {
        this->resume();
    } else {
        this->pause();
    }
}

void VideoView::setSpeed(float speed) {
    mpvCore->command_str(fmt::format("set speed {}", speed).c_str());
    DanmakuCore::instance().refresh();
}

/// OSD
void VideoView::showOSD(bool temp) {
    if (temp) {
        this->osdLastShowTime =
            wiliwili::unix_time() + VideoView::OSD_SHOW_TIME;
        this->osd_state = OSDState::SHOWN;
    } else {
        this->osdLastShowTime = 0xffffffff;
        this->osd_state       = OSDState::ALWAYS_ON;
    }
}

void VideoView::hideOSD() {
    this->osdLastShowTime = 0;
    this->osd_state       = OSDState::HIDDEN;
}

bool VideoView::isOSDShown() { return this->is_osd_shown; }

void VideoView::onOSDStateChanged(bool state) {
    // 当焦点位于video组件内部重新赋予焦点，用来隐藏屏幕上的高亮框
    if (!state && isChildFocused()) {
        brls::Application::giveFocus(this);
    }
}

void VideoView::toggleDanmaku() {
    if (DanmakuCore::DANMAKU_ON) {
        DanmakuCore::DANMAKU_ON = false;
        this->btnDanmakuIcon->setImageFromSVGRes(
            "svg/bpx-svg-sprite-danmu-switch-off.svg");
    } else {
        DanmakuCore::DANMAKU_ON = true;
        this->btnDanmakuIcon->setImageFromSVGRes(
            "svg/bpx-svg-sprite-danmu-switch-on.svg");
    }
    DanmakuCore::save();
}

void VideoView::toggleOSD() {
    if (isOSDShown()) {
        this->hideOSD();
    } else {
        this->showOSD(true);
    }
}

// Loading
void VideoView::showLoading() {
    centerLabel->setVisibility(brls::Visibility::INVISIBLE);
    osdCenterBox->setVisibility(brls::Visibility::VISIBLE);
}

void VideoView::hideLoading() {
    osdCenterBox->setVisibility(brls::Visibility::GONE);
}

void VideoView::hideDanmakuButton() {
    btnDanmakuIcon->getParent()->setVisibility(brls::Visibility::GONE);
    btnDanmakuSettingIcon->getParent()->setVisibility(brls::Visibility::GONE);
}

void VideoView::setTitle(std::string title) {
    ASYNC_RETAIN
    brls::Threading::sync([ASYNC_TOKEN, title]() {
        ASYNC_RELEASE
        this->videoTitleLabel->setText(title);
    });
}

void VideoView::setOnlineCount(std::string count) {
    ASYNC_RETAIN
    brls::Threading::sync([ASYNC_TOKEN, count]() {
        ASYNC_RELEASE
        this->videoOnlineCountLabel->setText(count);
    });
}

std::string VideoView::getTitle() {
    return this->videoTitleLabel->getFullText();
}

void VideoView::setQuality(std::string str) {
    ASYNC_RETAIN
    brls::Threading::sync([ASYNC_TOKEN, str]() {
        ASYNC_RELEASE
        this->videoQuality->setText(str);
    });
}

std::string VideoView::getQuality() {
    return this->videoQuality->getFullText();
}

void VideoView::setDuration(std::string value) {
    this->rightStatusLabel->setText(value);
}

void VideoView::setPlaybackTime(std::string value) {
    this->leftStatusLabel->setText(value);
}

void VideoView::setFullscreenIcon(bool fs) {
    if (fs) {
        btnFullscreenIcon->setImageFromSVGRes(
            "svg/bpx-svg-sprite-fullscreen-off.svg");
    } else {
        btnFullscreenIcon->setImageFromSVGRes(
            "svg/bpx-svg-sprite-fullscreen.svg");
    }
}

void VideoView::refreshDanmakuIcon() {
    if (DanmakuCore::DANMAKU_ON) {
        this->btnDanmakuIcon->setImageFromSVGRes(
            "svg/bpx-svg-sprite-danmu-switch-on.svg");
    } else {
        this->btnDanmakuIcon->setImageFromSVGRes(
            "svg/bpx-svg-sprite-danmu-switch-off.svg");
    }
}

void VideoView::refreshToggleIcon() {
    if (mpvCore->isPaused()) {
        btnToggleIcon->setImageFromSVGRes("svg/bpx-svg-sprite-play.svg");
    } else {
        btnToggleIcon->setImageFromSVGRes("svg/bpx-svg-sprite-pause.svg");
    }
}

void VideoView::setProgress(float value) {
    this->osdSlider->setProgress(value);
}

float VideoView::getProgress() { return this->osdSlider->getProgress(); }

View* VideoView::create() { return new VideoView(); }

bool VideoView::isFullscreen() {
    auto rect = this->getFrame();
    return rect.getHeight() == brls::Application::contentHeight &&
           rect.getWidth() == brls::Application::contentWidth;
}

void VideoView::setFullScreen(bool fs) {
    if (!allowFullscreen) {
        brls::Logger::error("Not being allowed to set fullscreen");
        return;
    }

    if (fs == isFullscreen()) {
        brls::Logger::error("Already set fullscreen state to: {}", fs);
        return;
    }

    brls::Logger::info("VideoView set fullscreen state: {}", fs);
    if (fs) {
        this->unRegisterMpvEvent();
        auto container = new brls::Box();
        auto video     = new VideoView();
        float width    = brls::Application::contentWidth;
        float height   = brls::Application::contentHeight;

        container->setDimensions(width, height);
        video->setDimensions(width, height);
        video->setWidthPercentage(100);
        video->setHeightPercentage(100);
        video->setId("video");
        video->setTitle(this->getTitle());
        video->setQuality(this->getQuality());
        video->setDuration(this->rightStatusLabel->getFullText());
        video->setPlaybackTime(this->leftStatusLabel->getFullText());
        video->setProgress(this->getProgress());
        video->showOSD(this->osd_state != OSDState::ALWAYS_ON);
        video->setFullscreenIcon(true);
        video->setHideHighlight(true);
        video->refreshToggleIcon();
        DanmakuCore::instance().refresh();
        video->setOnlineCount(this->videoOnlineCountLabel->getFullText());
        if (osdCenterBox->getVisibility() == brls::Visibility::GONE) {
            video->hideLoading();
        }
        container->addView(video);
        brls::Application::pushActivity(new brls::Activity(container),
                                        brls::TransitionAnimation::NONE);

        // 手动将焦点 赋给新的video组件
        brls::sync([video]() { brls::Application::giveFocus(video); });
    } else {
        ASYNC_RETAIN
        brls::sync([ASYNC_TOKEN]() {
            ASYNC_RELEASE
            //todo: a better way to get videoView pointer
            BasePlayerActivity* last = dynamic_cast<BasePlayerActivity*>(
                Application::getActivitiesStack()
                    [Application::getActivitiesStack().size() - 2]);
            if (last) {
                VideoView* video = dynamic_cast<VideoView*>(
                    last->getView("video/detail/video"));
                if (video) {
                    video->setProgress(this->getProgress());
                    video->showOSD(this->osd_state != OSDState::ALWAYS_ON);
                    video->setDuration(this->rightStatusLabel->getFullText());
                    video->setPlaybackTime(
                        this->leftStatusLabel->getFullText());
                    video->registerMpvEvent();
                    video->refreshToggleIcon();
                    video->refreshDanmakuIcon();
                    video->setQuality(this->getQuality());
                    DanmakuCore::instance().refresh();
                    if (osdCenterBox->getVisibility() ==
                        brls::Visibility::GONE) {
                        video->hideLoading();
                    } else {
                        video->showLoading();
                    }
                    // 立刻准确地显示视频尺寸
                    this->mpvCore->setFrameSize(video->getFrame());
                }
            }
            // Pop fullscreen videoView
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
        });
    }
}

void VideoView::setCloseOnEndOfFile(bool value) {
    this->closeOnEndOfFile = value;
}

View* VideoView::getDefaultFocus() {
    if (isFullscreen() && isOSDShown())
        return this->btnToggle;
    else
        return this;
}

View* VideoView::getNextFocus(brls::FocusDirection direction,
                              View* currentView) {
    if (this->isFullscreen()) return this;
    return Box::getNextFocus(direction, currentView);
}

enum ClickState {
    IDLE         = 0,
    PRESS        = 1,
    FAST_RELEASE = 3,
    FAST_PRESS   = 4,
    CLICK_DOUBLE = 5
};

void VideoView::buttonProcessing() {
    ControllerState state{};
    input->updateUnifiedControllerState(&state);
    static int64_t rsb_press_time = 0;
    int CHECK_TIME                = 200000;

    static int click_state = ClickState::IDLE;
    switch (click_state) {
        case ClickState::IDLE:
            if (state.buttons[BUTTON_RSB]) {
                setSpeed(2.0f);
                rsb_press_time = getCPUTimeUsec();
                click_state    = ClickState::PRESS;
            }
            break;
        case ClickState::PRESS:
            if (!state.buttons[BUTTON_RSB]) {
                setSpeed(1.0f);
                int64_t current_time = getCPUTimeUsec();
                if (current_time - rsb_press_time < CHECK_TIME) {
                    // 点击事件
                    brls::Logger::debug("点击");
                    rsb_press_time = current_time;
                    click_state    = ClickState::FAST_RELEASE;
                } else {
                    click_state = ClickState::IDLE;
                }
            }
            break;
        case ClickState::FAST_RELEASE:
            if (state.buttons[BUTTON_RSB]) {
                setSpeed(2.0f);
                int64_t current_time = getCPUTimeUsec();
                if (current_time - rsb_press_time < CHECK_TIME) {
                    rsb_press_time = current_time;
                    click_state    = ClickState::FAST_PRESS;
                } else {
                    rsb_press_time = current_time;
                    click_state    = ClickState::PRESS;
                }
            }
            break;
        case ClickState::FAST_PRESS:
            if (!state.buttons[BUTTON_RSB]) {
                int64_t current_time = getCPUTimeUsec();
                if (current_time - rsb_press_time < CHECK_TIME) {
                    rsb_press_time = current_time;
                    // 双击事件
                    setSpeed(2.0f);
                    click_state = ClickState::CLICK_DOUBLE;
                } else {
                    setSpeed(1.0f);
                    click_state = ClickState::IDLE;
                }
            }
            break;
        case ClickState::CLICK_DOUBLE:
            brls::Logger::debug("speed lock: 2.0");
            click_state = ClickState::IDLE;
            break;
    }

    // 当OSD显示时左右切换选择按钮，持续显示OSD
    if (isOSDShown() &&
        (state.buttons[BUTTON_NAV_RIGHT] || state.buttons[BUTTON_NAV_LEFT])) {
        if (this->osd_state == OSDState::SHOWN) this->showOSD(true);
    }
}

void VideoView::registerMpvEvent() {
    if (registerMPVEvent) {
        brls::Logger::error("VideoView already register MPV Event");
    }
    eventSubscribeID =
        mpvCore->getEvent()->subscribe([this](MpvEventEnum event) {
            // brls::Logger::info("mpv event => : {}", event);
            switch (event) {
                case MpvEventEnum::MPV_RESUME:
                    this->showOSD(true);
                    this->hideLoading();
                    this->btnToggleIcon->setImageFromSVGRes(
                        "svg/bpx-svg-sprite-pause.svg");
                    break;
                case MpvEventEnum::MPV_PAUSE:
                    this->showOSD(false);
                    this->btnToggleIcon->setImageFromSVGRes(
                        "svg/bpx-svg-sprite-play.svg");
                    break;
                case MpvEventEnum::START_FILE:
                    this->showOSD(false);
                    break;
                case MpvEventEnum::LOADING_START:
                    this->showLoading();
                    break;
                case MpvEventEnum::LOADING_END:
                    this->hideLoading();
                    break;
                case MpvEventEnum::MPV_STOP:
                    // todo: 当前播放结束，尝试播放下一个视频
                    this->hideLoading();
                    this->showOSD(false);
                    break;
                case MpvEventEnum::MPV_LOADED:
                    this->setPlaybackTime(
                        wiliwili::sec2Time(this->mpvCore->video_progress));
                    break;
                case MpvEventEnum::UPDATE_DURATION:
                    this->setDuration(wiliwili::sec2Time(mpvCore->duration));
                    this->setProgress(this->mpvCore->playback_time /
                                      this->mpvCore->duration);
                    break;
                case MpvEventEnum::UPDATE_PROGRESS:
                    this->setPlaybackTime(
                        wiliwili::sec2Time(this->mpvCore->video_progress));
                    this->setProgress(this->mpvCore->playback_time /
                                      this->mpvCore->duration);
                    break;
                case MpvEventEnum::END_OF_FILE:
                    // 播放结束自动取消全屏
                    this->showOSD(false);
                    if (this->closeOnEndOfFile && this->isFullscreen()) {
                        this->setFullScreen(false);
                    }
                    break;
                case MpvEventEnum::CACHE_SPEED_CHANGE:
                    // 仅当加载圈已经开始转起的情况显示缓存
                    if (this->osdCenterBox->getVisibility() !=
                        brls::Visibility::GONE) {
                        if (this->centerLabel->getVisibility() !=
                            brls::Visibility::VISIBLE)
                            this->centerLabel->setVisibility(
                                brls::Visibility::VISIBLE);
                        this->centerLabel->setText(mpvCore->getCacheSpeed());
                    }
                    break;
                case MpvEventEnum::QUALITY_CHANGED:
                    videoQuality->setText(mpvCore->qualityStr);
                    break;
                default:
                    break;
            }
        });
    registerMPVEvent = true;
}

void VideoView::unRegisterMpvEvent() {
    if (!registerMPVEvent) return;
    mpvCore->getEvent()->unsubscribe(eventSubscribeID);
    registerMPVEvent = false;
}

void VideoView::onChildFocusGained(View* directChild, View* focusedView) {
    Box::onChildFocusGained(directChild, focusedView);
    // 只有在全屏显示OSD时允许OSD组件获取焦点
    if (this->isFullscreen() && isOSDShown()) {
        // 当弹幕按钮隐藏时不可获取焦点
        if (focusedView->getParent()->getVisibility() ==
            brls::Visibility::GONE) {
            brls::Application::giveFocus(this);
        }
        return;
    }
    brls::Application::giveFocus(this);
}
