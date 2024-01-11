//
// Created by fang on 2022/8/22.
//

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>

namespace brls {
class RadioCell;
class BooleanCell;
class InputCell;
class Label;
}  // namespace brls

class TextBox;
class SelectorCell;

class SettingActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/setting_activity.xml");

    SettingActivity();

    void onContentAvailable() override;

    ~SettingActivity() override;

private:
    BRLS_BIND(brls::RadioCell, btnTutorialOpenApp, "tools/tutorial_open");
    BRLS_BIND(brls::RadioCell, btnTutorialOpenVideoIntro, "tools/tutorial_video_intro");
    BRLS_BIND(brls::RadioCell, btnTutorialWiki, "tools/tutorial_wiki");
    BRLS_BIND(brls::RadioCell, btnTutorialError, "tools/tutorial_error");
    BRLS_BIND(brls::RadioCell, btnTutorialFont, "tools/tutorial_font");
    BRLS_BIND(brls::RadioCell, btnHotKey, "tools/hot_key");
    BRLS_BIND(brls::RadioCell, btnNetworkChecker, "tools/network_checker");
    BRLS_BIND(brls::RadioCell, btnReleaseChecker, "tools/release_checker");
    BRLS_BIND(brls::RadioCell, btnQuit, "tools/quit");
    BRLS_BIND(brls::RadioCell, btnOpenConfig, "tools/config_dir");
    BRLS_BIND(brls::RadioCell, btnVibrationTest, "tools/vibration_test");
    BRLS_BIND(brls::RadioCell, btnDLNA, "tools/dlna");
    BRLS_BIND(brls::BooleanCell, btnTls, "setting/network/tls");
    BRLS_BIND(brls::BooleanCell, btnProxy, "setting/network/proxy");
    BRLS_BIND(brls::InputCell, btnProxyInput, "setting/network/input");
    BRLS_BIND(SelectorCell, selectorLang, "setting/language");
    BRLS_BIND(SelectorCell, selectorTheme, "setting/ui/theme");
    BRLS_BIND(SelectorCell, selectorCustomTheme, "setting/custom/theme");
    BRLS_BIND(SelectorCell, selectorUIScale, "setting/ui/scale");
    BRLS_BIND(SelectorCell, selectorTexture, "setting/image/texture");
    BRLS_BIND(SelectorCell, selectorThreads, "setting/image/threads");
    BRLS_BIND(SelectorCell, selectorKeymap, "setting/keymap");
    BRLS_BIND(brls::BooleanCell, btnOpencc, "setting/opencc");
    BRLS_BIND(brls::BooleanCell, btnQuality, "setting/video/quality");
    BRLS_BIND(brls::BooleanCell, btnHWDEC, "setting/video/hwdec");
    BRLS_BIND(SelectorCell, selectorInmemory, "setting/video/inmemory");
    BRLS_BIND(SelectorCell, selectorFormat, "setting/video/format");
    BRLS_BIND(SelectorCell, selectorCodec, "setting/video/codec");
    BRLS_BIND(SelectorCell, selectorQuality, "setting/audio/quality");
    BRLS_BIND(SelectorCell, selectorFPS, "setting/fps");
    BRLS_BIND(TextBox, labelOpensource, "setting/label/opensource");
    BRLS_BIND(brls::BooleanCell, cellHideBar, "cell/hideBottomBar");
    BRLS_BIND(brls::BooleanCell, cellHideFPS, "cell/hideFPS");
    BRLS_BIND(brls::BooleanCell, cellTvSearch, "cell/tvSearch");
    BRLS_BIND(brls::BooleanCell, cellFullscreen, "cell/fullscreen");
    BRLS_BIND(brls::BooleanCell, cellVibration, "cell/gamepadVibration");
    BRLS_BIND(brls::Label, labelAboutVersion, "setting/about/version");
};