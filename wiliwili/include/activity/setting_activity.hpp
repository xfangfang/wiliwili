//
// Created by fang on 2022/8/22.
//

#pragma once

#include <borealis.hpp>

class TextBox;

class SettingActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/setting_activity.xml");

    SettingActivity();

    void onContentAvailable() override;

    ~SettingActivity() override;

private:
    BRLS_BIND(brls::RadioCell, btnTutorialOpenApp, "tools/tutorial_open");
    BRLS_BIND(brls::RadioCell, btnTutorialOpenVideoIntro,
              "tools/tutorial_video_intro");
    BRLS_BIND(brls::RadioCell, btnTutorialError, "tools/tutorial_error");
    BRLS_BIND(brls::RadioCell, btnTutorialFont, "tools/tutorial_font");
    BRLS_BIND(brls::RadioCell, btnHotKey, "tools/hot_key");
    BRLS_BIND(brls::RadioCell, btnNetworkChecker, "tools/network_checker");
    BRLS_BIND(brls::RadioCell, btnReleaseChecker, "tools/release_checker");
    BRLS_BIND(brls::RadioCell, btnOpenConfig, "tools/config_dir");
    BRLS_BIND(brls::RadioCell, btnVibrationTest, "tools/vibration_test");
    BRLS_BIND(brls::SelectorCell, selectorLang, "setting/language");
    BRLS_BIND(brls::SelectorCell, selectorTheme, "setting/ui/theme");
    BRLS_BIND(brls::SelectorCell, selectorTexture, "setting/image/texture");
    BRLS_BIND(brls::SelectorCell, selectorThreads, "setting/image/threads");
    BRLS_BIND(brls::SelectorCell, selectorKeymap, "setting/keymap");
    BRLS_BIND(brls::BooleanCell, btnOpencc, "setting/opencc");
    BRLS_BIND(brls::BooleanCell, btnQuality, "setting/video/quality");
    BRLS_BIND(brls::BooleanCell, btnHWDEC, "setting/video/hwdec");
    BRLS_BIND(brls::SelectorCell, selectorInmemory, "setting/video/inmemory");
    BRLS_BIND(brls::SelectorCell, selectorFormat, "setting/video/format");
    BRLS_BIND(TextBox, labelOpensource, "setting/label/opensource");
    BRLS_BIND(brls::BooleanCell, cellHideBar, "cell/hideBottomBar");
    BRLS_BIND(brls::BooleanCell, cellHideFPS, "cell/hideFPS");
    BRLS_BIND(brls::BooleanCell, cellFullscreen, "cell/fullscreen");
    BRLS_BIND(brls::BooleanCell, cellVibration, "cell/gamepadVibration");
    BRLS_BIND(brls::Label, labelAboutVersion, "setting/about/version");
};