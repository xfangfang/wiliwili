//
// Created by fang on 2022/8/22.
//

#include "activity/setting_activity.hpp"
#include "activity/hint_activity.hpp"
#include "activity/player_activity.hpp"
#include "fragment/setting_network.hpp"
#include "fragment/test_rumble.hpp"
#include "view/text_box.hpp"
#include "utils/config_helper.hpp"
#include "utils/vibration_helper.hpp"
#include "utils/dialog_helper.hpp"
#include "borealis/core/cache_helper.hpp"
#include "borealis/views/applet_frame.hpp"

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
#include "borealis/platforms/desktop/desktop_platform.hpp"
#endif

using namespace brls::literals;

const std::string OPENSOURCE =
    "--------------------------------\n"
    "FFmpeg\n"
    "--------------------------------\n"
    "Official site:    https://www.ffmpeg.org\n"
    "Copyright (c) FFmpeg developers and contributors\n\n"
    "Licensed under LGPLv2.1 or later\n\n\n"
    "--------------------------------\n"
    "mpv\n"
    "--------------------------------\n"
    "Official site:    https://mpv.io\n"
    "Copyright (c) mpv developers and contributors\n\n"
    "Licensed under GPL-2.0 or LGPLv2.1\n\n\n"
    "--------------------------------\n"
    "borealis\n"
    "--------------------------------\n"
    "https://github.com/natinusala/borealis\n"
    "Copyright (c) 2019-2022, natinusala and contributors\n\n"
    "Modifications for touch and recycler list support\n"
    "https://github.com/XITRIX/borealis\n"
    "Copyright (c) XITRIX \n\n"
    "Modified version: https://github.com/xfangfang/borealis\n\n"
    "Licensed under Apache-2.0 license\n\n\n"
    "--------------------------------\n"
    "OpenCC\n"
    "--------------------------------\n"
    "https://github.com/BYVoid/OpenCC\n"
    "Copyright (c) Carbo Kuo and contributors\n\n"
    "Modified version: https://github.com/xfangfang/OpenCC\n\n"
    "Licensed under Apache-2.0 license\n\n\n"
    "--------------------------------\n"
    "pystring\n"
    "--------------------------------\n"
    "https://github.com/imageworks/pystring\n\n"
    "Copyright (c) imageworks and contributors\n\n"
    "Licensed under BCD-3-Clause license\n\n\n"
    "--------------------------------\n"
    "QR-Code-generator\n"
    "--------------------------------\n"
    "Official site: https://www.nayuki.io/page/qr-code-generator-library\n"
    "https://github.com/nayuki/QR-Code-generator\n\n"
    "Copyright © 2020 Project Nayuki.\n\nLicensed under MIT license\n\n\n"
    "--------------------------------\n"
    "lunasvg\n"
    "--------------------------------\n"
    "https://github.com/sammycage/lunasvg\n\n"
    "Copyright (c) 2020 Nwutobo Samuel Ugochukwu.\n\n"
    "Licensed under MIT license\n\n\n"
    "--------------------------------\n"
    "cpr\n"
    "--------------------------------\n"
    "Official site: https://docs.libcpr.org\n"
    "https://github.com/libcpr/cpr\n\n"
    "Copyright (c) 2017-2021 Huu Nguyen\n"
    "Copyright (c) 2022 libcpr and many other contributors\n\n"
    "Licensed under MIT license\n\n\n"
    "--------------------------------\n"
    "nx\n--------------------------------\n"
    "https://github.com/switchbrew/libnx\n\n"
    "Copyright 2017-2018 libnx Authors\n\nPublic domain\n\n\n"
    "--------------------------------\n"
    "devkitPro\n"
    "--------------------------------\n"
    "https://devkitpro.org\n\n"
    "Copyright devkitPro Authors\n\n"
    "Public domain\n"
    "\n";

SettingActivity::SettingActivity() {
    brls::Logger::debug("SettingActivity: create");
    GA("open_setting")
}

void SettingActivity::onContentAvailable() {
    brls::Logger::debug("SettingActivity: onContentAvailable");

#ifdef __SWITCH__
    btnTutorialOpenApp->registerClickAction([](...) -> bool {
        brls::Application::pushActivity(new HintActivity());
        return true;
    });
#else
    btnTutorialOpenApp->setVisibility(brls::Visibility::GONE);
#endif

    btnTutorialOpenVideoIntro->registerClickAction([](...) -> bool {
        brls::Application::pushActivity(new PlayerActivity(
            "wiliwili/setting/tools/tutorial/intro_bvid"_i18n));
        return true;
    });

#ifdef __SWITCH__
    btnTutorialError->registerClickAction([](...) -> bool {
        auto dialog =
            new brls::Dialog((brls::Box*)brls::View::createFromXMLResource(
                "fragment/settings_tutorial_error.xml"));
        dialog->addButton("hints/ok"_i18n, []() {});
        dialog->open();
        return true;
    });
#else
    btnTutorialError->setVisibility(brls::Visibility::GONE);
#endif

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
    btnOpenConfig->registerClickAction([](...) -> bool {
        auto* p = (brls::DesktopPlatform*)brls::Application::getPlatform();
        p->openBrowser(ProgramConfig::instance().getConfigDir());
        return true;
    });
    btnTutorialFont->registerClickAction([](...) -> bool {
        auto dialog =
            new brls::Dialog((brls::Box*)brls::View::createFromXMLResource(
                "fragment/settings_tutorial_font.xml"));
        dialog->addButton("hints/ok"_i18n, []() {});
        dialog->open();
        return true;
    });
#else
    btnOpenConfig->setVisibility(brls::Visibility::GONE);
    btnTutorialFont->setVisibility(brls::Visibility::GONE);
#endif

    btnHotKey->registerClickAction([](...) -> bool {
        auto dialog =
            new brls::Dialog((brls::Box*)brls::View::createFromXMLResource(
                "fragment/settings_hot_keys.xml"));
        dialog->addButton("hints/ok"_i18n, []() {});
        dialog->open();
        return true;
    });

    btnNetworkChecker->registerClickAction([](...) -> bool {
        auto dialog = new brls::Dialog((brls::Box*)new SettingNetwork());
        dialog->addButton("hints/ok"_i18n, []() {});
        dialog->open();
        return true;
    });

#ifdef __SWITCH__
    btnVibrationTest->registerClickAction([](...) -> bool {
        auto dialog = new brls::Dialog((brls::Box*)new TestRumble());
        dialog->addButton("hints/ok"_i18n, []() {});
        dialog->open();
        return true;
    });
#else
    btnVibrationTest->setVisibility(brls::Visibility::GONE);
#endif

    std::string version = APPVersion::instance().git_tag.empty()
                              ? "v" + APPVersion::instance().getVersionStr()
                              : APPVersion::instance().git_tag;
    btnReleaseChecker->title->setText(
        "wiliwili/setting/tools/others/release"_i18n + " (" +
        "hints/current"_i18n + ": " + version + ")");
    btnReleaseChecker->registerClickAction([](...) -> bool {
        // todo: 弹出一个提示提醒用户正在检查更新
        APPVersion::instance().checkUpdate(0, true);
        return true;
    });
    labelAboutVersion->setText(version);
    labelOpensource->setText(OPENSOURCE);

    auto& conf = ProgramConfig::instance();

    /// Hide bottom bar
    cellHideBar->init(
        "wiliwili/setting/app/others/hide_bottom"_i18n,
        conf.getBoolOption(SettingItem::HIDE_BOTTOM_BAR), [](bool value) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::HIDE_BOTTOM_BAR, value);
            // 更新设置
            brls::AppletFrame::HIDE_BOTTOM_BAR = value;

            // 修改所有正在显示的activity的底栏
            auto stack = brls::Application::getActivitiesStack();
            for (auto& activity : stack) {
                auto* frame = dynamic_cast<brls::AppletFrame*>(
                    activity->getContentView());
                if (!frame) continue;
                frame->setFooterVisibility(value ? brls::Visibility::GONE
                                                 : brls::Visibility::VISIBLE);
            }
        });

    /// Hide FPS
    cellHideFPS->init("wiliwili/setting/app/others/hide_fps"_i18n,
                      conf.getBoolOption(SettingItem::HIDE_FPS),
                      [](bool value) {
                          ProgramConfig::instance().setSettingItem(
                              SettingItem::HIDE_FPS, value);
                          brls::Application::setFPSStatus(!value);
                      });

/// Gamepad vibration
#ifdef __SWITCH__
    cellVibration->init("wiliwili/setting/app/others/vibration"_i18n,
                        conf.getBoolOption(SettingItem::GAMEPAD_VIBRATION),
                        [](bool value) {
                            ProgramConfig::instance().setSettingItem(
                                SettingItem::GAMEPAD_VIBRATION, value);
                            VibrationHelper::GAMEPAD_VIBRATION = value;
                        });
#else
    cellVibration->setVisibility(brls::Visibility::GONE);
#endif

/// Fullscreen
#if defined(__linux__) || defined(_WIN32)
    cellFullscreen->init(
        "wiliwili/setting/app/others/fullscreen"_i18n,
        conf.getBoolOption(SettingItem::FULLSCREEN), [](bool value) {
            ProgramConfig::instance().setSettingItem(SettingItem::FULLSCREEN,
                                                     value);
            // 更新设置
            VideoContext::FULLSCREEN = value;
            // 设置当前状态
            brls::Application::getPlatform()->getVideoContext()->fullScreen(
                value);
        });
#else
    cellFullscreen->setVisibility(brls::Visibility::GONE);
#endif

    /// App theme
    int themeData = conf.getStringOptionIndex(SettingItem::APP_THEME);
    selectorTheme->init(
        "wiliwili/setting/app/others/theme/header"_i18n,
        {"wiliwili/setting/app/others/theme/1"_i18n,
         "wiliwili/setting/app/others/theme/2"_i18n,
         "wiliwili/setting/app/others/theme/3"_i18n},
        themeData, [themeData](int data) {
            if (themeData == data) return false;
            auto optionData =
                ProgramConfig::instance().getOptionData(SettingItem::APP_THEME);
            ProgramConfig::instance().setSettingItem(
                SettingItem::APP_THEME, optionData.optionList[data]);
            DialogHelper::quitApp();
            return true;
        });

    /// App Keymap
#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
    int keyIndex = conf.getStringOptionIndex(SettingItem::KEYMAP);
    selectorKeymap->init(
        "wiliwili/setting/app/others/keymap/header"_i18n,
        {
            "wiliwili/setting/app/others/keymap/xbox"_i18n,
            "wiliwili/setting/app/others/keymap/ps"_i18n,
            "wiliwili/setting/app/others/keymap/keyboard"_i18n,
        },
        keyIndex, [keyIndex](int data) {
            if (keyIndex == data) return false;
            auto optionData =
                ProgramConfig::instance().getOptionData(SettingItem::KEYMAP);
            ProgramConfig::instance().setSettingItem(
                SettingItem::KEYMAP, optionData.optionList[data]);
            DialogHelper::quitApp();
            return true;
        });
#else
    selectorKeymap->setVisibility(brls::Visibility::GONE);
#endif

    /// App language
    int langIndex = conf.getStringOptionIndex(SettingItem::APP_LANG);
    selectorLang->init(
        "wiliwili/setting/app/others/language/header"_i18n,
        {
#ifdef __SWITCH__
            "wiliwili/setting/app/others/language/auto"_i18n,
#endif
            "wiliwili/setting/app/others/language/english"_i18n,
            "wiliwili/setting/app/others/language/japanese"_i18n,
            "wiliwili/setting/app/others/language/ryukyuan"_i18n,
            "wiliwili/setting/app/others/language/chinese_t"_i18n,
            "wiliwili/setting/app/others/language/chinese_s"_i18n,
            "wiliwili/setting/app/others/language/korean"_i18n,
        },
        langIndex, [langIndex](int data) {
            if (langIndex == data) return false;
            auto optionData =
                ProgramConfig::instance().getOptionData(SettingItem::APP_LANG);
            ProgramConfig::instance().setSettingItem(
                SettingItem::APP_LANG, optionData.optionList[data]);
            DialogHelper::quitApp();
            return true;
        });

    /// VideoFormat
    auto formatOption = conf.getOptionData(SettingItem::VIDEO_FORMAT);
    selectorFormat->init(
        "wiliwili/setting/app/playback/video_format"_i18n,
        formatOption.optionList,
        conf.getIntOptionIndex(SettingItem::VIDEO_FORMAT),
        [formatOption](int data) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::VIDEO_FORMAT, formatOption.rawOptionList[data]);
            bilibili::BilibiliClient::FNVAL =
                std::to_string(formatOption.rawOptionList[data]);
            return true;
        });

    /// Opencc
    if (brls::Application::getLocale() == brls::LOCALE_ZH_HANT ||
        brls::Application::getLocale() == brls::LOCALE_ZH_TW) {
        btnOpencc->init("wiliwili/setting/app/others/opencc"_i18n,
                        conf.getBoolOption(SettingItem::OPENCC_ON),
                        [](bool value) {
                            ProgramConfig::instance().setSettingItem(
                                SettingItem::OPENCC_ON, value);
                            DialogHelper::quitApp();
                        });
    } else {
        btnOpencc->setVisibility(brls::Visibility::GONE);
    }

    selectorTexture->init(
        "wiliwili/setting/app/image/texture"_i18n,
        {"100", "200 (" + "hints/preset"_i18n + ")", "300", "400", "500"},
        conf.getSettingItem(SettingItem::TEXTURE_CACHE_NUM, 200) / 100 - 1,
        [](int data) {
            int num = 100 * data + 100;
            ProgramConfig::instance().setSettingItem(
                SettingItem::TEXTURE_CACHE_NUM, num);
            brls::TextureCache::instance().cache.setCapacity(num);
        });

    /// Image request threads
    auto threadOption = conf.getOptionData(SettingItem::IMAGE_REQUEST_THREADS);
    selectorThreads->init(
        "wiliwili/setting/app/image/threads"_i18n, threadOption.optionList,
        conf.getIntOptionIndex(SettingItem::IMAGE_REQUEST_THREADS),
        [threadOption](int data) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::IMAGE_REQUEST_THREADS,
                threadOption.rawOptionList[data]);
            ImageHelper::setRequestThreads(threadOption.rawOptionList[data]);
        });

    selectorInmemory->init(
        "wiliwili/setting/app/playback/in_memory_cache"_i18n,
        {"0MB (" + "hints/off"_i18n + ")", "10MB", "20MB", "50MB", "100MB",
         "200MB", "500MB"},
        conf.getIntOptionIndex(SettingItem::PLAYER_INMEMORY_CACHE),
        [](int data) {
            auto inmemoryOption = ProgramConfig::instance().getOptionData(
                SettingItem::PLAYER_INMEMORY_CACHE);
            ProgramConfig::instance().setSettingItem(
                SettingItem::PLAYER_INMEMORY_CACHE,
                inmemoryOption.rawOptionList[data]);
            if (MPVCore::INMEMORY_CACHE == inmemoryOption.rawOptionList[data])
                return;
            MPVCore::INMEMORY_CACHE = inmemoryOption.rawOptionList[data];
            MPVCore::instance().restart();
        });

    /// Upload history record
    btnHistory->init("wiliwili/setting/app/playback/report"_i18n,
                     conf.getSettingItem(SettingItem::HISTORY_REPORT, true),
                     [](bool value) {
                         ProgramConfig::instance().setSettingItem(
                             SettingItem::HISTORY_REPORT, value);
                         VideoDetail::REPORT_HISTORY = value;
                     });

    btnAutoNextPart->init(
        "wiliwili/setting/app/playback/auto_play_next_part"_i18n,
        conf.getBoolOption(SettingItem::AUTO_NEXT_PART), [this](bool value) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::AUTO_NEXT_PART, value);
            BasePlayerActivity::AUTO_NEXT_PART = value;
            if (!value) {
                ProgramConfig::instance().setSettingItem(
                    SettingItem::AUTO_NEXT_RCMD, false);
                BasePlayerActivity::AUTO_NEXT_RCMD = false;
                btnAutoNextRcmd->setOn(false, btnAutoNextRcmd->isOn());
            }
        });

    btnAutoNextRcmd->init(
        "wiliwili/setting/app/playback/auto_play_recommend"_i18n,
        conf.getBoolOption(SettingItem::AUTO_NEXT_RCMD), [this](bool value) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::AUTO_NEXT_RCMD, value);
            BasePlayerActivity::AUTO_NEXT_RCMD = value;
            if (value) {
                ProgramConfig::instance().setSettingItem(
                    SettingItem::AUTO_NEXT_PART, true);
                BasePlayerActivity::AUTO_NEXT_PART = true;
                btnAutoNextPart->setOn(true, !btnAutoNextPart->isOn());
            }
        });

    /// Player bottom bar
    btnProgress->init("wiliwili/setting/app/playback/player_bar"_i18n,
                      conf.getBoolOption(SettingItem::PLAYER_BOTTOM_BAR),
                      [](bool value) {
                          ProgramConfig::instance().setSettingItem(
                              SettingItem::PLAYER_BOTTOM_BAR, value);
                          MPVCore::BOTTOM_BAR = value;
                      });

/// Hardware decode
#ifdef __SWITCH__
    btnHWDEC->setVisibility(brls::Visibility::GONE);
#else
    btnHWDEC->init("wiliwili/setting/app/playback/hwdec"_i18n,
                   conf.getBoolOption(SettingItem::PLAYER_HWDEC),
                   [](bool value) {
                       ProgramConfig::instance().setSettingItem(
                           SettingItem::PLAYER_HWDEC, value);
                       if (MPVCore::HARDWARE_DEC == value) return;
                       MPVCore::HARDWARE_DEC = value;
                       MPVCore::instance().restart();
                   });
#endif

    /// Decode quality
    btnQuality->init("wiliwili/setting/app/playback/low_quality"_i18n,
                     conf.getBoolOption(SettingItem::PLAYER_LOW_QUALITY),
                     [](bool value) {
                         ProgramConfig::instance().setSettingItem(
                             SettingItem::PLAYER_LOW_QUALITY, value);
                         if (MPVCore::LOW_QUALITY == value) return;
                         MPVCore::LOW_QUALITY = value;
                         MPVCore::instance().restart();
                     });
}

SettingActivity::~SettingActivity() {
    brls::Logger::debug("SettingActivity: delete");
}