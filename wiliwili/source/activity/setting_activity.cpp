//
// Created by fang on 2022/8/22.
//

#include <pystring.h>
#include <borealis/core/i18n.hpp>
#include <borealis/core/application.hpp>
#include <borealis/core/cache_helper.hpp>
#include <borealis/views/applet_frame.hpp>
#include <borealis/views/dialog.hpp>
#include <borealis/views/cells/cell_bool.hpp>
#include <borealis/views/cells/cell_input.hpp>

#include "bilibili.h"
#include "activity/setting_activity.hpp"
#include "activity/hint_activity.hpp"
#include "activity/search_activity_tv.hpp"
#include "fragment/setting_network.hpp"
#include "fragment/test_rumble.hpp"
#include "utils/config_helper.hpp"
#include "utils/vibration_helper.hpp"
#include "utils/dialog_helper.hpp"
#include "utils/activity_helper.hpp"
#include "view/text_box.hpp"
#include "view/selector_cell.hpp"
#include "view/mpv_core.hpp"

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
#include "borealis/platforms/desktop/desktop_platform.hpp"
#endif

#ifdef __linux__
#include "borealis/platforms/desktop/steam_deck.hpp"
#endif

using namespace brls::literals;

const std::string OPENSOURCE =
    "--------------------------------\n"
    "FFmpeg\n"
    "--------------------------------\n"
    "Official site:    https://www.ffmpeg.org\n\n"
    "Copyright (c) FFmpeg developers and contributors.\n\n"
    "Licensed under LGPLv2.1 or later\n\n\n"
    "--------------------------------\n"
    "mpv\n"
    "--------------------------------\n"
    "Official site:    https://mpv.io\n\n"
    "Copyright (c) mpv developers and contributors.\n\n"
    "Licensed under GPL-2.0 or LGPLv2.1\n\n\n"
    "--------------------------------\n"
    "borealis\n"
    "--------------------------------\n"
    "https://github.com/natinusala/borealis\n\n"
    "Modifications for touch and recycler list support:\n"
    "https://github.com/XITRIX/borealis\n\n"
    "Modified version for more system support:\n"
    "https://github.com/xfangfang/borealis\n\n"
    "Copyright (c) 2019-2022, natinusala and contributors.\n"
    "Copyright (c) XITRIX.\n\n"
    "Licensed under Apache-2.0 license\n\n\n"
    "--------------------------------\n"
    "OpenCC\n"
    "--------------------------------\n"
    "https://github.com/BYVoid/OpenCC\n\n"
    "Copyright (c) Carbo Kuo and contributors.\n\n"
    "Modified version: https://github.com/xfangfang/OpenCC\n\n"
    "Licensed under Apache-2.0 license\n\n\n"
    "--------------------------------\n"
    "pystring\n"
    "--------------------------------\n"
    "https://github.com/imageworks/pystring\n\n"
    "Copyright (c) imageworks and contributors.\n\n"
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
    "Copyright (c) 2017-2021 Huu Nguyen.\n"
    "Copyright (c) 2022 libcpr and many other contributors.\n\n"
    "Licensed under MIT license\n\n\n"
    "--------------------------------\n"
    "mongoose\n"
    "--------------------------------\n"
    "Official site:    https://mongoose.ws\n"
    "https://github.com/cesanta/mongoose\n\n"
    "Copyright (c) 2004-2013 Sergey Lyubka\n"
    "Copyright (c) 2013-2023 Cesanta Software Limited\n\n"
    "Licensed under GPL-2.0 or GPL without warranty\n\n\n"
#ifdef USE_WEBP
    "--------------------------------\n"
    "libwebp\n"
    "--------------------------------\n"
    "https://chromium.googlesource.com/webm/libwebp\n\n"
    "Copyright (c) Google Inc. All Rights Reserved.\n\n"
    "Licensed under BSD 3-Clause \"New\" or \"Revised\" License\n\n\n"
#endif
#ifdef __SWITCH__
    "--------------------------------\n"
    "nx\n"
    "--------------------------------\n"
    "https://github.com/switchbrew/libnx\n\n"
    "Copyright 2017-2018 libnx Authors.\n\nPublic domain\n\n\n"
    "--------------------------------\n"
    "devkitPro\n"
    "--------------------------------\n"
    "https://devkitpro.org\n\n"
    "Copyright devkitPro Authors.\n\n"
    "Public domain\n"
#endif
#ifdef __PSV__
    "--------------------------------\n"
    "vitasdk\n"
    "--------------------------------\n"
    "https://github.com/vitasdk\n\n"
    "Copyright vitasdk Authors.\n\n"
    "Public domain\n"
#endif
#ifdef PS4
    "--------------------------------\n"
    "pacbrew\n"
    "--------------------------------\n"
    "https://github.com/PacBrew/pacbrew-packages\n\n"
    "Copyright PacBrew Authors.\n\n"
    "Public domain\n\n\n"
    "--------------------------------\n"
    "OpenOrbis-PS4-Toolchain\n"
    "--------------------------------\n"
    "https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain\n\n"
    "Copyright OpenOrbis Authors.\n\n"
    "Licensed under GPL-3.0\n"
#endif
    "\n";

SettingActivity::SettingActivity() {
    brls::Logger::debug("SettingActivity: create");
    GA("open_setting")
}

void SettingActivity::onContentAvailable() {
    brls::Logger::debug("SettingActivity: onContentAvailable");

#ifdef __SWITCH__
    btnTutorialOpenApp->registerClickAction([](...) -> bool {
        Intent::openHint();
        return true;
    });
#else
    btnTutorialOpenApp->setVisibility(brls::Visibility::GONE);
#endif

    btnTutorialOpenVideoIntro->registerClickAction([](...) -> bool {
        Intent::openCollection("2511565362");
        return true;
    });

    btnTutorialWiki->registerClickAction([](...) -> bool {
        brls::Application::getPlatform()->openBrowser("https://github.com/xfangfang/wiliwili/wiki");
        return true;
    });

#ifdef __SWITCH__
    btnTutorialError->registerClickAction([](...) -> bool {
        auto dialog =
            new brls::Dialog((brls::Box*)brls::View::createFromXMLResource("fragment/settings_tutorial_error.xml"));
        dialog->addButton("hints/ok"_i18n, []() {});
        dialog->open();
        return true;
    });
#else
    btnTutorialError->setVisibility(brls::Visibility::GONE);
#endif

    btnDLNA->registerClickAction([](...) -> bool {
        Intent::openDLNA();
        return true;
    });

#if !defined(__SWITCH__) && !defined(IOS) && !defined(__PSV__) && !defined(PS4)
    btnOpenConfig->registerClickAction([](...) -> bool {
        auto* p = (brls::DesktopPlatform*)brls::Application::getPlatform();
        p->openBrowser(ProgramConfig::instance().getConfigDir());
        return true;
    });
#ifdef __linux__
    if (brls::isSteamDeck()) {
        btnOpenConfig->setVisibility(brls::Visibility::GONE);
    }
#endif
#else
    btnOpenConfig->setVisibility(brls::Visibility::GONE);
#endif
    btnTutorialFont->registerClickAction([](...) -> bool {
        auto dialog =
            new brls::Dialog((brls::Box*)brls::View::createFromXMLResource("fragment/settings_tutorial_font.xml"));
        dialog->addButton("hints/ok"_i18n, []() {});
        dialog->open();
        return true;
    });

    btnHotKey->registerClickAction([](...) -> bool {
        auto dialog = new brls::Dialog((brls::Box*)brls::View::createFromXMLResource("fragment/settings_hot_keys.xml"));
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

    std::string version = APPVersion::instance().git_tag.empty() ? "v" + APPVersion::instance().getVersionStr()
                                                                 : APPVersion::instance().git_tag;
    btnReleaseChecker->title->setText("wiliwili/setting/tools/others/release"_i18n + " (" + "hints/current"_i18n +
                                      ": " + version + ")");
    btnReleaseChecker->registerClickAction([](...) -> bool {
        // todo: 弹出一个提示提醒用户正在检查更新
        APPVersion::instance().checkUpdate(0, true);
        return true;
    });

    labelAboutVersion->setText(version
#if defined(BOREALIS_USE_DEKO3D)
                                + " (deko3d)"
#elif defined(BOREALIS_USE_OPENGL)
#if defined(USE_GL2)
                                + " (OpenGL2)"
#elif defined(USE_GLES2)
                                + " (OpenGL ES2)"
#elif defined(USE_GLES3)
                                + " (OpenGL ES3)"
#else
                                + " (OpenGL)"
#endif
#elif defined(BOREALIS_USE_D3D11)
                                + " (D3D11)"
#endif
    );
    labelOpensource->setText(OPENSOURCE);

    /// Quit APP
#ifdef IOS
    btnQuit->setVisibility(brls::Visibility::GONE);
#else
    btnQuit->registerClickAction([](...) -> bool {
        auto dialog = new brls::Dialog("hints/exit_hint"_i18n);
        dialog->addButton("hints/cancel"_i18n, []() {});
        dialog->addButton("hints/ok"_i18n, []() { brls::Application::quit(); });
        dialog->open();
        return true;
    });
#endif

    auto& conf = ProgramConfig::instance();

    /// Hide bottom bar
    cellHideBar->init("wiliwili/setting/app/others/hide_bottom"_i18n, conf.getBoolOption(SettingItem::HIDE_BOTTOM_BAR),
                      [this](bool value) {
                          ProgramConfig::instance().setSettingItem(SettingItem::HIDE_BOTTOM_BAR, value);
                          // 更新设置
                          brls::AppletFrame::HIDE_BOTTOM_BAR = value;

                          // 修改所有正在显示的activity的底栏
                          auto stack = brls::Application::getActivitiesStack();
                          for (auto& activity : stack) {
                              auto* frame = dynamic_cast<brls::AppletFrame*>(activity->getContentView());
                              if (!frame) continue;
                              frame->setFooterVisibility(value ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
                          }

                          if (value) {
                              ProgramConfig::instance().setSettingItem(SettingItem::HIDE_FPS, true);
                              brls::Application::setFPSStatus(false);
                          }
                          this->cellHideFPS->setOn(true);
                      });

    /// Hide FPS
    cellHideFPS->init("wiliwili/setting/app/others/hide_fps"_i18n, conf.getBoolOption(SettingItem::HIDE_FPS),
                      [](bool value) {
                          ProgramConfig::instance().setSettingItem(SettingItem::HIDE_FPS, value);
                          brls::Application::setFPSStatus(!value);
                      });

    /// Limited FPS
    auto fpsOption = conf.getOptionData(SettingItem::LIMITED_FPS);
    selectorFPS->init("wiliwili/setting/app/others/limited_fps"_i18n,
                      {"wiliwili/setting/app/others/limited_fps_vsync"_i18n, "30", "60", "90", "120"},
                      (size_t)conf.getIntOptionIndex(SettingItem::LIMITED_FPS), [fpsOption](int data) {
                          int fps = fpsOption.rawOptionList[data];
                          brls::Application::setLimitedFPS(fps);
                          ProgramConfig::instance().setSettingItem(SettingItem::LIMITED_FPS, fps);
                          return true;
                      });

    /// TV Search Mode
    cellTvSearch->init("wiliwili/setting/app/others/tv_search"_i18n, conf.getBoolOption(SettingItem::SEARCH_TV_MODE),
                       [](bool value) {
                           ProgramConfig::instance().setSettingItem(SettingItem::SEARCH_TV_MODE, value);
                           TVSearchActivity::TV_MODE = value;
                       });

    /// TV OSD Control Mode
    cellTvOSD->init(
        "wiliwili/setting/app/ui/tv_osd"_i18n, conf.getBoolOption(SettingItem::PLAYER_OSD_TV_MODE),
        [](bool value) { ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_OSD_TV_MODE, value); });

/// Gamepad vibration
#ifdef __SWITCH__
    cellVibration->init("wiliwili/setting/app/others/vibration"_i18n,
                        conf.getBoolOption(SettingItem::GAMEPAD_VIBRATION), [](bool value) {
                            ProgramConfig::instance().setSettingItem(SettingItem::GAMEPAD_VIBRATION, value);
                            VibrationHelper::GAMEPAD_VIBRATION = value;
                        });
#else
    cellVibration->setVisibility(brls::Visibility::GONE);
#endif

/// Fullscreen
#ifdef ALLOW_FULLSCREEN
    cellFullscreen->init("wiliwili/setting/app/others/fullscreen"_i18n, conf.getBoolOption(SettingItem::FULLSCREEN),
                         [](bool value) {
                             ProgramConfig::instance().setSettingItem(SettingItem::FULLSCREEN, value);
                             // 更新设置
                             VideoContext::FULLSCREEN = value;
                             // 设置当前状态
                             brls::Application::getPlatform()->getVideoContext()->fullScreen(value);
                         });

    cellAlwaysOnTop->init("wiliwili/setting/app/others/always_on_top"_i18n, conf.getBoolOption(SettingItem::ALWAYS_ON_TOP),
                         [](bool value) {
                             ProgramConfig::instance().setSettingItem(SettingItem::ALWAYS_ON_TOP, value);
                             // 设置当前状态
                             brls::Application::getPlatform()->setWindowAlwaysOnTop(value);
                         });
#else
    cellFullscreen->setVisibility(brls::Visibility::GONE);
    cellAlwaysOnTop->setVisibility(brls::Visibility::GONE);
#endif

    /// App theme
    static int themeData = conf.getStringOptionIndex(SettingItem::APP_THEME);
    selectorTheme->init("wiliwili/setting/app/others/theme/header"_i18n,
                        {"wiliwili/setting/app/others/theme/1"_i18n, "wiliwili/setting/app/others/theme/2"_i18n,
                         "wiliwili/setting/app/others/theme/3"_i18n},
                        themeData, [](int data) {
                            if (themeData == data) return false;
                            themeData       = data;
                            auto optionData = ProgramConfig::instance().getOptionData(SettingItem::APP_THEME);
                            ProgramConfig::instance().setSettingItem(SettingItem::APP_THEME,
                                                                     optionData.optionList[data]);
                            DialogHelper::quitApp();
                            return true;
                        });

    /// App custom theme
    std::string customThemeID = conf.getSettingItem(SettingItem::APP_RESOURCES, std::string{""});
    conf.loadCustomThemes();
    auto customThemeList = conf.getCustomThemes();
    if (customThemeList.empty()) {
        selectorCustomTheme->setVisibility(brls::Visibility::GONE);
    } else {
        std::vector<std::string> customThemeNameList = {"hints/off"_i18n};
        int customThemeIndex                         = 0;
        for (size_t index = 0; index < customThemeList.size(); index++) {
            customThemeNameList.emplace_back(customThemeList[index].name);
            if (customThemeID == customThemeList[index].id) {
                customThemeIndex = index + 1;
            }
        }
        selectorCustomTheme->init("wiliwili/setting/app/others/custom_theme/header"_i18n, customThemeNameList,
                                  customThemeIndex, [customThemeIndex, customThemeList](int data) {
                                      if (customThemeIndex == data) return false;
                                      if (data <= 0) {
                                          ProgramConfig::instance().setSettingItem(SettingItem::APP_RESOURCES, "");
                                      } else {
                                          ProgramConfig::instance().setSettingItem(SettingItem::APP_RESOURCES,
                                                                                   customThemeList[data - 1].id);
                                      }

                                      DialogHelper::quitApp();
                                      return true;
                                  });
    }

    // APP UI Scale
    static int UIScaleIndex = conf.getStringOptionIndex(SettingItem::APP_UI_SCALE);
    selectorUIScale->init("wiliwili/setting/app/others/scale/header"_i18n,
                          {
                              "wiliwili/setting/app/others/scale/544p"_i18n,
                              "wiliwili/setting/app/others/scale/720p"_i18n,
                              "wiliwili/setting/app/others/scale/900p"_i18n,
                              "wiliwili/setting/app/others/scale/1080p"_i18n,
                          },
                          UIScaleIndex, [](int data) {
                              if (UIScaleIndex == data) return false;
                              UIScaleIndex    = data;
                              auto optionData = ProgramConfig::instance().getOptionData(SettingItem::APP_UI_SCALE);
                              ProgramConfig::instance().setSettingItem(SettingItem::APP_UI_SCALE,
                                                                       optionData.optionList[data]);
                              DialogHelper::quitApp();
                              return true;
                          });

    /// App Keymap
#if !defined(__SWITCH__) && !defined(__PSV__) && !defined(PS4)
    static int keyIndex = conf.getStringOptionIndex(SettingItem::KEYMAP);
    selectorKeymap->init("wiliwili/setting/app/others/keymap/header"_i18n,
                         {
                             "wiliwili/setting/app/others/keymap/xbox"_i18n,
                             "wiliwili/setting/app/others/keymap/ps"_i18n,
                             "wiliwili/setting/app/others/keymap/keyboard"_i18n,
                         },
                         keyIndex, [](int data) {
                             if (keyIndex == data) return false;
                             keyIndex        = data;
                             auto optionData = ProgramConfig::instance().getOptionData(SettingItem::KEYMAP);
                             ProgramConfig::instance().setSettingItem(SettingItem::KEYMAP, optionData.optionList[data]);
                             DialogHelper::quitApp();
                             return true;
                         });
#else
    selectorKeymap->setVisibility(brls::Visibility::GONE);
#endif

    /// App language
    static int langIndex = conf.getStringOptionIndex(SettingItem::APP_LANG);
    selectorLang->init("wiliwili/setting/app/others/language/header"_i18n,
                       {
#if defined(__SWITCH__) || defined(__PSV__) || defined(PS4)
                           "wiliwili/setting/app/others/language/auto"_i18n,
#endif
                           "wiliwili/setting/app/others/language/english"_i18n,
                           "wiliwili/setting/app/others/language/japanese"_i18n,
                           "wiliwili/setting/app/others/language/ryukyuan"_i18n,
                           "wiliwili/setting/app/others/language/chinese_t"_i18n,
                           "wiliwili/setting/app/others/language/chinese_s"_i18n,
                           "wiliwili/setting/app/others/language/korean"_i18n,
                           "wiliwili/setting/app/others/language/italiano"_i18n,
                       },
                       langIndex, [](int data) {
                           if (langIndex == data) return false;
                           langIndex       = data;
                           auto optionData = ProgramConfig::instance().getOptionData(SettingItem::APP_LANG);
                           ProgramConfig::instance().setSettingItem(SettingItem::APP_LANG, optionData.optionList[data]);
                           DialogHelper::quitApp();
                           return true;
                       });

    /// VideoCodec
    auto codecOption = conf.getOptionData(SettingItem::VIDEO_CODEC);
    selectorCodec->init("wiliwili/setting/app/playback/video_codec"_i18n, codecOption.optionList,
                        conf.getIntOptionIndex(SettingItem::VIDEO_CODEC), [codecOption](int data) {
                            ProgramConfig::instance().setSettingItem(SettingItem::VIDEO_CODEC,
                                                                     codecOption.rawOptionList[data]);
                            bilibili::BilibiliClient::VIDEO_CODEC = codecOption.rawOptionList[data];
                            return true;
                        });

    /// AudioBandwidth
    auto bandwidthOption = conf.getOptionData(SettingItem::AUDIO_QUALITY);
    selectorQuality->init(
        "wiliwili/setting/app/playback/audio_quality"_i18n,
        {"wiliwili/home/common/high"_i18n, "wiliwili/home/common/medium"_i18n, "wiliwili/home/common/low"_i18n},
        conf.getIntOptionIndex(SettingItem::AUDIO_QUALITY), [bandwidthOption](int data) {
            ProgramConfig::instance().setSettingItem(SettingItem::AUDIO_QUALITY, bandwidthOption.rawOptionList[data]);
            bilibili::BilibiliClient::AUDIO_QUALITY = bandwidthOption.rawOptionList[data];
            return true;
        });

    /// VideoFormat
    auto formatOption = conf.getOptionData(SettingItem::VIDEO_FORMAT);
    selectorFormat->init("wiliwili/setting/app/playback/video_format"_i18n, formatOption.optionList,
                         conf.getIntOptionIndex(SettingItem::VIDEO_FORMAT), [this, formatOption](int data) {
                             ProgramConfig::instance().setSettingItem(SettingItem::VIDEO_FORMAT,
                                                                      formatOption.rawOptionList[data]);
                             bilibili::BilibiliClient::FNVAL = std::to_string(formatOption.rawOptionList[data]);
                             // 非 Dash 模式，无法调整视频编码与音频质量
                             if (formatOption.rawOptionList[data] == 0) {
                                 selectorCodec->setVisibility(brls::Visibility::GONE);
                                 selectorQuality->setVisibility(brls::Visibility::GONE);
                             } else {
                                 selectorCodec->setVisibility(brls::Visibility::VISIBLE);
                                 selectorQuality->setVisibility(brls::Visibility::VISIBLE);
                             }
                             return true;
                         });

    // 非 Dash 模式，无法调整视频编码与音频质量
    if (conf.getIntOption(SettingItem::VIDEO_FORMAT) == 0) {
        selectorCodec->setVisibility(brls::Visibility::GONE);
        selectorQuality->setVisibility(brls::Visibility::GONE);
    }

    /// Opencc
#if defined(IOS) || defined(DISABLE_OPENCC)
    btnOpencc->setVisibility(brls::Visibility::GONE);
#else
    if (brls::Application::getLocale() == brls::LOCALE_ZH_HANT ||
        brls::Application::getLocale() == brls::LOCALE_ZH_TW) {
        btnOpencc->init("wiliwili/setting/app/others/opencc"_i18n, conf.getBoolOption(SettingItem::OPENCC_ON),
                        [](bool value) {
                            ProgramConfig::instance().setSettingItem(SettingItem::OPENCC_ON, value);
                            DialogHelper::quitApp();
                        });
    } else {
        btnOpencc->setVisibility(brls::Visibility::GONE);
    }
#endif

#if defined(__PSV__) || defined(PS4)
    selectorTexture->setVisibility(brls::Visibility::GONE);
#else
    selectorTexture->init("wiliwili/setting/app/image/texture"_i18n,
                          {"100", "200 (" + "hints/preset"_i18n + ")", "300", "400", "500"},
                          conf.getSettingItem(SettingItem::TEXTURE_CACHE_NUM, 200) / 100 - 1, [](int data) {
                              int num = 100 * data + 100;
                              ProgramConfig::instance().setSettingItem(SettingItem::TEXTURE_CACHE_NUM, num);
                              brls::TextureCache::instance().cache.setCapacity(num);
                          });
#endif

    /// Image request threads
    auto threadOption = conf.getOptionData(SettingItem::IMAGE_REQUEST_THREADS);
    selectorThreads->init("wiliwili/setting/app/image/threads"_i18n, threadOption.optionList,
                          conf.getIntOptionIndex(SettingItem::IMAGE_REQUEST_THREADS), [threadOption](int data) {
                              ProgramConfig::instance().setSettingItem(SettingItem::IMAGE_REQUEST_THREADS,
                                                                       threadOption.rawOptionList[data]);
                              ImageHelper::setRequestThreads(threadOption.rawOptionList[data]);
                          });

    selectorInmemory->init("wiliwili/setting/app/playback/in_memory_cache"_i18n,
#ifdef __PSV__
                           {"0MB (" + "hints/off"_i18n + ")", "1MB", "5MB", "10MB"},
#else
        {"0MB (" + "hints/off"_i18n + ")", "10MB", "20MB", "50MB", "100MB"},
#endif
                           conf.getIntOptionIndex(SettingItem::PLAYER_INMEMORY_CACHE), [](int data) {
                               auto inmemoryOption =
                                   ProgramConfig::instance().getOptionData(SettingItem::PLAYER_INMEMORY_CACHE);
                               ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_INMEMORY_CACHE,
                                                                        inmemoryOption.rawOptionList[data]);
                               if (MPVCore::INMEMORY_CACHE == inmemoryOption.rawOptionList[data]) return;
                               MPVCore::INMEMORY_CACHE = inmemoryOption.rawOptionList[data];
                               MPVCore::instance().restart();
                           });

    /// TLS verify
    btnTls->init("wiliwili/setting/app/network/tls"_i18n, conf.getBoolOption(SettingItem::TLS_VERIFY), [](bool data) {
        auto& conf = ProgramConfig::instance();
        conf.setSettingItem(SettingItem::TLS_VERIFY, data);
        conf.setTlsVerify(data);
    });

    /// HTTP proxy
    bool httpProxyStatus = conf.getBoolOption(SettingItem::HTTP_PROXY_STATUS);
    btnProxy->init("wiliwili/setting/app/network/proxy"_i18n, httpProxyStatus, [this](bool data) {
        auto& conf = ProgramConfig::instance();
        conf.setSettingItem(SettingItem::HTTP_PROXY_STATUS, data);
        btnProxyInput->setVisibility(data ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
        conf.setProxy(data ? conf.getSettingItem(SettingItem::HTTP_PROXY, std::string{""}) : "");
    });

    btnProxyInput->setVisibility(httpProxyStatus ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
    auto httpProxy = conf.getSettingItem(SettingItem::HTTP_PROXY, std::string{""});
    btnProxyInput->init(
        "wiliwili/setting/app/network/proxy"_i18n, httpProxy,
        [](const std::string& data) {
            std::string httpProxy = pystring::strip(data);
            // 如果没有写协议，默认用 http
            if (!httpProxy.empty() && !pystring::startswith(httpProxy, "http://") &&
                !pystring::startswith(httpProxy, "https://") && !pystring::startswith(httpProxy, "socks5://")) {
                httpProxy = "http://" + httpProxy;
            }
            ProgramConfig::instance().setSettingItem(SettingItem::HTTP_PROXY, httpProxy);
            ProgramConfig::instance().setProxy(httpProxy);
        },
        "http://127.0.0.1:7890", "wiliwili/setting/app/network/proxy_hint"_i18n);

/// Hardware decode
#ifdef PS4
    btnHWDEC->setVisibility(brls::Visibility::GONE);
#else
    btnHWDEC->init("wiliwili/setting/app/playback/hwdec"_i18n, conf.getBoolOption(SettingItem::PLAYER_HWDEC),
                   [](bool value) {
                       ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_HWDEC, value);
                       if (MPVCore::HARDWARE_DEC == value) return;
                       MPVCore::HARDWARE_DEC = value;
                       MPVCore::instance().restart();
                   });
#endif

    /// Decode quality
    btnQuality->init("wiliwili/setting/app/playback/low_quality"_i18n,
                     conf.getBoolOption(SettingItem::PLAYER_LOW_QUALITY), [](bool value) {
                         ProgramConfig::instance().setSettingItem(SettingItem::PLAYER_LOW_QUALITY, value);
                         if (MPVCore::LOW_QUALITY == value) return;
                         MPVCore::LOW_QUALITY = value;
                         MPVCore::instance().restart();
                     });
}

SettingActivity::~SettingActivity() { brls::Logger::debug("SettingActivity: delete"); }
