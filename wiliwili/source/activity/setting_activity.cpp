//
// Created by fang on 2022/8/22.
//

#include "activity/setting_activity.hpp"
#include "activity/hint_activity.hpp"
#include "activity/player_activity.hpp"
#include "fragment/setting_network.hpp"
#include "view/text_box.hpp"
#include "utils/config_helper.hpp"
#include "borealis/core/cache_helper.hpp"
#include "borealis/views/applet_frame.hpp"

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
}

void SettingActivity::onContentAvailable() {
    brls::Logger::debug("SettingActivity: onContentAvailable");

    btnTutorialOpenApp->registerClickAction([](...) -> bool {
        brls::Application::pushActivity(new HintActivity());
        return true;
    });

    btnTutorialOpenVideoIntro->registerClickAction([](...) -> bool {
        brls::Application::pushActivity(new PlayerActivity(
            "wiliwili/setting/tools/tutorial/intro_bvid"_i18n));
        return true;
    });

    btnTutorialError->registerClickAction([](...) -> bool {
        auto dialog =
            new brls::Dialog((brls::Box*)brls::View::createFromXMLResource(
                "fragment/settings_tutorial_error.xml"));
        dialog->addButton("hints/ok"_i18n, []() {});
        dialog->open();
        return true;
    });

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

    std::string version = APPVersion::instance().git_tag.empty()
                              ? "v" + APPVersion::instance().getVersionStr()
                              : APPVersion::instance().git_tag;
    btnReleaseChecker->title->setText(
        "wiliwili/setting/tools/others/release"_i18n + " (" +
        "hints/current"_i18n + ": " + version + ")");
    btnReleaseChecker->registerClickAction([](...) -> bool {
        brls::Application::getPlatform()->openBrowser(
            "https://github.com/xfangfang/wiliwili/releases/latest");
        return true;
    });

    labelOpensource->setText(OPENSOURCE);

    auto& conf = ProgramConfig::instance();

    cellHideBar->init(
        "wiliwili/setting/app/others/hide_bottom"_i18n,
        conf.getSettingItem(SettingItem::HIDE_BOTTOM_BAR, false),
        [this](bool value) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::HIDE_BOTTOM_BAR, value);
            // 更新设置
            brls::AppletFrame::HIDE_BOTTOM_BAR = value;
            // 设置当前底栏
            ((brls::AppletFrame*)this->getContentView())
                ->setFooterVisibility(value ? brls::Visibility::GONE
                                            : brls::Visibility::VISIBLE);
            // 设置MainActivity底栏
            ((brls::AppletFrame*)brls::Application::getActivitiesStack()[0]
                 ->getContentView())
                ->setFooterVisibility(value ? brls::Visibility::GONE
                                            : brls::Visibility::VISIBLE);
        });

#if defined(__linux__) || defined(_WIN32)
    cellFullscreen->init(
        "wiliwili/setting/app/others/fullscreen"_i18n,
        conf.getSettingItem(SettingItem::FULLSCREEN, true), [this](bool value) {
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

    int themeData = conf.getSettingItem(SettingItem::APP_THEME, 0);
    selectorTheme->init(
        "wiliwili/setting/app/others/theme/header"_i18n,
        {"wiliwili/setting/app/others/theme/1"_i18n,
         "wiliwili/setting/app/others/theme/2"_i18n,
         "wiliwili/setting/app/others/theme/3"_i18n},
        themeData, [themeData](int data) {
            if (themeData == data) return false;
            auto dialog = new brls::Dialog("wiliwili/setting/quit_hint"_i18n);
            dialog->addButton("hints/ok"_i18n, [data]() {
                ProgramConfig::instance().setSettingItem(SettingItem::APP_THEME,
                                                         data);
                // switch 在此模式下会重启app
                brls::Application::getPlatform()->exitToHomeMode(false);
                brls::Application::quit();
            });
            dialog->setCancelable(false);
            dialog->open();
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

    if (brls::Application::getLocale() == brls::LOCALE_ZH_HANT ||
        brls::Application::getLocale() == brls::LOCALE_ZH_TW) {
        btnOpencc->init(
            "wiliwili/setting/app/others/opencc"_i18n,
            conf.getSettingItem(SettingItem::OPENCC_ON, true), [](bool value) {
                auto dialog =
                    new brls::Dialog("wiliwili/setting/quit_hint"_i18n);
                dialog->addButton("hints/ok"_i18n, [value]() {
                    ProgramConfig::instance().setSettingItem(
                        SettingItem::OPENCC_ON, value);
                    brls::Application::getPlatform()->exitToHomeMode(false);
                    brls::Application::quit();
                });
                dialog->setCancelable(false);
                dialog->open();
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

    auto threadOption = conf.getOptionData(SettingItem::IMAGE_REQUEST_THREADS);
    selectorThreads->init(
        "wiliwili/setting/app/image/threads"_i18n, threadOption.optionList,
        conf.getIntOptionIndex(SettingItem::IMAGE_REQUEST_THREADS),
        [threadOption](int data) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::IMAGE_REQUEST_THREADS,
                threadOption.rawOptionList[data]);
            ImageHelper::REQUEST_THREADS = threadOption.rawOptionList[data];
        });

    // todo: 从config_helper中实现一个可通用的选项选择方式
    std::vector<int> inmemoryData = {0, 10, 20, 50, 100, 200, 500};
    int inmemory = conf.getSettingItem(SettingItem::PLAYER_INMEMORY_CACHE, 10);
    size_t inmemorySelect = 1;
    for (size_t i = 0; i < inmemoryData.size(); i++) {
        inmemorySelect = i;
        if (inmemory <= inmemoryData[i]) break;
    }
    selectorInmemory->init(
        "wiliwili/setting/app/playback/in_memory_cache"_i18n,
        {"0MB (" + "hints/off"_i18n + ")", "10MB (" + "hints/preset"_i18n + ")",
         "20MB", "50MB", "100MB", "200MB", "500MB"},
        inmemorySelect, [inmemoryData](int data) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::PLAYER_INMEMORY_CACHE, inmemoryData[data]);
            if (MPVCore::INMEMORY_CACHE == inmemoryData[data]) return;
            MPVCore::INMEMORY_CACHE = inmemoryData[data];
            MPVCore::instance().restart();
        });

    btnHistory->init("wiliwili/setting/app/playback/report"_i18n,
                     conf.getSettingItem(SettingItem::HISTORY_REPORT, true),
                     [](bool value) {
                         ProgramConfig::instance().setSettingItem(
                             SettingItem::HISTORY_REPORT, value);
                         VideoDetail::REPORT_HISTORY = value;
                     });

    btnAutoNextPart->init(
        "wiliwili/setting/app/playback/auto_play_next_part"_i18n,
        conf.getSettingItem(SettingItem::AUTO_NEXT_PART, true),
        [this](bool value) {
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
        conf.getSettingItem(SettingItem::AUTO_NEXT_RCMD, true),
        [this](bool value) {
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

    btnProgress->init("wiliwili/setting/app/playback/player_bar"_i18n,
                      conf.getSettingItem(SettingItem::PLAYER_BOTTOM_BAR, true),
                      [](bool value) {
                          ProgramConfig::instance().setSettingItem(
                              SettingItem::PLAYER_BOTTOM_BAR, value);
                          MPVCore::BOTTOM_BAR = value;
                      });

#ifdef __SWITCH__
    btnHWDEC->setVisibility(brls::Visibility::GONE);
#else
    btnHWDEC->init("wiliwili/setting/app/playback/hwdec"_i18n,
                   conf.getSettingItem(SettingItem::PLAYER_HWDEC, true),
                   [](bool value) {
                       ProgramConfig::instance().setSettingItem(
                           SettingItem::PLAYER_HWDEC, value);
                       if (MPVCore::HARDWARE_DEC == value) return;
                       MPVCore::HARDWARE_DEC = value;
                       MPVCore::instance().restart();
                   });
#endif

#ifdef __SWITCH__
    bool defaultValue = true;
#else
    bool defaultValue = false;
#endif
    btnQuality->init(
        "wiliwili/setting/app/playback/low_quality"_i18n,
        conf.getSettingItem(SettingItem::PLAYER_LOW_QUALITY, defaultValue),
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