//
// Created by fang on 2022/4/23.
//

#include <limits>

#include <borealis/views/label.hpp>
#include <borealis/views/progress_spinner.hpp>
#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>
#include <borealis/views/slider.hpp>
#include <borealis/views/applet_frame.hpp>
#include <pystring.h>

#include "utils/number_helper.hpp"
#include "utils/config_helper.hpp"
#include "utils/string_helper.hpp"
#include "utils/gesture_helper.hpp"
#include "utils/activity_helper.hpp"
#include "activity/player_activity.hpp"
#include "fragment/player_danmaku_setting.hpp"
#include "fragment/player_setting.hpp"
#include "fragment/player_dlna_search.hpp"
#include "view/video_view.hpp"
#include "view/live_core.hpp"
#include "view/subtitle_core.hpp"
#include "view/video_progress_slider.hpp"
#include "view/svg_image.hpp"
#include "view/grid_dropdown.hpp"
#include "view/video_profile.hpp"
#include "view/danmaku_core.hpp"
#include "view/mpv_core.hpp"

enum ClickState { IDLE = 0, PRESS = 1, FAST_RELEASE = 3, FAST_PRESS = 4, CLICK_DOUBLE = 5 };

static int getSeekRange(int current) {
    current = abs(current);
    if (current < 60) return 5;
    if (current < 300) return 10;
    if (current < 600) return 20;
    if (current < 1200) return 60;
    return current / 15;
}

#define CHECK_OSD(shake)                                                              \
    if (is_osd_lock) {                                                                \
        if (isOSDShown()) {                                                           \
            brls::Application::giveFocus(this->osdLockBox);                           \
            if (shake) this->osdLockBox->shakeHighlight(brls::FocusDirection::RIGHT); \
        } else {                                                                      \
            this->showOSD(true);                                                      \
        }                                                                             \
        return true;                                                                  \
    }

VideoView::VideoView() {
    mpvCore = &MPVCore::instance();
    this->inflateFromXMLRes("xml/views/video_view.xml");
    this->setHideHighlightBackground(true);
    this->setHideClickAnimation(true);

    setTvControlMode(ProgramConfig::instance().getBoolOption(SettingItem::PLAYER_OSD_TV_MODE));

    input = brls::Application::getPlatform()->getInputManager();

    this->registerBoolXMLAttribute("allowFullscreen", [this](bool value) {
        this->allowFullscreen = value;
        if (!value) {
            this->btnFullscreenIcon->getParent()->setVisibility(brls::Visibility::GONE);
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
            CHECK_OSD(true);
            seeking_range -= getSeekRange(seeking_range);
            this->requestSeeking(seeking_range);
            return true;
        },
        false, true);

    this->registerAction(
        "\uE08E", brls::ControllerButton::BUTTON_RB,
        [this](brls::View* view) -> bool {
            CHECK_OSD(true);
            brls::ControllerState state{};
            input->updateUnifiedControllerState(&state);
            bool buttonY =
                brls::Application::isSwapInputKeys() ? state.buttons[brls::BUTTON_X] : state.buttons[brls::BUTTON_Y];
            if (buttonY) {
                seeking_range -= getSeekRange(seeking_range);
            } else {
                seeking_range += getSeekRange(seeking_range);
            }
            this->requestSeeking(seeking_range);
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
            CHECK_OSD(true);
            this->toggleDanmaku();
            return true;
        },
        true);

    this->registerAction(
        "volumeUp", brls::ControllerButton::BUTTON_NAV_UP,
        [this](brls::View* view) -> bool {
            CHECK_OSD(true);
            brls::ControllerState state{};
            input->updateUnifiedControllerState(&state);
            if (state.buttons[brls::BUTTON_RT]) {
                this->requestVolume((int)MPVCore::instance().volume + 5, 400);
                return true;
            }
            return false;
        },
        true, true);

    this->registerAction(
        "volumeDown", brls::ControllerButton::BUTTON_NAV_DOWN,
        [this](brls::View* view) -> bool {
            CHECK_OSD(true);
            brls::ControllerState state{};
            input->updateUnifiedControllerState(&state);
            if (state.buttons[brls::BUTTON_RT]) {
                this->requestVolume((int)MPVCore::instance().volume - 5, 400);
                return true;
            }
            return false;
        },
        true, true);

    this->registerMpvEvent();

    osdSlider->getProgressSetEvent()->subscribe([this](float progress) {
        brls::Logger::verbose("Set progress: {}", progress);
        this->showOSD(true);
        if (real_duration > 0) {
            // 当设置了视频时长数据
            mpvCore->seek((float)real_duration * progress);
        } else {
            mpvCore->seekPercent(progress);
        }
    });

    osdSlider->getProgressEvent()->subscribe([this](float progress) {
        this->showOSD(false);
        leftStatusLabel->setText(wiliwili::sec2Time(getRealDuration() * progress));
    });

    /// 组件触摸事件
    /// 单击控制 OSD
    /// 双击控制播放与暂停
    /// 长按加速
    /// 滑动调整进度
    /// 左右侧滑动调整音量，在支持调节背光的设备上左侧滑动调节背光亮度，右侧调节音量
    this->addGestureRecognizer(new OsdGestureRecognizer([this](OsdGestureStatus status) {
        switch (status.osdGestureType) {
            case OsdGestureType::TAP:
                this->toggleOSD();
                break;
            case OsdGestureType::DOUBLE_TAP_END:
                if (is_osd_lock) {
                    this->toggleOSD();
                    break;
                }
                this->togglePlay();
                break;
            case OsdGestureType::LONG_PRESS_START: {
                if (is_osd_lock) break;
                float SPEED = MPVCore::VIDEO_SPEED == 100 ? 2.0 : MPVCore::VIDEO_SPEED * 0.01f;
                this->setSpeed(SPEED);
                // 绘制临时加速标识
                this->speedHintLabel->setText(wiliwili::format("wiliwili/player/current_speed"_i18n, SPEED));
                this->speedHintBox->setVisibility(brls::Visibility::VISIBLE);
                break;
            }
            case OsdGestureType::LONG_PRESS_CANCEL:
            case OsdGestureType::LONG_PRESS_END:
                if (is_osd_lock) {
                    this->toggleOSD();
                    break;
                }
                this->setSpeed(1.0f);
                this->speedHintBox->setVisibility(brls::Visibility::GONE);
                break;
            case OsdGestureType::HORIZONTAL_PAN_START:
                if (is_osd_lock) break;
                this->showCenterHint();
                this->setCenterHintIcon("svg/arrow-left-right.svg");
                break;
            case OsdGestureType::HORIZONTAL_PAN_UPDATE:
                if (is_osd_lock) break;
                this->requestSeeking(fmin(120.0f, getRealDuration()) * status.deltaX);
                break;
            case OsdGestureType::HORIZONTAL_PAN_CANCEL:
                if (is_osd_lock) break;
                // 立即取消
                this->requestSeeking(VIDEO_CANCEL_SEEKING, VIDEO_SEEK_IMMEDIATELY);
                break;
            case OsdGestureType::HORIZONTAL_PAN_END:
                if (is_osd_lock) {
                    this->toggleOSD();
                    break;
                }
                // 立即跳转
                this->requestSeeking(fmin(120.0f, getRealDuration()) * status.deltaX, VIDEO_SEEK_IMMEDIATELY);
                break;
            case OsdGestureType::LEFT_VERTICAL_PAN_START:
                if (is_osd_lock) break;
                if (brls::Application::getPlatform()->canSetBacklightBrightness()) {
                    this->brightness_init = brls::Application::getPlatform()->getBacklightBrightness();
                    this->showCenterHint();
                    this->setCenterHintIcon("svg/sun-fill.svg");
                    break;
                }
            case OsdGestureType::RIGHT_VERTICAL_PAN_START:
                if (is_osd_lock) break;
                this->volume_init = (int)MPVCore::instance().volume;
                this->showCenterHint();
                this->setCenterHintIcon("svg/bpx-svg-sprite-volume.svg");
                break;
            case OsdGestureType::LEFT_VERTICAL_PAN_UPDATE:
                if (is_osd_lock) break;
                if (brls::Application::getPlatform()->canSetBacklightBrightness()) {
                    this->requestBrightness(this->brightness_init + status.deltaY);
                    break;
                }
            case OsdGestureType::RIGHT_VERTICAL_PAN_UPDATE:
                if (is_osd_lock) break;
                this->requestVolume(this->volume_init + status.deltaY * 100);
                break;
            case OsdGestureType::LEFT_VERTICAL_PAN_CANCEL:
            case OsdGestureType::LEFT_VERTICAL_PAN_END:
                if (is_osd_lock) {
                    this->toggleOSD();
                    break;
                }
                if (brls::Application::getPlatform()->canSetBacklightBrightness()) {
                    this->hideCenterHint();
                    break;
                }
            case OsdGestureType::RIGHT_VERTICAL_PAN_CANCEL:
            case OsdGestureType::RIGHT_VERTICAL_PAN_END:
                if (is_osd_lock) {
                    this->toggleOSD();
                    break;
                }
                this->hideCenterHint();
                ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_VOLUME, MPVCore::VIDEO_VOLUME);
                break;
            default:
                break;
        }
    }));

    /// 播放/暂停 按钮
    this->btnToggle->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnToggle, [this]() { this->togglePlay(); }));
    this->btnToggle->registerClickAction([this](...) {
        this->togglePlay();
        return true;
    });

    /// 清晰度按钮
    this->videoQuality->getParent()->registerClickAction([](...) {
        APP_E->fire(VideoView::QUALITY_CHANGE, nullptr);
        return true;
    });
    this->videoQuality->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->videoQuality->getParent()));
    this->registerAction("wiliwili/player/quality"_i18n, brls::ControllerButton::BUTTON_START,
                         [this](brls::View* view) -> bool {
                             CHECK_OSD(true);
                             APP_E->fire(VideoView::QUALITY_CHANGE, nullptr);
                             return true;
                         });

    /// 视频详情信息
    this->registerAction(
        "profile", brls::ControllerButton::BUTTON_BACK,
        [this](brls::View* view) -> bool {
            CHECK_OSD(true);
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
        auto conf = ProgramConfig::instance().getOptionData(SettingItem::PLAYER_DEFAULT_SPEED);

        // 找到当前倍速对应的列表值
        double speed      = MPVCore::instance().getSpeed();
        int selectedIndex = (int)ProgramConfig::instance().getIntOptionIndex(SettingItem::PLAYER_DEFAULT_SPEED);
        for (size_t i = 0; i < conf.rawOptionList.size(); i++) {
            if (fabs(conf.rawOptionList[i] * 0.01 - speed) < 1e-5) {
                selectedIndex = (int)i;
                break;
            }
        }

        // 展示倍速列表
        BaseDropdown::text(
            "wiliwili/player/speed"_i18n, conf.optionList,
            [conf](int selected) {
                // 设置播放器倍速
                MPVCore::instance().setSpeed(conf.rawOptionList[selected] * 0.01);
                // 保存下倍速非1时的值，快捷键触发时使用此值
                if (conf.rawOptionList[selected] != 100) {
                    MPVCore::VIDEO_SPEED = conf.rawOptionList[selected];
                    ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_DEFAULT_SPEED, MPVCore::VIDEO_SPEED);
                }
            },
            selectedIndex);

        return true;
    });
    this->videoSpeed->getParent()->addGestureRecognizer(new brls::TapGestureRecognizer(this->videoSpeed->getParent()));

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
        brls::Application::pushActivity(new brls::Activity(setting));
        GA("open_danmaku_setting")
        return true;
    });
    this->btnDanmakuSettingIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnDanmakuSettingIcon->getParent()));

    /// 播放器设置按钮
    this->btnSettingIcon->getParent()->registerClickAction([this](...) {
        auto setting = new PlayerSetting();

        setting->setBangumiCustomSetting(bangumiTitle, bangumiSeasonId);

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
        if (!showHighlightLineSetting) {
            setting->hideHighlightLineCells();
        }
        if (!showOpeningCreditsSetting) {
            setting->hideSkipOpeningCreditsSetting();
        }
        brls::Application::pushActivity(new brls::Activity(setting));
        GA("open_player_setting")
        return true;
    });
    this->btnSettingIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnSettingIcon->getParent()));

    /// 音量按钮
    this->btnVolumeIcon->getParent()->registerClickAction([this](brls::View* view) {
        // 一直显示 OSD
        this->showOSD(false);
        auto theme     = brls::Application::getTheme();
        auto container = new brls::Box();
        container->setHideClickAnimation(true);
        container->addGestureRecognizer(new brls::TapGestureRecognizer(container, [this, container]() {
            // 几秒后自动关闭 OSD
            this->showOSD(true);
            container->dismiss();
            // 保存结果
            ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_VOLUME, MPVCore::VIDEO_VOLUME);
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
        slider->getProgressEvent()->subscribe([](float progress) { MPVCore::instance().setVolume(progress * 100); });
        sliderBox->addView(slider);
        container->addView(sliderBox);
        auto frame = new brls::AppletFrame(container);
        frame->setInFadeAnimation(true);
        frame->setHeaderVisibility(brls::Visibility::GONE);
        frame->setFooterVisibility(brls::Visibility::GONE);
        frame->setBackgroundColor(theme.getColor("brls/backdrop"));
        container->registerAction("hints/back"_i18n, brls::BUTTON_B, [this, container](...) {
            // 几秒后自动关闭 OSD
            this->showOSD(true);
            container->dismiss();
            // 保存结果
            ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_VOLUME, MPVCore::VIDEO_VOLUME);
            return true;
        });
        brls::Application::pushActivity(new brls::Activity(frame));
        return true;
    });
    this->btnVolumeIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnVolumeIcon->getParent()));
    if (mpvCore->volume <= 0) {
        this->btnVolumeIcon->setImageFromSVGRes("svg/bpx-svg-sprite-volume-off.svg");
    }

    /// 投屏按钮
    this->btnCastIcon->getParent()->registerClickAction([this](...) {
        this->pause();

        auto dlna = new PlayerDlnaSearch();
        brls::Application::pushActivity(new brls::Activity(dlna));

        GA("open_player_cast")
        return true;
    });
    this->btnCastIcon->getParent()->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->btnCastIcon->getParent()));

    /// OSD 锁定按钮
    this->osdLockBox->registerClickAction([this](...) {
        this->toggleOSDLock();
        return true;
    });
    this->osdLockBox->addGestureRecognizer(new brls::TapGestureRecognizer(this->osdLockBox));

    this->refreshDanmakuIcon();

    this->registerAction(
        "cancel", brls::ControllerButton::BUTTON_B,
        [this](brls::View* view) -> bool {
            if (is_osd_lock) {
                this->toggleOSD();
                return true;
            }
            if (this->isFullscreen()) {
                if (isTvControlMode && isOSDShown()) {
                    this->toggleOSD();
                    return true;
                }
                this->setFullScreen(false);
            } else {
                this->dismiss();
            }
            return true;
        },
        true);

    this->registerAction("wiliwili/player/fs"_i18n, brls::ControllerButton::BUTTON_A, [this](brls::View* view) {
        CHECK_OSD(false);
        if (this->isFullscreen()) {
            this->showOSD(true);
            if (isTvControlMode) {
                // 焦点设置在默认位置
                brls::sync([this]() { brls::Application::giveFocus(this); });
            } else {
                // 直接切换播放状态
                this->togglePlay();
            }
        } else {
            //非全屏状态点击视频组件进入全屏
            this->setFullScreen(true);
        }
        return true;
    });

    // 自定义的mpv事件
    customEventSubscribeID = APP_E->subscribe([this](const std::string& event, void* data) {
        if (event == VideoView::SET_TITLE) {
            this->setTitle((const char*)data);
        } else if (event == VideoView::SET_ONLINE_NUM) {
            this->setOnlineCount((const char*)data);
        } else if (event == VideoView::SET_QUALITY) {
            this->setQuality((const char*)data);
        } else if (event == VideoView::REAL_DURATION) {
            this->real_duration = *(int*)data;
            this->setDuration(wiliwili::sec2Time(real_duration));
            this->setProgress((float)mpvCore->playback_time / (float)real_duration);
        } else if (event == VideoView::LAST_TIME) {
            if (*(int64_t*)data == VideoView::POSITION_DISCARD)
                this->setLastPlayedPosition(VideoView::POSITION_DISCARD);
            else if (this->getLastPlayedPosition() != VideoView::POSITION_DISCARD) {
                int64_t p = *(int64_t*)data / 1000;
                this->setLastPlayedPosition(p);
                mpvCore->seek(p);
            }
        } else if (event == VideoView::HINT) {
            this->showHint((const char*)data);
        } else if (event == VideoView::CLIP_INFO) {
            osdSlider->addClipPoint(*(float*)data);
        } else if (event == VideoView::HIGHLIGHT_INFO) {
            this->setHighlightProgress(*(VideoHighlightData*)data);
        } else if (event == VideoView::REPLAY) {
            // 显示重播按钮
            showReplay = true;
            this->refreshToggleIcon();
        }
    });
}

void VideoView::requestVolume(int volume, int delay) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    MPVCore::instance().setVolume(volume);
    setCenterHintText(fmt::format("{} %", volume));
    if (delay == 0) return;
    if (volume_iter == 0) {
        this->showCenterHint();
        this->setCenterHintIcon("svg/bpx-svg-sprite-volume.svg");
    } else {
        brls::cancelDelay(volume_iter);
    }
    ASYNC_RETAIN
    volume_iter = brls::delay(delay, [ASYNC_TOKEN]() {
        ASYNC_RELEASE
        this->hideCenterHint();
        ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_VOLUME, MPVCore::VIDEO_VOLUME);
        this->volume_iter = 0;
    });
}

void VideoView::requestBrightness(float brightness) {
    if (brightness < 0) brightness = 0.0f;
    if (brightness > 1) brightness = 1.0f;
    brls::Application::getPlatform()->setBacklightBrightness(brightness);
    setCenterHintText(fmt::format("{} %", (int)(brightness * 100)));
}

void VideoView::requestSeeking(int seek, int delay) {
    if (getRealDuration() <= 0) {
        seeking_range = 0;
        is_seeking    = false;
        return;
    }
    double progress = (this->mpvCore->playback_time + seek) / getRealDuration();

    if (progress < 0) {
        progress = 0;
        seek     = (int64_t)this->mpvCore->playback_time * -1;
    } else if (progress > 1) {
        progress = 1;
        seek     = getRealDuration();
    }

    showOSD(false);
    if (osdCenterBox2->getVisibility() != brls::Visibility::VISIBLE) {
        showCenterHint();
        setCenterHintIcon("svg/arrow-left-right.svg");
    }
    setCenterHintText(fmt::format("{:+d} s", seek));
    osdSlider->setProgress((float)progress);
    leftStatusLabel->setText(wiliwili::sec2Time(getRealDuration() * progress));

    // 取消之前的延迟触发
    brls::cancelDelay(seeking_iter);
    if (delay <= 0) {
        this->hideCenterHint();
        seeking_range = 0;
        is_seeking    = false;
        if (seek == 0) return;
        mpvCore->seekRelative(seek);
    } else {
        // 延迟触发跳转进度
        is_seeking = true;
        ASYNC_RETAIN
        seeking_iter = brls::delay(delay, [ASYNC_TOKEN, seek]() {
            ASYNC_RELEASE
            this->hideCenterHint();
            seeking_range = 0;
            is_seeking    = false;
            if (seek == 0) return;
            mpvCore->seekRelative(seek);
        });
    }
}

VideoView::~VideoView() {
    brls::Logger::debug("trying delete VideoView...");
    this->unRegisterMpvEvent();
    APP_E->unsubscribe(customEventSubscribeID);
    brls::Logger::debug("Delete VideoView done");
}

void VideoView::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                     brls::FrameContext* ctx) {
    if (!mpvCore->isValid()) return;
    float alpha    = this->getAlpha();
    time_t current = wiliwili::unix_time();
    bool drawOSD   = current < this->osdLastShowTime;

    // draw video
    mpvCore->draw(brls::Rect(x, y, width, height), alpha);

    // draw highlight progress
    // OSD绘制时，进度条也包含了高能进度条，避免太杂乱仅在不显示 OSD 时绘制
    if (HIGHLIGHT_PROGRESS_BAR && (!drawOSD || is_osd_lock)) {
        drawHighlightProgress(vg, x, y + height, width, alpha);
    }

    // draw bottom bar
    if (BOTTOM_BAR && showBottomLineSetting) {
        bottomBarColor.a = alpha;
        float progress   = mpvCore->playback_time / getRealDuration();
        progress         = progress > 1.0f ? 1.0f : progress;
        nvgFillColor(vg, bottomBarColor);
        nvgBeginPath(vg);
        nvgRect(vg, x, y + height - 2, width * progress, 2);
        nvgFill(vg);
    }

    // draw danmaku
    if (enableDanmaku) {
        isLiveMode ? LiveDanmakuCore::instance().draw(vg, x, y, width, height, alpha)
                   : DanmakuCore::instance().draw(vg, x, y, width, height, alpha);
    }

    // draw osd
    if (drawOSD) {
        if (!is_osd_shown) {
            is_osd_shown = true;
            this->onOSDStateChanged(true);
        }

        // 当 osd 锁定时，只显示锁定按钮
        if (!is_osd_lock) {
            // draw highlight progress
            auto sliderRect = osdSlider->getFrame();
            drawHighlightProgress(vg, sliderRect.getMinX() + 30, sliderRect.getMinY() + 25, sliderRect.getWidth() - 60,
                                  alpha);

            // draw osd
            osdTopBox->setVisibility(brls::Visibility::VISIBLE);
            osdBottomBox->setVisibility(brls::Visibility::VISIBLE);
            osdBottomBox->frame(ctx);
            osdTopBox->frame(ctx);
        }
        // draw osd lock button
        if (!hide_lock_button) {
            osdLockBox->setVisibility(brls::Visibility::VISIBLE);
            osdLockBox->frame(ctx);
        }
    } else {
        if (is_osd_shown) {
            is_osd_shown = false;
            this->onOSDStateChanged(false);
        }
        osdTopBox->setVisibility(brls::Visibility::INVISIBLE);
        osdBottomBox->setVisibility(brls::Visibility::INVISIBLE);
        osdLockBox->setVisibility(brls::Visibility::INVISIBLE);
    }

    // draw subtitle
    // 在正常显示 osd 时，将字幕向上偏移，避免被 OSD 挡住
    if (showSubtitleSetting)
        SubtitleCore::instance().drawSubtitle(vg, x, y, width, height - ((drawOSD && !is_osd_lock) ? 120.0f : 0.0f),
                                              alpha);

    if (current > this->hintLastShowTime && this->hintBox->getVisibility() == brls::Visibility::VISIBLE) {
        this->clearHint();
    }

    // hot key
    this->buttonProcessing();

    // draw speed hint
    if (speedHintBox->getVisibility() == brls::Visibility::VISIBLE) {
        speedHintBox->frame(ctx);
        brls::Rect frame = speedHintLabel->getFrame();

        // a1-3 周期 800，范围 800 * 0.3 / 2 = 120, 0 - 120 - 0
        int a1 = ((brls::getCPUTimeUsec() >> 10) % 800) * 0.3;
        int a2 = (a1 + 40) % 240;
        int a3 = (a1 + 80) % 240;
        if (a1 > 120) a1 = 240 - a1;
        if (a2 > 120) a2 = 240 - a2;
        if (a3 > 120) a3 = 240 - a3;

        float tx                              = frame.getMinX() - 50;
        float ty                              = frame.getMinY() + 4.5;
        std::vector<std::pair<int, int>> data = {{0, a3 + 80}, {15, a2 + 80}, {30, a1 + 80}};

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

    // center hint
    osdCenterBox2->frame(ctx);

    // draw video profile
    videoProfile->frame(ctx);
}

void VideoView::drawHighlightProgress(NVGcontext* vg, float x, float y, float width, float alpha) {
    if (highlightData.data.size() <= 1) return;
    nvgBeginPath(vg);
    nvgFillColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f * alpha));
    float baseY  = y;
    float dX     = width / ((float)highlightData.data.size() - 1);
    float halfDx = dX / 2;
    float pointX = x, lastX = x;
    float lastY = baseY;
    nvgMoveTo(vg, lastX, lastY);
    lastY -= 12;
    nvgLineTo(vg, lastX, lastY);

    for (size_t i = 1; i < highlightData.data.size(); i++) {
        float item = highlightData.data[i];
        pointX += dX;
        float pointY = baseY - 12 - item * 48;
        float cx     = lastX + halfDx;
#ifdef __PS4__
        if (fabs(lastY - pointY) < 10) {
            // 尽量画直线，减小 ps4 GPU 崩溃的可能
#else
        if (fabs(lastY - pointY) < 3) {
#endif
            // 相差太小，直接绘制直线
            nvgLineTo(vg, pointX, pointY);
        } else {
            nvgBezierTo(vg, cx, lastY, cx, pointY, pointX, pointY);
        }
        lastX = pointX;
        lastY = pointY;
    }
    nvgLineTo(vg, x + width, baseY);
    nvgFill(vg);
}

void VideoView::invalidate() { View::invalidate(); }

void VideoView::onLayout() { brls::View::onLayout(); }

std::string VideoView::genExtraUrlParam(int start, int end, const std::string& audio) {
    std::vector<std::string> audios;
    if (!audio.empty()) {
        audios.emplace_back(audio);
    }
    return genExtraUrlParam(start, end, audios);
}

std::string VideoView::genExtraUrlParam(int start, int end, const std::vector<std::string>& audios) {
    std::string extra =
#ifdef __PSV__
        "referrer=\"https://www.bilibili.com\",network-timeout=10";
#else
        "referrer=\"https://www.bilibili.com\",network-timeout=5";
#endif
    auto proxy = ProgramConfig::instance().getProxy();
    if (!proxy.empty()) {
        extra += fmt::format(",http-proxy=\"{}\"", proxy);
    }
    if (start > 0) {
        extra += fmt::format(",start={}", start);
    }
    if (end > 0) {
        extra += fmt::format(",end={}", end);
    }
    for (auto& audio : audios) extra += fmt::format(",audio-file=\"{}\"", audio);
    return extra;
}

void VideoView::setUrl(const std::string& url, int start, int end, const std::string& audio) {
    std::vector<std::string> audios;
    if (!audio.empty()) {
        audios.emplace_back(audio);
    }
    setUrl(url, start, end, audios);
}

void VideoView::setUrl(const std::string& url, int start, int end, const std::vector<std::string>& audios) {
    mpvCore->setUrl(url, genExtraUrlParam(start, end, audios));
}

void VideoView::setBackupUrl(const std::string& url, int start, int end, const std::string& audio) {
    setBackupUrl(url, start, end, std::vector{audio});
}

void VideoView::setBackupUrl(const std::string& url, int start, int end, const std::vector<std::string>& audios) {
    mpvCore->setBackupUrl(url, genExtraUrlParam(start, end, audios));
}

void VideoView::setUrl(const std::vector<EDLUrl>& edl_urls, int start, int end) {
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
        urls.emplace_back("!delay_open,media_type=video;!delay_open,media_type=audio;" +
                          fmt::format("%{}%{},length={}", i.url.size(), i.url, i.length));
    }
    url += pystring::join(";", urls);
    this->setUrl(url, start, end);
}

void VideoView::resume() { mpvCore->resume(); }

void VideoView::pause() { mpvCore->pause(); }

void VideoView::stop() { mpvCore->stop(); }

void VideoView::togglePlay() {
    if (customToggleAction != nullptr) return customToggleAction();
    if (this->mpvCore->isPaused()) {
        if (showReplay) {
            this->mpvCore->seek(0);
            this->mpvCore->resume();
        } else {
            this->mpvCore->resume();
        }
    } else {
        this->mpvCore->pause();
    }
}

void VideoView::setCustomToggleAction(std::function<void()> action) { this->customToggleAction = action; }

void VideoView::setSpeed(float speed) { mpvCore->setSpeed(speed); }

void VideoView::setLastPlayedPosition(int64_t p) { lastPlayedPosition = p; }

int64_t VideoView::getLastPlayedPosition() const { return lastPlayedPosition; }

/// OSD
void VideoView::showOSD(bool temp) {
    if (temp) {
        this->osdLastShowTime = wiliwili::unix_time() + VideoView::OSD_SHOW_TIME;
        this->osd_state       = OSDState::SHOWN;
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

bool VideoView::isOSDLock() const { return this->is_osd_lock; }

void VideoView::onOSDStateChanged(bool state) {
    // 当焦点位于video组件内部重新赋予焦点，用来隐藏屏幕上的高亮框
    if (!state && isChildFocused()) {
        brls::Application::giveFocus(this);
    }
}

void VideoView::toggleOSDLock() {
    is_osd_lock = !is_osd_lock;
    this->osdLockIcon->setImageFromSVGRes(is_osd_lock ? "svg/player-lock.svg" : "svg/player-unlock.svg");
    if (is_osd_lock) {
        osdTopBox->setVisibility(brls::Visibility::INVISIBLE);
        osdBottomBox->setVisibility(brls::Visibility::INVISIBLE);
        // 锁定时上下按键不可用
        osdLockBox->setCustomNavigationRoute(brls::FocusDirection::UP, "video/osd/lock/box");
        osdLockBox->setCustomNavigationRoute(brls::FocusDirection::DOWN, "video/osd/lock/box");
    } else {
        // 手动设置上下按键的导航路线
        osdLockBox->setCustomNavigationRoute(brls::FocusDirection::UP, "video/osd/setting");
        osdLockBox->setCustomNavigationRoute(brls::FocusDirection::DOWN, "video/osd/icon/box");
    }
    this->showOSD();
}

void VideoView::toggleDanmaku() {
    if (!enableDanmaku) return;
    DanmakuCore::DANMAKU_ON = !DanmakuCore::DANMAKU_ON;
    this->refreshDanmakuIcon();
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

void VideoView::hideLoading() { osdCenterBox->setVisibility(brls::Visibility::GONE); }

void VideoView::setCenterHintText(const std::string& text) { centerLabel2->setText(text); }

void VideoView::setCenterHintIcon(const std::string& svg) { centerIcon2->setImageFromSVGRes(svg); }

void VideoView::showCenterHint() { osdCenterBox2->setVisibility(brls::Visibility::VISIBLE); }

void VideoView::hideCenterHint() { osdCenterBox2->setVisibility(brls::Visibility::GONE); }

void VideoView::hideDanmakuButton() {
    enableDanmaku = false;
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

void VideoView::hideOSDLockButton() {
    osdLockBox->setVisibility(brls::Visibility::INVISIBLE);
    hide_lock_button = true;
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
    _setTvControlMode(false);
}

void VideoView::setTvControlMode(bool state) {
    isTvControlMode = state;
    // 直播模式下不显示进度条
    _setTvControlMode(isTvControlMode && !isLiveMode);
}

bool VideoView::getTvControlMode() const { return isTvControlMode; }

void VideoView::_setTvControlMode(bool state) {
    btnToggle->setCustomNavigationRoute(brls::FocusDirection::RIGHT, state ? osdSlider : iconBox);
    btnVolumeIcon->setCustomNavigationRoute(brls::FocusDirection::UP, state ? osdSlider : osdLockBox);
    btnDanmakuSettingIcon->setCustomNavigationRoute(brls::FocusDirection::UP, state ? osdSlider : osdLockBox);
    btnDanmakuIcon->setCustomNavigationRoute(brls::FocusDirection::UP, state ? osdSlider : osdLockBox);
    videoQuality->setCustomNavigationRoute(brls::FocusDirection::UP, state ? osdSlider : osdLockBox);
    videoSpeed->setCustomNavigationRoute(brls::FocusDirection::UP, state ? osdSlider : osdLockBox);
    btnFullscreenIcon->setCustomNavigationRoute(brls::FocusDirection::UP, state ? osdSlider : osdLockBox);
    osdLockBox->setCustomNavigationRoute(brls::FocusDirection::DOWN, state ? osdSlider : iconBox);
    osdSlider->setFocusable(state);
}

void VideoView::setStatusLabelLeft(const std::string& value) { leftStatusLabel->setText(value); }

void VideoView::setStatusLabelRight(const std::string& value) { rightStatusLabel->setText(value); }

void VideoView::disableCloseOnEndOfFile() { closeOnEndOfFile = false; }

void VideoView::hideHistorySetting() { showHistorySetting = false; }

void VideoView::hideVideoRelatedSetting() { showVideoRelatedSetting = false; }

void VideoView::hideSubtitleSetting() { showSubtitleSetting = false; }

void VideoView::hideBottomLineSetting() { showBottomLineSetting = false; }

void VideoView::hideHighlightLineSetting() { showHighlightLineSetting = false; }

void VideoView::hideSkipOpeningCreditsSetting() { showOpeningCreditsSetting = false; }

void VideoView::hideVideoProgressSlider() { osdSlider->setVisibility(brls::Visibility::GONE); }

void VideoView::setTitle(const std::string& title) { this->videoTitleLabel->setText(title); }

void VideoView::setOnlineCount(const std::string& count) { this->videoOnlineCountLabel->setText(count); }

std::string VideoView::getTitle() { return this->videoTitleLabel->getFullText(); }

void VideoView::setQuality(const std::string& str) { this->videoQuality->setText(str); }

std::string VideoView::getQuality() { return this->videoQuality->getFullText(); }

void VideoView::setDuration(const std::string& value) { this->rightStatusLabel->setText(value); }

void VideoView::setPlaybackTime(const std::string& value) {
    if (this->is_seeking) return;
    if (this->isLiveMode) return;
    this->leftStatusLabel->setText(value);
}

void VideoView::setFullscreenIcon(bool fs) {
    if (fs) {
        btnFullscreenIcon->setImageFromSVGRes("svg/bpx-svg-sprite-fullscreen-off.svg");
    } else {
        btnFullscreenIcon->setImageFromSVGRes("svg/bpx-svg-sprite-fullscreen.svg");
    }
}

brls::View* VideoView::getFullscreenIcon() { return btnFullscreenIcon; }

void VideoView::refreshDanmakuIcon() {
    if (DanmakuCore::DANMAKU_ON) {
        this->btnDanmakuIcon->setImageFromSVGRes("svg/bpx-svg-sprite-danmu-switch-on.svg");
        btnDanmakuSettingIcon->setVisibility(brls::Visibility::VISIBLE);
        btnDanmakuSettingIcon->getParent()->setVisibility(brls::Visibility::VISIBLE);
    } else {
        this->btnDanmakuIcon->setImageFromSVGRes("svg/bpx-svg-sprite-danmu-switch-off.svg");
        // 当焦点刚好位于弹幕设置按钮时，这时通过快捷键关闭弹幕设置会导致焦点不会自动切换
        if (brls::Application::getCurrentFocus() == btnDanmakuSettingIcon) {
            brls::Application::giveFocus(btnDanmakuIcon);
        }
        btnDanmakuSettingIcon->setVisibility(brls::Visibility::GONE);
        btnDanmakuSettingIcon->getParent()->setVisibility(brls::Visibility::GONE);
    }
}

void VideoView::refreshToggleIcon() {
    if (!mpvCore->isPlaying()) {
        if (showReplay) {
            btnToggleIcon->setImageFromSVGRes("svg/bpx-svg-sprite-re-play.svg");
            return;
        }
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

void VideoView::setHighlightProgress(const VideoHighlightData& data) {
    this->highlightData = data;
}

void VideoView::showHint(const std::string& value) {
    brls::Logger::debug("Video hint: {}", value);
    this->hintLabel->setText(value);
    this->hintBox->setVisibility(brls::Visibility::VISIBLE);
    this->hintLastShowTime = wiliwili::unix_time() + VideoView::OSD_SHOW_TIME;
    this->showOSD();
}

void VideoView::clearHint() { this->hintBox->setVisibility(brls::Visibility::GONE); }

void VideoView::setBangumiCustomSetting(const std::string& title, uint64_t seasonId) {
    this->bangumiTitle    = title;
    this->bangumiSeasonId = seasonId;
}

brls::View* VideoView::create() { return new VideoView(); }

bool VideoView::isFullscreen() {
    auto rect = this->getFrame();
    return rect.getHeight() == brls::Application::contentHeight && rect.getWidth() == brls::Application::contentWidth;
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
        video->setBangumiCustomSetting(this->bangumiTitle, this->bangumiSeasonId);
        if (this->hintBox->getVisibility() == brls::Visibility::VISIBLE)
            video->showHint(this->hintLabel->getFullText());
        if (!showOpeningCreditsSetting) video->hideSkipOpeningCreditsSetting();
        video->showOSD(this->osd_state != OSDState::ALWAYS_ON);
        video->setFullscreenIcon(true);
        video->setHideHighlight(true);
        video->showReplay    = showReplay;
        video->real_duration = real_duration;
        video->setLastPlayedPosition(lastPlayedPosition);
        video->osdSlider->setClipPoint(osdSlider->getClipPoint());
        video->refreshToggleIcon();
        video->setHighlightProgress(highlightData);
        if (video->isLiveMode) video->setLiveMode();
        video->setCustomToggleAction(customToggleAction);
        DanmakuCore::instance().refresh();
        video->setOnlineCount(this->videoOnlineCountLabel->getFullText());
        if (osdCenterBox->getVisibility() == brls::Visibility::GONE) {
            video->hideLoading();
        }
        if (this->seasonAction != nullptr) {
            brls::View *view = video->showEpisode->getParent();
            view->registerClickAction(this->seasonAction);
            view->addGestureRecognizer(new brls::TapGestureRecognizer(view));
            view->setVisibility(brls::Visibility::VISIBLE);
            video->showEpisode->setVisibility(brls::Visibility::VISIBLE);
        }
        container->addView(video);
        auto activity = new brls::Activity(container);
        brls::Application::pushActivity(activity, brls::TransitionAnimation::NONE);
        registerFullscreen(activity);
    } else {
        ASYNC_RETAIN
        brls::sync([ASYNC_TOKEN]() {
            ASYNC_RELEASE
            //todo: a better way to get videoView pointer
            auto activityStack = brls::Application::getActivitiesStack();

            // 当最前方的 activity 内不包含 videoView 时，不执行关闭全屏
            brls::Activity* top = activityStack[activityStack.size() - 1];
            if (!dynamic_cast<BasePlayerActivity*>(top)) {
                // 判断最顶层是否为video
                if (!dynamic_cast<VideoView*>(top->getContentView()->getView("video"))) return;
            }

            // 同时点击全屏按钮和评论会导致评论弹出在 BasePlayerActivity 和 videoView 之间，
            // 因此目前需要遍历全部的 activity 找到 BasePlayerActivity
            if (activityStack.size() <= 2) {
                brls::Application::popActivity();
                return;
            }
            for (size_t i = activityStack.size() - 2; i != 0; i--) {
                auto* last = dynamic_cast<BasePlayerActivity*>(activityStack[i]);
                if (!last) continue;
                auto* video = dynamic_cast<VideoView*>(last->getView("video"));
                if (video) {
                    video->setProgress(this->getProgress());
                    video->showOSD(this->osd_state != OSDState::ALWAYS_ON);
                    video->setDuration(this->rightStatusLabel->getFullText());
                    video->setPlaybackTime(this->leftStatusLabel->getFullText());
                    video->registerMpvEvent();
                    video->showReplay    = showReplay;
                    video->real_duration = real_duration;
                    video->setLastPlayedPosition(lastPlayedPosition);
                    video->osdSlider->setClipPoint(osdSlider->getClipPoint());
                    video->setBangumiCustomSetting(this->bangumiTitle, this->bangumiSeasonId);
                    video->refreshToggleIcon();
                    video->setHighlightProgress(highlightData);
                    video->refreshDanmakuIcon();
                    video->setQuality(this->getQuality());
                    video->videoSpeed->setText(this->videoSpeed->getFullText());
                    DanmakuCore::instance().refresh();
                    if (osdCenterBox->getVisibility() == brls::Visibility::GONE) {
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

void VideoView::setSeasonAction(brls::ActionListener action) {
    this->seasonAction = action;
}

brls::View* VideoView::getDefaultFocus() {
    if (isFullscreen() && isOSDShown())
        return this->btnToggle;
    else
        return this;
}

brls::View* VideoView::getNextFocus(brls::FocusDirection direction, View* currentView) {
    if (this->isFullscreen()) return this;
    return brls::Box::getNextFocus(direction, currentView);
}

void VideoView::buttonProcessing() {
    // 获取按键数据
    brls::ControllerState state{};
    input->updateUnifiedControllerState(&state);

    // 当OSD显示时上下左右切换选择按钮，持续显示OSD
    if (isOSDShown() && (state.buttons[brls::BUTTON_NAV_RIGHT] || state.buttons[brls::BUTTON_NAV_LEFT] ||
                         state.buttons[brls::BUTTON_NAV_UP] || state.buttons[brls::BUTTON_NAV_DOWN])) {
        if (this->osd_state == OSDState::SHOWN) this->showOSD(true);
    }
    if (is_osd_lock) return;

#ifndef __PSV__

    static int click_state        = ClickState::IDLE;
    static int64_t rsb_press_time = 0;
    if (isLiveMode) return;
    if (click_state == ClickState::IDLE && !state.buttons[brls::BUTTON_RSB]) return;

    int CHECK_TIME = 200000;
    float SPEED    = MPVCore::VIDEO_SPEED == 100 ? 2.0 : MPVCore::VIDEO_SPEED * 0.01f;

    switch (click_state) {
        case ClickState::IDLE:
            if (state.buttons[brls::BUTTON_RSB]) {
                setSpeed(SPEED);
                rsb_press_time = brls::getCPUTimeUsec();
                click_state    = ClickState::PRESS;
                // 绘制临时加速标识
                speedHintLabel->setText(wiliwili::format("wiliwili/player/current_speed"_i18n, SPEED));
                speedHintBox->setVisibility(brls::Visibility::VISIBLE);
            }
            break;
        case ClickState::PRESS:
            if (!state.buttons[brls::BUTTON_RSB]) {
                setSpeed(1.0f);
                int64_t current_time = brls::getCPUTimeUsec();
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
            if (state.buttons[brls::BUTTON_RSB]) {
                setSpeed(SPEED);
                int64_t current_time = brls::getCPUTimeUsec();
                if (current_time - rsb_press_time < CHECK_TIME) {
                    rsb_press_time = current_time;
                    click_state    = ClickState::FAST_PRESS;
                } else {
                    rsb_press_time = current_time;
                    click_state    = ClickState::PRESS;
                }
                // 绘制临时加速标识
                speedHintLabel->setText(wiliwili::format("wiliwili/player/current_speed"_i18n, SPEED));
                speedHintBox->setVisibility(brls::Visibility::VISIBLE);
            }
            break;
        case ClickState::FAST_PRESS:
            if (!state.buttons[brls::BUTTON_RSB]) {
                int64_t current_time = brls::getCPUTimeUsec();
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
    eventSubscribeID = mpvCore->getEvent()->subscribe([this](MpvEventEnum event) {
        // brls::Logger::info("mpv event => : {}", event);
        switch (event) {
            case MpvEventEnum::MPV_IDLE:
                refreshToggleIcon();
                break;
            case MpvEventEnum::MPV_RESUME:
                this->showReplay = false;
                this->showOSD(true);
                this->hideLoading();
                break;
            case MpvEventEnum::MPV_PAUSE:
                this->showOSD(false);
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
                this->setPlaybackTime(wiliwili::sec2Time(this->mpvCore->video_progress));
                if (lastPlayedPosition <= 0) break;
                if (abs(getRealDuration() - lastPlayedPosition) <= 5) {
                    mpvCore->seek(0);
                } else {
                    this->showHint(fmt::format("已为您定位至: {}", wiliwili::sec2Time(lastPlayedPosition)));
                    mpvCore->seek(lastPlayedPosition);
                    brls::Logger::info("Restore video position: {}", lastPlayedPosition);
                }
                lastPlayedPosition = 0;
                break;
            case MpvEventEnum::UPDATE_DURATION:
                this->setDuration(wiliwili::sec2Time(getRealDuration()));
                this->setProgress((float)mpvCore->playback_time / getRealDuration());
                break;
            case MpvEventEnum::UPDATE_PROGRESS:
                this->setPlaybackTime(wiliwili::sec2Time(this->mpvCore->video_progress));
                this->setProgress((float)mpvCore->playback_time / getRealDuration());
                break;
            case MpvEventEnum::VIDEO_SPEED_CHANGE:
                if (fabs(mpvCore->video_speed - 1) < 1e-5) {
                    this->videoSpeed->setText("wiliwili/player/speed"_i18n);
                } else {
                    this->videoSpeed->setText(fmt::format("{}x", mpvCore->video_speed));
                }
                break;
            case MpvEventEnum::END_OF_FILE:
                // 播放结束自动取消全屏
                this->showOSD(false);
                if (EXIT_FULLSCREEN_ON_END && closeOnEndOfFile && this->isFullscreen()) {
                    this->setFullScreen(false);
                }
                break;
            case MpvEventEnum::CACHE_SPEED_CHANGE:
                // 仅当加载圈已经开始转起的情况显示缓存
                if (this->osdCenterBox->getVisibility() != brls::Visibility::GONE) {
                    if (this->centerLabel->getVisibility() != brls::Visibility::VISIBLE)
                        this->centerLabel->setVisibility(brls::Visibility::VISIBLE);
                    this->centerLabel->setText(mpvCore->getCacheSpeed());
                }
                break;
            case MpvEventEnum::VIDEO_MUTE:
                this->btnVolumeIcon->setImageFromSVGRes("svg/bpx-svg-sprite-volume-off.svg");
                break;
            case MpvEventEnum::VIDEO_UNMUTE:
                this->btnVolumeIcon->setImageFromSVGRes("svg/bpx-svg-sprite-volume.svg");
                break;
            case MpvEventEnum::RESET:
                // 重置进度条标记点
                osdSlider->clearClipPoint();
                real_duration = 0;
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
    if (is_osd_lock) {
        brls::Application::giveFocus(this->osdLockBox);
        return;
    }
    // 只有在全屏显示OSD时允许OSD组件获取焦点
    if (this->isFullscreen() && isOSDShown()) {
        // 当弹幕按钮隐藏时不可获取焦点
        if (focusedView->getParent()->getVisibility() == brls::Visibility::GONE) {
            brls::Application::giveFocus(this);
        }
        static View* lastFocusedView = nullptr;

        // 设定自定义导航
        if (focusedView == this->btnSettingIcon) {
            this->btnSettingIcon->setCustomNavigationRoute(
                brls::FocusDirection::DOWN,
                lastFocusedView == this->btnToggle ? "video/osd/toggle" : "video/osd/lock/box");
        }
        lastFocusedView = focusedView;
        return;
    }
    brls::Application::giveFocus(this);
}

float VideoView::getRealDuration() { return real_duration > 0 ? (float)real_duration : (float)mpvCore->duration; }
