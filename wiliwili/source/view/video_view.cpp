//
// Created by fang on 2022/4/23.
//

#include "pystring.h"
#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "view/subtitle_core.hpp"
#include "view/video_progress_slider.hpp"
#include "view/svg_image.hpp"
#include "view/grid_dropdown.hpp"
#include "view/video_profile.hpp"
#include "utils/number_helper.hpp"
#include "utils/config_helper.hpp"
#include "activity/player_activity.hpp"
#include "fragment/player_danmaku_setting.hpp"
#include "fragment/player_setting.hpp"

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
    this->videoQuality->getParent()->registerClickAction([](...) {
        MPV_CE->fire(VideoView::QUALITY_CHANGE, nullptr);
        return true;
    });
    this->videoQuality->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->videoQuality->getParent()));
    this->registerAction("wiliwili/player/quality"_i18n,
                         brls::ControllerButton::BUTTON_START,
                         [](brls::View* view) -> bool {
                             MPV_CE->fire(VideoView::QUALITY_CHANGE, nullptr);
                             return true;
                         });

    /// 视频详情信息
    this->registerAction(
        "profile", brls::ControllerButton::BUTTON_BACK,
        [this](brls::View* view) -> bool {
            if (videoProfile->getVisibility() == brls::Visibility::VISIBLE) {
                videoProfile->setVisibility(brls::Visibility::INVISIBLE);
                return true;
            }
            videoProfile->setVisibility(brls::Visibility::VISIBLE);
            videoProfile->update();
            return true;
        },
        true);

    /// 倍速按钮
    this->videoSpeed->getParent()->registerClickAction([](...) {
        auto conf = ProgramConfig::instance().getOptionData(
            SettingItem::PLAYER_DEFAULT_SPEED);

        // 找到当前倍速对应的列表值
        double speed      = MPVCore::instance().getSpeed();
        int selectedIndex = (int)ProgramConfig::instance().getIntOptionIndex(
            SettingItem::PLAYER_DEFAULT_SPEED);
        for (size_t i = 0; i < conf.rawOptionList.size(); i++) {
            if (fabs(conf.rawOptionList[i] * 0.01 - speed) < 1e-5) {
                selectedIndex = (int)i;
                break;
            }
        }

        // 展示倍速列表
        auto* drop = BaseDropdown::text(
            "wiliwili/player/speed"_i18n, conf.optionList,
            [conf](int selected) {
                // 设置播放器倍速
                MPVCore::instance().setSpeed(conf.rawOptionList[selected] *
                                             0.01);
                // 保存下倍速非1时的值，快捷键触发时使用此值
                if (conf.rawOptionList[selected] != 100) {
                    MPVCore::VIDEO_SPEED = conf.rawOptionList[selected];
                    ProgramConfig::instance().setSettingItem(
                        SettingItem::PLAYER_DEFAULT_SPEED,
                        MPVCore::VIDEO_SPEED);
                }
            },
            selectedIndex);

        // 手动将焦点赋给菜单页面
        brls::sync([drop]() { brls::Application::giveFocus(drop); });

        return true;
    });
    this->videoSpeed->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->videoSpeed->getParent()));

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
        GA("open_danmaku_setting")
        return true;
    });
    this->btnDanmakuSettingIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(
            this->btnDanmakuSettingIcon->getParent()));

    /// 播放器设置按钮
    this->btnSettingIcon->getParent()->registerClickAction([](...) {
        auto setting = new PlayerSetting();
        brls::Application::pushActivity(new Activity(setting));
        // 手动将焦点赋给设置页面
        brls::sync([setting]() { brls::Application::giveFocus(setting); });
        GA("open_player_setting")
        return true;
    });
    this->btnSettingIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnSettingIcon->getParent()));

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

    // 自定义的mpv事件
    customEventSubscribeID = MPV_CE->subscribe([this](const std::string& event,
                                                      void* data) {
        if (event == VideoView::SET_TITLE) {
            this->setTitle((const char*)data);
        } else if (event == VideoView::SET_ONLINE_NUM) {
            this->setOnlineCount((const char*)data);
        } else if (event == VideoView::SET_QUALITY) {
            this->setQuality((const char*)data);
        } else if (event == VideoView::LAST_TIME) {
            if (this->getLastPlayedPosition() != VideoView::POSITION_DISCARD)
                this->setLastPlayedPosition(*(int64_t*)data / 1000);
        } else if (event == VideoView::HINT) {
            this->showHint((const char*)data);
        }
    });
}

VideoView::~VideoView() {
    brls::Logger::debug("trying delete VideoView...");
    this->unRegisterMpvEvent();
    MPV_CE->unsubscribe(customEventSubscribeID);
    brls::Logger::debug("Delete VideoView done");
}

void VideoView::draw(NVGcontext* vg, float x, float y, float width,
                     float height, Style style, FrameContext* ctx) {
    if (!mpvCore->isValid()) return;

    // draw video
    mpvCore->openglDraw(this->getFrame(), this->getAlpha());

    // draw danmaku
    DanmakuCore::instance().drawDanmaku(vg, x, y, width, height, getAlpha());

    // draw osd
    time_t current = wiliwili::unix_time();
    if (current < this->osdLastShowTime) {
        if (!is_osd_shown) {
            is_osd_shown = true;
            this->onOSDStateChanged(true);
        }
        osdTopBox->setVisibility(brls::Visibility::VISIBLE);
        osdBottomBox->setVisibility(brls::Visibility::VISIBLE);
        osdBottomBox->frame(ctx);
        osdTopBox->frame(ctx);

        // draw subtitle (upon osd)
        SubtitleCore::instance().drawSubtitle(vg, x, y, width, height - 120,
                                              getAlpha());
    } else {
        if (is_osd_shown) {
            is_osd_shown = false;
            this->onOSDStateChanged(false);
        }
        osdTopBox->setVisibility(brls::Visibility::INVISIBLE);
        osdBottomBox->setVisibility(brls::Visibility::INVISIBLE);

        // draw subtitle (without osd)
        SubtitleCore::instance().drawSubtitle(vg, x, y, width, height,
                                              getAlpha());
    }
    if (current > this->hintLastShowTime &&
        this->hintBox->getVisibility() == brls::Visibility::VISIBLE) {
        this->clearHint();
    }

    // hot key
    this->buttonProcessing();

    // draw speed hint
    if (speedHintBox->getVisibility() == brls::Visibility::VISIBLE) {
        speedHintBox->frame(ctx);
        Rect frame = speedHintLabel->getFrame();

        // a1-3 周期 800，范围 800 * 0.3 / 2 = 120, 0 - 120 - 0
        int a1 = ((brls::getCPUTimeUsec() >> 10) % 800) * 0.3;
        int a2 = (a1 + 40) % 240;
        int a3 = (a1 + 80) % 240;
        if (a1 > 120) a1 = 240 - a1;
        if (a2 > 120) a2 = 240 - a2;
        if (a3 > 120) a3 = 240 - a3;

        float tx                              = frame.getMinX() - 50;
        float ty                              = frame.getMinY() + 7;
        std::vector<std::pair<int, int>> data = {
            {0, a3 + 80}, {15, a2 + 80}, {30, a1 + 80}};

        for (auto& i : data) {
            nvgBeginPath(vg);
            nvgMoveTo(vg, tx + i.first, ty);
            nvgLineTo(vg, tx + i.first, ty + 12);
            nvgLineTo(vg, tx + i.first + 12, ty + 6);
            nvgFillColor(vg, a(nvgRGBA(255, 255, 255, i.second)));
            nvgClosePath(vg);
            nvgFill(vg);
        }
    }

    // cache info
    osdCenterBox->frame(ctx);

    // draw video profile
    videoProfile->frame(ctx);
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

std::string VideoView::genExtraUrlParam(int progress,
                                        const std::string& audio) {
    std::string extra =
        "referrer=\"https://www.bilibili.com\",network-timeout=5";
    if (progress > 0) {
        extra += fmt::format(",start={}", progress);
    }
    if (!audio.empty()) {
        extra += fmt::format(",audio-file=\"{}\"", audio);
    }
    return extra;
}

void VideoView::setUrl(const std::string& url, int progress,
                       const std::string& audio) {
    mpvCore->setUrl(url, genExtraUrlParam(progress, audio));
}

void VideoView::setBackupUrl(const std::string& url, int progress,
                             const std::string& audio) {
    mpvCore->setBackupUrl(url, genExtraUrlParam(progress, audio));
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

void VideoView::setSpeed(float speed) { mpvCore->setSpeed(speed); }

void VideoView::setLastPlayedPosition(int64_t p) { lastPlayedPosition = p; }

int64_t VideoView::getLastPlayedPosition() const { return lastPlayedPosition; }

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

void VideoView::hideActionButtons() {
    btnDanmakuIcon->getParent()->setVisibility(brls::Visibility::GONE);
    btnDanmakuSettingIcon->getParent()->setVisibility(brls::Visibility::GONE);
    btnSettingIcon->getParent()->setVisibility(brls::Visibility::GONE);
    videoSpeed->getParent()->setVisibility(brls::Visibility::GONE);
}

void VideoView::setTitle(const std::string& title) {
    this->videoTitleLabel->setText(title);
}

void VideoView::setOnlineCount(const std::string& count) {
    this->videoOnlineCountLabel->setText(count);
}

std::string VideoView::getTitle() {
    return this->videoTitleLabel->getFullText();
}

void VideoView::setQuality(const std::string& str) {
    this->videoQuality->setText(str);
}

std::string VideoView::getQuality() {
    return this->videoQuality->getFullText();
}

void VideoView::setDuration(const std::string& value) {
    this->rightStatusLabel->setText(value);
}

void VideoView::setPlaybackTime(const std::string& value) {
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
    if (isnan(value)) return;
    this->osdSlider->setProgress(value);
}

float VideoView::getProgress() { return this->osdSlider->getProgress(); }

void VideoView::showHint(const std::string& value) {
    brls::Logger::debug("Video hint: {}", value);
    this->hintLabel->setText(value);
    this->hintBox->setVisibility(brls::Visibility::VISIBLE);
    this->hintLastShowTime = wiliwili::unix_time() + VideoView::OSD_SHOW_TIME;
    this->showOSD();
}

void VideoView::clearHint() {
    this->hintBox->setVisibility(brls::Visibility::GONE);
}

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
        video->videoSpeed->setText(this->videoSpeed->getFullText());
        video->setDuration(this->rightStatusLabel->getFullText());
        video->setPlaybackTime(this->leftStatusLabel->getFullText());
        video->setProgress(this->getProgress());
        if (this->hintBox->getVisibility() == brls::Visibility::VISIBLE)
            video->showHint(this->hintLabel->getFullText());
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

            // 同时点击全屏按钮和评论会导致评论弹出在 BasePlayerActivity 和 videoView 之间，
            // 因此目前需要遍历全部的 activity 找到 BasePlayerActivity
            auto activityStack = Application::getActivitiesStack();
            if (activityStack.size() <= 2) {
                brls::Application::popActivity(brls::TransitionAnimation::NONE);
                return;
            }
            for (size_t i = activityStack.size() - 2; i != 0; i--) {
                auto* last =
                    dynamic_cast<BasePlayerActivity*>(activityStack[i]);
                if (!last) continue;
                auto* video = dynamic_cast<VideoView*>(
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
                    video->videoSpeed->setText(this->videoSpeed->getFullText());
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
                break;
            }
            // Pop fullscreen videoView
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
        });
    }
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
    // 获取按键数据
    ControllerState state{};
    input->updateUnifiedControllerState(&state);

    // 当OSD显示时上下左右切换选择按钮，持续显示OSD
    if (isOSDShown() &&
        (state.buttons[BUTTON_NAV_RIGHT] || state.buttons[BUTTON_NAV_LEFT] ||
         state.buttons[BUTTON_NAV_UP] || state.buttons[BUTTON_NAV_DOWN])) {
        if (this->osd_state == OSDState::SHOWN) this->showOSD(true);
    }

    static int click_state        = ClickState::IDLE;
    static int64_t rsb_press_time = 0;
    if (click_state == ClickState::IDLE && !state.buttons[BUTTON_RSB]) return;

    int CHECK_TIME = 200000;
    float SPEED =
        MPVCore::VIDEO_SPEED == 100 ? 2.0 : MPVCore::VIDEO_SPEED * 0.01f;

    switch (click_state) {
        case ClickState::IDLE:
            if (state.buttons[BUTTON_RSB]) {
                setSpeed(SPEED);
                rsb_press_time = getCPUTimeUsec();
                click_state    = ClickState::PRESS;
                // 绘制临时加速标识
                speedHintLabel->setText(
                    fmt::format("wiliwili/player/current_speed"_i18n, SPEED));
                speedHintBox->setVisibility(brls::Visibility::VISIBLE);
            }
            break;
        case ClickState::PRESS:
            if (!state.buttons[BUTTON_RSB]) {
                setSpeed(1.0f);
                int64_t current_time = getCPUTimeUsec();
                if (current_time - rsb_press_time < CHECK_TIME) {
                    // 点击事件
                    rsb_press_time = current_time;
                    click_state    = ClickState::FAST_RELEASE;
                } else {
                    click_state = ClickState::IDLE;
                }
                speedHintBox->setVisibility(brls::Visibility::GONE);
            }
            break;
        case ClickState::FAST_RELEASE:
            if (state.buttons[BUTTON_RSB]) {
                setSpeed(SPEED);
                int64_t current_time = getCPUTimeUsec();
                if (current_time - rsb_press_time < CHECK_TIME) {
                    rsb_press_time = current_time;
                    click_state    = ClickState::FAST_PRESS;
                } else {
                    rsb_press_time = current_time;
                    click_state    = ClickState::PRESS;
                }
                // 绘制临时加速标识
                speedHintLabel->setText(
                    fmt::format("wiliwili/player/current_speed"_i18n, SPEED));
                speedHintBox->setVisibility(brls::Visibility::VISIBLE);
            }
            break;
        case ClickState::FAST_PRESS:
            if (!state.buttons[BUTTON_RSB]) {
                int64_t current_time = getCPUTimeUsec();
                if (current_time - rsb_press_time < CHECK_TIME) {
                    rsb_press_time = current_time;
                    // 双击事件
                    setSpeed(SPEED);
                    click_state = ClickState::CLICK_DOUBLE;
                } else {
                    setSpeed(1.0f);
                    click_state = ClickState::IDLE;
                }
                speedHintBox->setVisibility(brls::Visibility::GONE);
            }
            break;
        case ClickState::CLICK_DOUBLE:
            brls::Logger::debug("speed lock: {}", SPEED);
            click_state = ClickState::IDLE;
            break;
        default:
            break;
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
                    this->hideLoading();
                    this->showOSD(false);
                    break;
                case MpvEventEnum::MPV_LOADED:
                    this->setPlaybackTime(
                        wiliwili::sec2Time(this->mpvCore->video_progress));
                    if (lastPlayedPosition > 0) {
                        this->showHint(fmt::format(
                            "已为您定位至: {}",
                            wiliwili::sec2Time(lastPlayedPosition)));
                        mpvCore->seek(lastPlayedPosition);
                        brls::Logger::info("Restore video position: {}",
                                           lastPlayedPosition);
                    }
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
                case MpvEventEnum::VIDEO_SPEED_CHANGE:
                    if (fabs(mpvCore->video_speed - 1) < 1e-5) {
                        this->videoSpeed->setText("wiliwili/player/speed"_i18n);
                    } else {
                        this->videoSpeed->setText(
                            fmt::format("{}x", mpvCore->video_speed));
                    }
                    break;
                case MpvEventEnum::END_OF_FILE:
                    // 播放结束自动取消全屏
                    this->showOSD(false);
                    this->btnToggleIcon->setImageFromSVGRes(
                        "svg/bpx-svg-sprite-play.svg");
                    if (EXIT_FULLSCREEN_ON_END && this->isFullscreen()) {
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
