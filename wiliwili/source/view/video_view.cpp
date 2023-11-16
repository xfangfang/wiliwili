//
// Created by fang on 2022/4/23.
//

#include <limits>

#include "pystring.h"
#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "view/live_core.hpp"
#include "view/subtitle_core.hpp"
#include "view/video_progress_slider.hpp"
#include "view/svg_image.hpp"
#include "view/grid_dropdown.hpp"
#include "view/video_profile.hpp"
#include "view/danmaku_core.hpp"
#include "utils/number_helper.hpp"
#include "utils/config_helper.hpp"
#include "utils/string_helper.hpp"
#include "activity/player_activity.hpp"
#include "fragment/player_danmaku_setting.hpp"
#include "fragment/player_setting.hpp"
#include "fragment/player_dlna_search.hpp"

using namespace brls;

enum ClickState {
    IDLE         = 0,
    PRESS        = 1,
    FAST_RELEASE = 3,
    FAST_PRESS   = 4,
    CLICK_DOUBLE = 5
};

static int64_t getSeekRange(int64_t current) {
    current = abs(current);
    if (current < 60) return 5;
    if (current < 300) return 10;
    if (current < 600) return 20;
    if (current < 1200) return 60;
    return current / 15;
}

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
            seeking_range -= getSeekRange(seeking_range);
            this->requestSeeking();
            return true;
        },
        false, true);

    this->registerAction(
        "\uE08E", brls::ControllerButton::BUTTON_RB,
        [this](brls::View* view) -> bool {
            ControllerState state{};
            input->updateUnifiedControllerState(&state);
            bool buttonY = brls::Application::isSwapInputKeys()
                               ? state.buttons[BUTTON_X]
                               : state.buttons[BUTTON_Y];
            if (buttonY) {
                seeking_range -= getSeekRange(seeking_range);
            } else {
                seeking_range += getSeekRange(seeking_range);
            }
            this->requestSeeking();
            return true;
        },
        false, true);

    this->registerAction(
        "toggleOSD", brls::ControllerButton::BUTTON_Y,
        [this](brls::View* view) -> bool {
            // 拖拽进度时不要影响显示 OSD
            if (is_seeking) return true;
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
        mpvCore->seekPercent(progress);
    });

    osdSlider->getProgressEvent()->subscribe([this](float progress) {
        this->showOSD(false);
        leftStatusLabel->setText(
            wiliwili::sec2Time(mpvCore->duration * progress));
    });

    /// 组件触摸事件
    /// 单击控制 OSD
    /// 双击控制播放与暂停
    /// 长按加速
    //todo 滑动调整进度
    this->addGestureRecognizer(new brls::TapGestureRecognizer(
        [this](brls::TapGestureStatus status, brls::Sound* soundToPlay) {
            brls::Application::giveFocus(this);
            static size_t iter = 0;
            // 当长按时已经加速，则忽视此次加速
            static bool ignoreSpeed = false;
            switch (status.state) {
                case brls::GestureState::UNSURE: {
                    if (isLiveMode) break;
                    // 长按加速
                    if (fabs(mpvCore->getSpeed() - 1) > 10e-2) {
                        ignoreSpeed = true;
                        break;
                    }
                    ignoreSpeed = false;
                    brls::cancelDelay(iter);
                    ASYNC_RETAIN
                    iter = brls::delay(200, [ASYNC_TOKEN]() {
                        ASYNC_RELEASE
                        float SPEED = MPVCore::VIDEO_SPEED == 100
                                          ? 2.0
                                          : MPVCore::VIDEO_SPEED * 0.01f;
                        this->setSpeed(SPEED);
                        // 绘制临时加速标识
                        this->speedHintLabel->setText(wiliwili::format(
                            "wiliwili/player/current_speed"_i18n, SPEED));
                        this->speedHintBox->setVisibility(
                            brls::Visibility::VISIBLE);
                    });
                    break;
                }
                case brls::GestureState::FAILED:
                case brls::GestureState::INTERRUPTED: {
                    // 打断加速
                    if (!ignoreSpeed) {
                        brls::cancelDelay(iter);
                        this->setSpeed(1.0f);
                        this->speedHintBox->setVisibility(
                            brls::Visibility::GONE);
                    }
                    break;
                }
                case brls::GestureState::END: {
                    // 打断加速
                    if (!ignoreSpeed) {
                        brls::cancelDelay(iter);
                        this->setSpeed(1.0f);
                        if (this->speedHintBox->getVisibility() ==
                            brls::Visibility::VISIBLE) {
                            this->speedHintBox->setVisibility(
                                brls::Visibility::GONE);
                            // 正在加速时抬起手指，不触发后面 OSD 相关内容，直接结束此次事件
                            break;
                        }
                    }

                    // 处理点击事件
                    static int click_state    = ClickState::IDLE;
                    static int64_t press_time = 0;

                    int CHECK_TIME         = 200000;
                    static size_t tap_iter = 0;
                    switch (click_state) {
                        case ClickState::IDLE: {
                            press_time  = getCPUTimeUsec();
                            click_state = ClickState::CLICK_DOUBLE;
                            // 单击切换 OSD，设置一个延迟用来等待双击结果
                            ASYNC_RETAIN
                            tap_iter = brls::delay(200, [ASYNC_TOKEN]() {
                                ASYNC_RELEASE
                                this->toggleOSD();
                            });
                            break;
                        }
                        case ClickState::CLICK_DOUBLE: {
                            brls::cancelDelay(tap_iter);
                            int64_t current_time = getCPUTimeUsec();
                            if (current_time - press_time < CHECK_TIME) {
                                // 双击切换播放状态
                                togglePlay();
                                click_state = ClickState::IDLE;
                            } else {
                                // 单击切换 OSD，设置一个延迟用来等待双击结果
                                press_time  = getCPUTimeUsec();
                                click_state = ClickState::CLICK_DOUBLE;
                                ASYNC_RETAIN
                                tap_iter = brls::delay(200, [ASYNC_TOKEN]() {
                                    ASYNC_RELEASE
                                    this->toggleOSD();
                                });
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
                default:
                    break;
            }
        }));

    /// 播放/暂停 按钮
    this->btnToggle->addGestureRecognizer(new brls::TapGestureRecognizer(
        this->btnToggle, [this]() { this->togglePlay(); },
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
    this->btnSettingIcon->getParent()->registerClickAction([this](...) {
        auto setting = new PlayerSetting();
        // 不显示弹幕则认为不是在播放B站视频，此时隐藏设置菜单中的上传历史记录
        if (!showHistorySetting) {
            setting->hideHistoryCell();
        }
        if (!showVideoRelatedSetting) {
            setting->hideVideoRelatedCells();
        }
        if (!showSubtitleSetting) {
            setting->hideSubtitleCells();
        }
        if (!showBottomLineSetting) {
            setting->hideBottomLineCells();
        }
        brls::Application::pushActivity(new Activity(setting));
        // 手动将焦点赋给设置页面
        brls::sync([setting]() { brls::Application::giveFocus(setting); });
        GA("open_player_setting")
        return true;
    });
    this->btnSettingIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnSettingIcon->getParent()));

    /// 音量按钮
    this->btnVolumeIcon->getParent()->registerClickAction(
        [this](brls::View* view) {
            // 一直显示 OSD
            this->showOSD(false);
            auto theme     = brls::Application::getTheme();
            auto container = new brls::Box();
            container->setHideClickAnimation(true);
            container->addGestureRecognizer(
                new brls::TapGestureRecognizer(container, [this, container]() {
                    // 几秒后自动关闭 OSD
                    this->showOSD(true);
                    container->dismiss();
                    // 保存结果
                    ProgramConfig::instance().setSettingItem(
                        SettingItem::PLAYER_VOLUME, MPVCore::VIDEO_VOLUME);
                }));
            // 滑动条背景
            auto sliderBox = new brls::Box();
            sliderBox->setAlignItems(brls::AlignItems::CENTER);
            sliderBox->setHeight(60);
            sliderBox->setCornerRadius(4);
            sliderBox->setBackgroundColor(theme.getColor("color/grey_1"));
            float sliderX = view->getX() - 120;
            if (sliderX < 0) sliderX = 20;
            if (sliderX > brls::Application::ORIGINAL_WINDOW_WIDTH - 332)
                sliderX = brls::Application::ORIGINAL_WINDOW_WIDTH - 332;
            sliderBox->setTranslationX(sliderX);
            sliderBox->setTranslationY(view->getY() - 70);
            // 滑动条
            auto slider = new brls::Slider();
            slider->setMargins(8, 16, 8, 16);
            slider->setWidth(300);
            slider->setHeight(40);
            slider->setProgress(MPVCore::instance().getVolume() * 1.0 / 100);
            slider->getProgressEvent()->subscribe([](float progress) {
                MPVCore::instance().setVolume(progress * 100);
            });
            sliderBox->addView(slider);
            container->addView(sliderBox);
            auto frame = new AppletFrame(container);
            frame->setInFadeAnimation(true);
            frame->setHeaderVisibility(brls::Visibility::GONE);
            frame->setFooterVisibility(brls::Visibility::GONE);
            frame->setBackgroundColor(theme.getColor("brls/backdrop"));
            container->registerAction(
                "hints/back"_i18n, BUTTON_B, [this, container](...) {
                    // 几秒后自动关闭 OSD
                    this->showOSD(true);
                    container->dismiss();
                    // 保存结果
                    ProgramConfig::instance().setSettingItem(
                        SettingItem::PLAYER_VOLUME, MPVCore::VIDEO_VOLUME);
                    return true;
                });
            brls::Application::pushActivity(new Activity(frame));

            // 手动将焦点赋给音量组件
            brls::sync(
                [container]() { brls::Application::giveFocus(container); });
            return true;
        });
    this->btnVolumeIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnVolumeIcon->getParent()));
    if (mpvCore->volume <= 0) {
        this->btnVolumeIcon->setImageFromSVGRes(
            "svg/bpx-svg-sprite-volume-off.svg");
    }

    /// 投屏按钮
    this->btnCastIcon->getParent()->registerClickAction([this](...) {
        this->pause();

        auto dlna = new PlayerDlnaSearch();
        brls::Application::pushActivity(new brls::Activity(dlna));

        // 手动将焦点赋给设置页面
        brls::sync([dlna]() { brls::Application::giveFocus(dlna); });
        GA("open_player_cast")
        return true;
    });
    this->btnCastIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnCastIcon->getParent()));

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

void VideoView::requestSeeking() {
    if (mpvCore->duration <= 0) {
        seeking_range = 0;
        is_seeking    = false;
        return;
    }
    double progress =
        (this->mpvCore->playback_time + seeking_range) / mpvCore->duration;

    if (progress < 0) {
        progress      = 0;
        seeking_range = (int64_t)this->mpvCore->playback_time * -1;
    } else if (progress > 1) {
        progress      = 1;
        seeking_range = mpvCore->duration;
    }

    showOSD(false);
    osdSlider->setProgress((float)progress);
    leftStatusLabel->setText(wiliwili::sec2Time(mpvCore->duration * progress));
    is_seeking = true;

    // 延迟触发跳转进度
    brls::cancelDelay(seeking_iter);
    ASYNC_RETAIN
    seeking_iter = brls::delay(400, [ASYNC_TOKEN]() {
        ASYNC_RELEASE
        mpvCore->seekRelative(seeking_range);
        seeking_range = 0;
        is_seeking    = false;
    });
}

void VideoView::disableDimming(bool disable) {
    brls::Application::getPlatform()->disableScreenDimming(
        disable, "Playing video", APPVersion::getPackageName());
    static bool deactivationAvailable =
        ProgramConfig::instance().getSettingItem(SettingItem::DEACTIVATED_TIME,
                                                 0) > 0;
    if (deactivationAvailable) {
        brls::Application::setAutomaticDeactivation(!disable);
    }
}

VideoView::~VideoView() {
    brls::Logger::debug("trying delete VideoView...");
    this->unRegisterMpvEvent();
    MPV_CE->unsubscribe(customEventSubscribeID);
    disableDimming(false);
    brls::Logger::debug("Delete VideoView done");
}

void VideoView::draw(NVGcontext* vg, float x, float y, float width,
                     float height, Style style, FrameContext* ctx) {
    if (!mpvCore->isValid()) return;
    float alpha = this->getAlpha();

    // draw video
    mpvCore->draw(brls::Rect(x, y, width, height), alpha);

    // draw bottom bar
    if (BOTTOM_BAR && showBottomLineSetting) {
        bottomBarColor.a = alpha;
        nvgFillColor(vg, bottomBarColor);
        nvgBeginPath(vg);
        nvgRect(vg, x, y + height - 2, width * mpvCore->percent_pos / 100, 2);
        nvgFill(vg);
    }

    // draw danmaku
    if (showDanmaku) {
        isLiveMode
            ? LiveDanmakuCore::instance().draw(vg, x, y, width, height, alpha)
            : DanmakuCore::instance().draw(vg, x, y, width, height, alpha);
    }

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
        if (showSubtitleSetting)
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
        if (showSubtitleSetting)
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
        float ty                              = frame.getMinY() + 4.5;
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

void VideoView::onLayout() { brls::View::onLayout(); }

std::string VideoView::genExtraUrlParam(int progress,
                                        const std::string& audio) {
    std::vector<std::string> audios;
    if (!audio.empty()) {
        audios.emplace_back(audio);
    }
    return genExtraUrlParam(progress, audios);
}

std::string VideoView::genExtraUrlParam(
    int progress, const std::vector<std::string>& audios) {
    std::string extra =
#ifdef __PSV__
        "referrer=\"https://www.bilibili.com\",network-timeout=10";
#else
        "referrer=\"https://www.bilibili.com\",network-timeout=5";
#endif
    if (progress > 0) {
        extra += fmt::format(",start={}", progress);
    }
    for (auto& audio : audios)
        extra += fmt::format(",audio-file=\"{}\"", audio);
    return extra;
}

void VideoView::setUrl(const std::string& url, int progress,
                       const std::string& audio) {
    std::vector<std::string> audios;
    if (!audio.empty()) {
        audios.emplace_back(audio);
    }
    setUrl(url, progress, audios);
}

void VideoView::setUrl(const std::string& url, int progress,
                       const std::vector<std::string>& audios) {
    mpvCore->setUrl(url, genExtraUrlParam(progress, audios));
}

void VideoView::setBackupUrl(const std::string& url, int progress,
                             const std::string& audio) {
    setBackupUrl(url, progress, std::vector{audio});
}

void VideoView::setBackupUrl(const std::string& url, int progress,
                             const std::vector<std::string>& audios) {
    mpvCore->setBackupUrl(url, genExtraUrlParam(progress, audios));
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
    if (customToggleAction != nullptr) return customToggleAction();
    if (this->mpvCore->isPaused()) {
        this->mpvCore->resume();
    } else {
        this->mpvCore->pause();
    }
}

void VideoView::setCustomToggleAction(std::function<void()> action) {
    this->customToggleAction = action;
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
#ifdef __WINRT__
        this->osdLastShowTime = 0xffffffff;
#else
        this->osdLastShowTime = (std::numeric_limits<std::time_t>::max)();
#endif
        this->osd_state = OSDState::ALWAYS_ON;
    }
}

void VideoView::hideOSD() {
    this->osdLastShowTime = 0;
    this->osd_state       = OSDState::HIDDEN;
}

bool VideoView::isOSDShown() const { return this->is_osd_shown; }

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
    disableDimming(false);
}

void VideoView::hideLoading() {
    osdCenterBox->setVisibility(brls::Visibility::GONE);
    disableDimming(true);
}

void VideoView::hideDanmakuButton() {
    showDanmaku = false;
    btnDanmakuIcon->setVisibility(brls::Visibility::GONE);
    btnDanmakuIcon->getParent()->setVisibility(brls::Visibility::GONE);
    btnDanmakuSettingIcon->setVisibility(brls::Visibility::GONE);
    btnDanmakuSettingIcon->getParent()->setVisibility(brls::Visibility::GONE);
}

void VideoView::hideDLNAButton() {
    btnCastIcon->setVisibility(brls::Visibility::GONE);
    btnCastIcon->getParent()->setVisibility(brls::Visibility::GONE);
}

void VideoView::hideVideoQualityButton() {
    videoQuality->setVisibility(brls::Visibility::GONE);
    videoQuality->getParent()->setVisibility(brls::Visibility::GONE);
}

void VideoView::hideVideoSpeedButton() {
    videoSpeed->setVisibility(brls::Visibility::GONE);
    videoSpeed->getParent()->setVisibility(brls::Visibility::GONE);
}

void VideoView::hideStatusLabel() {
    leftStatusLabel->setVisibility(brls::Visibility::GONE);
    centerStatusLabel->setVisibility(brls::Visibility::GONE);
    rightStatusLabel->setVisibility(brls::Visibility::GONE);
}

void VideoView::setLiveMode() {
    isLiveMode = true;
    centerStatusLabel->setVisibility(brls::Visibility::GONE);
    rightStatusLabel->setVisibility(brls::Visibility::GONE);
}

void VideoView::setStatusLabelLeft(const std::string& value) {
    leftStatusLabel->setText(value);
}

void VideoView::setStatusLabelRight(const std::string& value) {
    rightStatusLabel->setText(value);
}

void VideoView::disableCloseOnEndOfFile() { closeOnEndOfFile = false; }

void VideoView::hideHistorySetting() { showHistorySetting = false; }

void VideoView::hideVideoRelatedSetting() { showVideoRelatedSetting = false; }

void VideoView::hideSubtitleSetting() { showSubtitleSetting = false; }

void VideoView::hideBottomLineSetting() { showBottomLineSetting = false; }

void VideoView::hideVideoProgressSlider() {
    osdSlider->setVisibility(brls::Visibility::GONE);
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
    if (this->is_seeking) return;
    if (this->isLiveMode) return;
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

brls::View* VideoView::getFullscreenIcon() { return btnFullscreenIcon; }

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
    if (mpvCore->isPaused() || mpvCore->isStopped()) {
        btnToggleIcon->setImageFromSVGRes("svg/bpx-svg-sprite-play.svg");
    } else {
        btnToggleIcon->setImageFromSVGRes("svg/bpx-svg-sprite-pause.svg");
    }
}

void VideoView::setProgress(float value) {
    if (is_seeking) return;
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
        if (video->isLiveMode) video->setLiveMode();
        video->setCustomToggleAction(customToggleAction);
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
                brls::Application::popActivity();
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

#ifndef __PSV__

    static int click_state        = ClickState::IDLE;
    static int64_t rsb_press_time = 0;
    if (isLiveMode) return;
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
                speedHintLabel->setText(wiliwili::format(
                    "wiliwili/player/current_speed"_i18n, SPEED));
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
                speedHintLabel->setText(wiliwili::format(
                    "wiliwili/player/current_speed"_i18n, SPEED));
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

#endif
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
                    refreshToggleIcon();
                    break;
                case MpvEventEnum::MPV_PAUSE:
                    this->showOSD(false);
                    disableDimming(false);
                    refreshToggleIcon();
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
                    disableDimming(false);
                    this->btnToggleIcon->setImageFromSVGRes(
                        "svg/bpx-svg-sprite-play.svg");
                    if (EXIT_FULLSCREEN_ON_END && closeOnEndOfFile &&
                        this->isFullscreen()) {
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
                case MpvEventEnum::VIDEO_MUTE:
                    this->btnVolumeIcon->setImageFromSVGRes(
                        "svg/bpx-svg-sprite-volume-off.svg");
                    break;
                case MpvEventEnum::VIDEO_UNMUTE:
                    this->btnVolumeIcon->setImageFromSVGRes(
                        "svg/bpx-svg-sprite-volume.svg");
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
