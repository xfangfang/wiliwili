//
// Created by fang on 2023/3/4.
//

#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/thread.hpp>
#include <borealis/views/cells/cell_bool.hpp>
#include <borealis/views/cells/cell_slider.hpp>
#include <borealis/views/cells/cell_input.hpp>

#include "utils/config_helper.hpp"
#include "utils/shader_helper.hpp"
#include "utils/number_helper.hpp"
#include "utils/activity_helper.hpp"
#include "activity/player_activity.hpp"
#include "fragment/player_setting.hpp"
#include "view/danmaku_core.hpp"
#include "view/button_close.hpp"
#include "view/subtitle_core.hpp"
#include "view/video_view.hpp"
#include "view/selector_cell.hpp"
#include "view/svg_image.hpp"
#include "view/mpv_core.hpp"

using namespace brls::literals;

PlayerSetting::PlayerSetting() {
    this->inflateFromXMLRes("xml/fragment/player_setting.xml");
    brls::Logger::debug("Fragment PlayerSetting: create");

    setupCustomShaders();
    setupCommonSetting();
    setupSubtitle();

    this->registerAction("hints/cancel"_i18n, brls::BUTTON_B, [](...) {
        brls::Application::popActivity();
        return true;
    });

    this->cancel->registerClickAction([](...) {
        brls::Application::popActivity();
        return true;
    });
    this->cancel->addGestureRecognizer(new brls::TapGestureRecognizer(this->cancel));

    closebtn->registerClickAction([](...) {
        brls::Application::popActivity();
        return true;
    });
}

PlayerSetting::~PlayerSetting() { brls::Logger::debug("Fragment PlayerSetting: delete"); }

brls::View* PlayerSetting::create() { return new PlayerSetting(); }

bool PlayerSetting::isTranslucent() { return true; }

brls::View* PlayerSetting::getDefaultFocus() { return this->settings->getDefaultFocus(); }

void PlayerSetting::setupCustomShaders() {
    // TODO Fix: shaders cannot work with deko3d and ps4
#if !defined(_DEBUG) && (defined(BOREALIS_USE_DEKO3D) || defined(PS4))
    // hide shader setting: deko3d and ps4
    auto* cell = new brls::RadioCell();
    cell->title->setText("wiliwili/dialog/not_supported"_i18n);
    shaderBox->addView(cell);
    return;
#else
    if (!ShaderHelper::instance().isAvailable()) {
        auto* cell = new brls::RadioCell();
        cell->title->setText("wiliwili/player/setting/common/wiki"_i18n);
        cell->registerClickAction([](...) {
            brls::Application::getPlatform()->openBrowser("https://github.com/xfangfang/wiliwili/wiki");
            return true;
        });
        shaderBox->addView(cell);
        auto* hint = new brls::Label();
        hint->setText("https://github.com/xfangfang/wiliwili/wiki#自定义着色器");
        hint->setFontSize(12);
        hint->setMargins(10, 0, 0, 10);
        hint->setTextColor(brls::Application::getTheme().getColor("font/grey"));
        shaderBox->addView(hint);
        return;
    }
#endif

    auto pack = ShaderHelper::instance().getShaderPack();
    for (auto& p : pack.profiles) {
        brls::Logger::debug("name: {}", p.name);

        auto* cell = new brls::RadioCell();
        cell->title->setText(p.name);
        registerHideBackground(cell);
        if (MPVCore::instance().currentShaderProfile == p.name) cell->setSelected(true);

        cell->registerClickAction([cell, this](...) {
            bool value = !cell->getSelected();
            cell->setSelected(value);

            for (auto& child : shaderBox->getChildren()) {
                auto* v = dynamic_cast<brls::RadioCell*>(child);
                if (cell == v) continue;
                if (v) v->setSelected(false);
            }

            if (value) {
                size_t index = *(size_t*)cell->getParentUserData();
                ShaderHelper::instance().setShader(index);
            } else {
                ShaderHelper::instance().clearShader();
            }

#ifdef BOREALIS_USE_D3D11
            // 如果正在使用硬解，那么将硬解更新为 auto-copy，避免直接硬解因为不经过 cpu 处理导致滤镜无效
            if (MPVCore::HARDWARE_DEC) {
                std::string hwdec = value ? "auto-copy" : MPVCore::PLAYER_HWDEC_METHOD;
                MPVCore::instance().command_async("set", "hwdec", hwdec);
                brls::Logger::info("MPV hardware decode: {}", hwdec);
            }
#endif

            GA("player_setting", {{"shader", cell->title->getFullText()}});
            return true;
        });

        shaderBox->addView(cell);
    }
}

void PlayerSetting::setupCommonSetting() {
    auto& conf  = ProgramConfig::instance();
    auto locale = brls::Application::getLocale();

    /// Upload history record
    btnHistory->init("wiliwili/setting/app/playback/report"_i18n,
                     conf.getSettingItem(SettingItem::HISTORY_REPORT, true), [](bool value) {
                         ProgramConfig::instance().setSettingItem(SettingItem::HISTORY_REPORT, value);
                         VideoDetail::REPORT_HISTORY = value;
                         GA("player_setting", {{"history", value ? "true" : "false"}});
                     });

    /// Skip opening credits
    btnSkip->init("wiliwili/player/setting/common/skip_opening_credits"_i18n,
                  conf.getBoolOption(SettingItem::PLAYER_SKIP_OPENING_CREDITS), [](bool value) {
                      ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_SKIP_OPENING_CREDITS, value);
                      BasePlayerActivity::PLAYER_SKIP_OPENING_CREDITS = value;
                      std::string hint = value ? "wiliwili/player/setting/common/skip_hint1"_i18n
                                               : "wiliwili/player/setting/common/skip_hint2"_i18n;
                      APP_E->fire(VideoView::HINT, (void*)hint.c_str());
                      GA("player_setting", {{"skip", value ? "true" : "false"}});
                  });

    /// player strategy
    int strategyIndex = conf.getIntOption(SettingItem::PLAYER_STRATEGY);
    // 中文比较简洁可以显示出来，其他语言翻译过长，在这里就先不展示了
    bool showStrategy = locale == brls::LOCALE_ZH_HANT || locale == brls::LOCALE_ZH_HANS;
    btnPlayStrategy->setText("wiliwili/setting/app/playback/play_strategy"_i18n);
    std::vector<std::string> optionList = {"wiliwili/setting/app/playback/auto_play_recommend"_i18n,
                                           "wiliwili/setting/app/playback/auto_play_next_part"_i18n,
                                           "wiliwili/setting/app/playback/auto_loop"_i18n,
                                           "wiliwili/setting/app/playback/auto_no"_i18n};
    btnPlayStrategy->setDetailText(showStrategy ? optionList[strategyIndex] : " ");
    btnPlayStrategy->registerClickAction([this, optionList, showStrategy](View* view) {
        BaseDropdown::text(
            "wiliwili/setting/app/playback/play_strategy"_i18n, optionList,
            [this, optionList, showStrategy](int data) {
                if (showStrategy) btnPlayStrategy->setDetailText(optionList[data]);
                BasePlayerActivity::PLAYER_STRATEGY = data;
                ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_STRATEGY, data);
                GA("player_setting", {{"strategy", optionList[data]}});
            },
            ProgramConfig::instance().getIntOption(SettingItem::PLAYER_STRATEGY),
            "wiliwili/setting/app/playback/auto_hint"_i18n);
        return true;
    });

    btnExitFullscreen->init("wiliwili/setting/app/playback/exit_fullscreen"_i18n,
                            conf.getBoolOption(SettingItem::PLAYER_EXIT_FULLSCREEN_ON_END), [](bool value) {
                                ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_EXIT_FULLSCREEN_ON_END,
                                                                         value);
                                VideoView::EXIT_FULLSCREEN_ON_END = value;
                                GA("player_setting", {{"exit_fs", value ? "true" : "false"}});
                            });

    /// Player bottom bar
    btnProgress->init("wiliwili/setting/app/playback/player_bar"_i18n,
                      conf.getBoolOption(SettingItem::PLAYER_BOTTOM_BAR), [](bool value) {
                          ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_BOTTOM_BAR, value);
                          VideoView::BOTTOM_BAR = value;
                          GA("player_setting", {{"bottom_bar", value ? "true" : "false"}});
                      });

    /// Player mirror
    btnMirror->init("wiliwili/player/setting/common/mirror"_i18n, MPVCore::VIDEO_MIRROR, [](bool value) {
        MPVCore::instance().setMirror(!MPVCore::VIDEO_MIRROR);
        GA("player_setting", {{"mirror", value ? "true" : "false"}});

        // 如果正在使用硬解，那么将硬解更新为 auto-copy，避免直接硬解因为不经过 cpu 处理导致镜像翻转无效
        if (MPVCore::HARDWARE_DEC) {
            std::string hwdec = MPVCore::VIDEO_MIRROR ? "auto-copy" : MPVCore::PLAYER_HWDEC_METHOD;
            MPVCore::instance().command_async("set", "hwdec", hwdec);
            brls::Logger::info("MPV hardware decode: {}", hwdec);
        }
    });

    /// Player aspect
    btnAspect->init("wiliwili/player/setting/aspect/header"_i18n,
                    {"wiliwili/player/setting/aspect/auto"_i18n, "wiliwili/player/setting/aspect/stretch"_i18n,
                     "wiliwili/player/setting/aspect/crop"_i18n, "4:3", "16:9"},
                    conf.getStringOptionIndex(SettingItem::PLAYER_ASPECT), [this](int value) {
                        auto option        = ProgramConfig::instance().getOptionData(SettingItem::PLAYER_ASPECT);
                        auto& aspect       = option.optionList[value];
                        auto seasonSetting = ProgramConfig::instance().getSeasonCustom(std::to_string(seasonId));
                        if (seasonSetting.player_aspect.empty()) {
                            // 只有在不是番剧，或者没有自定义番剧设置时，才会直接修改当前的视频比例
                            MPVCore::instance().setAspect(aspect);
                        }
                        ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_ASPECT, aspect);
                        GA("player_setting", {{"aspect", aspect}});
                    });

    /// Player Highlight progress bar
    btnHighlight->init("wiliwili/player/setting/common/highlight"_i18n, VideoView::HIGHLIGHT_PROGRESS_BAR,
                       [](bool value) {
                           ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_HIGHLIGHT_BAR, value);
                           VideoView::HIGHLIGHT_PROGRESS_BAR = value;
                           GA("player_setting", {{"highlight", value ? "true" : "false"}});
                       });

    /// Auto Sleep
    btnSleep->setText("wiliwili/setting/app/playback/sleep"_i18n);
    updateCountdown(wiliwili::getUnixTime());
    btnSleep->registerClickAction([this](View* view) {
        std::vector<int> timeList           = {15, 30, 60, 90, 120};
        std::string min                     = "wiliwili/home/common/min"_i18n;
        std::vector<std::string> optionList = {"15 " + min, "30 " + min, "60 " + min, "90 " + min, "120 " + min};
        bool countdownStarted               = MPVCore::CLOSE_TIME != 0 && wiliwili::getUnixTime() < MPVCore::CLOSE_TIME;
        if (countdownStarted) {
            // 添加关闭选项
            timeList.insert(timeList.begin(), -1);
            optionList.insert(optionList.begin(), "hints/off"_i18n);
        }
        BaseDropdown::text(
            "wiliwili/setting/app/playback/sleep"_i18n, optionList,
            [this, timeList, countdownStarted](int data) {
                if (countdownStarted && data == 0) {
                    MPVCore::CLOSE_TIME = 0;
                    GA("player_setting", {{"sleep", "-1"}});
                } else {
                    MPVCore::CLOSE_TIME = wiliwili::getUnixTime() + timeList[data] * 60;
                    GA("player_setting", {{"sleep", timeList[data]}});
                }
                updateCountdown(wiliwili::getUnixTime());
            },
            -1);
        return true;
    });

/// Fullscreen
#ifdef ALLOW_FULLSCREEN
    btnFullscreen->init("wiliwili/setting/app/others/fullscreen"_i18n, conf.getBoolOption(SettingItem::FULLSCREEN),
                        [](bool value) {
                            ProgramConfig::instance().setSettingItem(SettingItem::FULLSCREEN, value);
                            // 更新设置
                            VideoContext::FULLSCREEN = value;
                            // 设置当前状态
                            brls::Application::getPlatform()->getVideoContext()->fullScreen(value);
                            GA("player_setting", {{"fullscreen", value ? "true" : "false"}});
                        });

    auto setOnTopCell = [this](bool enabled) {
        if (enabled) {
            btnOnTopMode->setDetailTextColor(brls::Application::getTheme()["brls/list/listItem_value_color"]);
        } else {
            btnOnTopMode->setDetailTextColor(brls::Application::getTheme()["brls/text_disabled"]);
        }
    };
    setOnTopCell(conf.getIntOptionIndex(SettingItem::ON_TOP_MODE) != 0);
    int onTopModeIndex = conf.getIntOption(SettingItem::ON_TOP_MODE);
    btnOnTopMode->setText("wiliwili/setting/app/others/always_on_top"_i18n);
    std::vector<std::string> onTopOptionList = {"hints/off"_i18n, "hints/on"_i18n,
                                                "wiliwili/player/setting/aspect/auto"_i18n};
    btnOnTopMode->setDetailText(onTopOptionList[onTopModeIndex]);
    btnOnTopMode->registerClickAction([this, onTopOptionList, setOnTopCell](brls::View* view) {
        BaseDropdown::text(
            "wiliwili/setting/app/others/always_on_top"_i18n, onTopOptionList,
            [this, onTopOptionList, setOnTopCell](int data) {
                btnOnTopMode->setDetailText(onTopOptionList[data]);
                ProgramConfig::instance().setSettingItem(SettingItem::ON_TOP_MODE, data);
                ProgramConfig::instance().checkOnTop();
                setOnTopCell(data != 0);
                GA("player_setting", {{"on_top_mode", data}});
            },
            ProgramConfig::instance().getIntOption(SettingItem::ON_TOP_MODE),
            "wiliwili/setting/app/others/always_on_top_hint"_i18n);
        return true;
    });

#else
    btnFullscreen->setVisibility(brls::Visibility::GONE);
    btnOnTopMode->setVisibility(brls::Visibility::GONE);
#endif

    btnEqualizerReset->registerClickAction([this](View* view) {
        btnEqualizerBrightness->slider->setProgress(0.5f);
        btnEqualizerContrast->slider->setProgress(0.5f);
        btnEqualizerSaturation->slider->setProgress(0.5f);
        btnEqualizerGamma->slider->setProgress(0.5f);
        btnEqualizerHue->slider->setProgress(0.5f);
        return true;
    });
    registerHideBackground(btnEqualizerReset);

    setupEqualizerSetting(btnEqualizerBrightness, "wiliwili/player/setting/equalizer/brightness"_i18n,
                          SettingItem::PLAYER_BRIGHTNESS, MPVCore::instance().getBrightness());
    setupEqualizerSetting(btnEqualizerContrast, "wiliwili/player/setting/equalizer/contrast"_i18n,
                          SettingItem::PLAYER_CONTRAST, MPVCore::instance().getContrast());
    setupEqualizerSetting(btnEqualizerSaturation, "wiliwili/player/setting/equalizer/saturation"_i18n,
                          SettingItem::PLAYER_SATURATION, MPVCore::instance().getSaturation());
    setupEqualizerSetting(btnEqualizerGamma, "wiliwili/player/setting/equalizer/gamma"_i18n, SettingItem::PLAYER_GAMMA,
                          MPVCore::instance().getGamma());
    setupEqualizerSetting(btnEqualizerHue, "wiliwili/player/setting/equalizer/hue"_i18n, SettingItem::PLAYER_HUE,
                          MPVCore::instance().getHue());
}

void PlayerSetting::setupEqualizerSetting(brls::SliderCell* cell, const std::string& title, SettingItem item,
                                          int initValue) {
    if (initValue < -100) initValue = -100;
    if (initValue > 100) initValue = 100;
    cell->detail->setWidth(50);
    cell->title->setWidth(116);
    cell->title->setMarginRight(0);
    cell->slider->setStep(0.05f);
    cell->slider->setMarginRight(0);
    cell->slider->setPointerSize(20);
    cell->setDetailText(std::to_string(initValue));
    cell->init(title, (initValue + 100) * 0.005f, [cell, item](float value) {
        int data = (int)(value * 200 - 100);
        if (data < -100) data = -100;
        if (data > 100) data = 100;
        cell->detail->setText(std::to_string(data));
        switch (item) {
            case SettingItem::PLAYER_BRIGHTNESS:
                MPVCore::instance().setBrightness(data);
                break;
            case SettingItem::PLAYER_CONTRAST:
                MPVCore::instance().setContrast(data);
                break;
            case SettingItem::PLAYER_SATURATION:
                MPVCore::instance().setSaturation(data);
                break;
            case SettingItem::PLAYER_GAMMA:
                MPVCore::instance().setGamma(data);
                break;
            case SettingItem::PLAYER_HUE:
                MPVCore::instance().setHue(data);
                break;
            default:
                break;
        }
        static size_t iter = 0;
        brls::cancelDelay(iter);
        iter = brls::delay(200, []() {
            ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_BRIGHTNESS, MPVCore::VIDEO_BRIGHTNESS, false);
            ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_CONTRAST, MPVCore::VIDEO_CONTRAST, false);
            ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_SATURATION, MPVCore::VIDEO_SATURATION, false);
            ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_GAMMA, MPVCore::VIDEO_GAMMA, false);
            ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_HUE, MPVCore::VIDEO_HUE, false);
            ProgramConfig::instance().save();
        });
    });
    registerHideBackground(cell->getDefaultFocus());
}

void PlayerSetting::registerHideBackground(brls::View* view) {
    view->getFocusEvent()->subscribe([this](...) { this->setBackgroundColor(nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f)); });

    view->getFocusLostEvent()->subscribe(
        [this](...) { this->setBackgroundColor(brls::Application::getTheme().getColor("brls/backdrop")); });
}

void PlayerSetting::setupSubtitle() {
    auto& sub = SubtitleCore::instance();
    if (!sub.isAvailable()) {
        subtitleBox->setVisibility(brls::Visibility::GONE);
        subtitleHeader->setVisibility(brls::Visibility::GONE);
        return;
    }

    for (auto& s : sub.getSubtitleList().subtitles) {
        auto* cell = new brls::RadioCell();
        if (SubtitleCore::instance().getCurrentSubtitleId() == s.id_str) cell->setSelected(true);
        cell->title->setText(s.lan_doc);

        cell->registerClickAction([cell, this](...) {
            bool value = !cell->getSelected();
            cell->setSelected(value);

            for (auto& child : subtitleBox->getChildren()) {
                auto* v = dynamic_cast<brls::RadioCell*>(child);
                if (cell == v) continue;
                if (v) v->setSelected(false);
            }

            if (value) {
                size_t index = *(size_t*)cell->getParentUserData();
                SubtitleCore::instance().selectSubtitle(index);
            } else {
                SubtitleCore::instance().clearSubtitle();
            }

            GA("player_setting", {{value ? "subtitle-on" : "subtitle-off", cell->title->getFullText()}});
            return true;
        });

        subtitleBox->addView(cell);
    }
}

void PlayerSetting::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                         brls::FrameContext* ctx) {
    static size_t updateTime = 0;
    size_t now               = wiliwili::getUnixTime();
    if (now != updateTime) {
        updateTime = now;
        updateCountdown(now);
    }
    Box::draw(vg, x, y, width, height, style, ctx);
}

void PlayerSetting::updateCountdown(size_t now) {
    if (MPVCore::CLOSE_TIME == 0 || now > MPVCore::CLOSE_TIME) {
        btnSleep->setDetailTextColor(brls::Application::getTheme()["brls/text_disabled"]);
        btnSleep->setDetailText("hints/off"_i18n);
    } else {
        btnSleep->setDetailTextColor(brls::Application::getTheme()["brls/list/listItem_value_color"]);
        btnSleep->setDetailText(wiliwili::sec2Time(MPVCore::CLOSE_TIME - now));
    }
}

void PlayerSetting::hideHistoryCell() { btnHistory->setVisibility(brls::Visibility::GONE); }

void PlayerSetting::hideVideoRelatedCells() {
    btnPlayStrategy->setVisibility(brls::Visibility::GONE);
    btnExitFullscreen->setVisibility(brls::Visibility::GONE);
}

void PlayerSetting::hideSubtitleCells() {
    subtitleBox->setVisibility(brls::Visibility::GONE);
    subtitleHeader->setVisibility(brls::Visibility::GONE);
}

void PlayerSetting::hideBottomLineCells() { btnProgress->setVisibility(brls::Visibility::GONE); }

void PlayerSetting::hideHighlightLineCells() { btnHighlight->setVisibility(brls::Visibility::GONE); }

void PlayerSetting::hideSkipOpeningCreditsSetting() { btnSkip->setVisibility(brls::Visibility::GONE); }

void PlayerSetting::setBangumiCustomSetting(const std::string& title, uint64_t id) {
    if (id == 0) return;
    seasonId = id;

    bangumiHeader->setTitle(title);
    bangumiHeader->setVisibility(brls::Visibility::VISIBLE);
    bangumiBox->setVisibility(brls::Visibility::VISIBLE);
    btnCustomAspect->setVisibility(brls::Visibility::VISIBLE);
    btnClip->setVisibility(brls::Visibility::VISIBLE);

    /// 番剧自定义数据
    auto seasonSetting = ProgramConfig::instance().getSeasonCustom(seasonId);

    std::unordered_map<std::string, int> aspectMap = {{"", 0},   {"-1", 1},  {"-2", 2},
                                                      {"-3", 3}, {"4:3", 4}, {"16:9", 5}};
    int aspect                                     = 0;
    if (aspectMap.find(seasonSetting.player_aspect) != aspectMap.end()) {
        aspect = aspectMap[seasonSetting.player_aspect];
    }
    if (aspect == 0) {
        btnCustomAspect->setDetailTextColor(brls::Application::getTheme()["brls/text_disabled"]);
    }
    btnCustomAspect->init(
        "wiliwili/player/setting/season/aspect"_i18n,
        {"hints/off"_i18n, "wiliwili/player/setting/aspect/auto"_i18n, "wiliwili/player/setting/aspect/stretch"_i18n,
         "wiliwili/player/setting/aspect/crop"_i18n, "4:3", "16:9"},
        aspect, [this, seasonSetting](int value) {
            std::vector<std::string> aspectOption = {"", "-1", "-2", "-3", "4:3", "16:9"};
            auto setting                          = ProgramConfig::instance().getSeasonCustom(seasonId);
            setting.player_aspect                 = aspectOption[value];
            ProgramConfig::instance().addSeasonCustomSetting(seasonId, setting);
            auto theme = brls::Application::getTheme();
            if (setting.player_aspect.empty()) {
                // 如果设置为空，则使用全局设置
                setting.player_aspect =
                    ProgramConfig::instance().getSettingItem(SettingItem::PLAYER_ASPECT, std::string{"-1"});
                btnCustomAspect->setDetailTextColor(theme["brls/text_disabled"]);
            } else {
                btnCustomAspect->setDetailTextColor(theme["brls/list/listItem_value_color"]);
            }
            MPVCore::instance().setAspect(setting.player_aspect);
            GA("season_custom_setting", {{"aspect", setting.player_aspect}});
        });
    if (seasonSetting.custom_clip) {
        btnClipStart->setVisibility(brls::Visibility::VISIBLE);
        btnClipEnd->setVisibility(brls::Visibility::VISIBLE);
    }
    btnClip->init("wiliwili/player/setting/season/clip"_i18n, seasonSetting.custom_clip, [this](bool value) {
        auto setting        = ProgramConfig::instance().getSeasonCustom(seasonId);
        setting.custom_clip = value;
        ProgramConfig::instance().addSeasonCustomSetting(seasonId, setting);
        std::string hint;
        if (value) {
            hint = "wiliwili/player/setting/common/skip_hint3"_i18n;
            btnClipStart->setVisibility(brls::Visibility::VISIBLE);
            btnClipEnd->setVisibility(brls::Visibility::VISIBLE);
        } else {
            hint = "wiliwili/player/setting/common/skip_hint4"_i18n;
            btnClipStart->setVisibility(brls::Visibility::GONE);
            btnClipEnd->setVisibility(brls::Visibility::GONE);
        }
        APP_E->fire(VideoView::HINT, (void*)hint.c_str());
        GA("season_custom_setting", {{"custom_clip", value ? "true" : "false"}});
    });
    btnClipStart->init(
        "wiliwili/player/setting/season/clip_start"_i18n, seasonSetting.clip_start,
        [this](int value) {
            auto setting       = ProgramConfig::instance().getSeasonCustom(seasonId);
            setting.clip_start = value;
            ProgramConfig::instance().addSeasonCustomSetting(seasonId, setting);
        },
        "wiliwili/player/setting/season/unit"_i18n, 4);
    btnClipEnd->init(
        "wiliwili/player/setting/season/clip_end"_i18n, seasonSetting.clip_end,
        [this](int value) {
            auto setting     = ProgramConfig::instance().getSeasonCustom(seasonId);
            setting.clip_end = value;
            ProgramConfig::instance().addSeasonCustomSetting(seasonId, setting);
        },
        "wiliwili/player/setting/season/unit"_i18n, 4);
}