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

    ~SettingActivity();

private:
    BRLS_BIND(brls::RadioCell, btnTutorialOpenApp, "tools/tutorial_open");
    BRLS_BIND(brls::RadioCell, btnTutorialOpenVideoIntro,
              "tools/tutorial_video_intro");
    BRLS_BIND(brls::RadioCell, btnTutorialError, "tools/tutorial_error");
    BRLS_BIND(brls::RadioCell, btnNetworkChecker, "tools/network_checker");
    BRLS_BIND(brls::RadioCell, btnReleaseChecker, "tools/release_checker");
    BRLS_BIND(brls::SelectorCell, selectorTheme, "setting/ui/theme");
    BRLS_BIND(brls::SelectorCell, selectorTexture, "setting/cache/texture");
    BRLS_BIND(brls::BooleanCell, btnHistory, "setting/history/log");
    BRLS_BIND(brls::BooleanCell, btnOpencc, "setting/opencc");
    BRLS_BIND(brls::BooleanCell, btnProgress, "setting/video/progress");
    BRLS_BIND(brls::BooleanCell, btnQuality, "setting/video/quality");
    BRLS_BIND(TextBox, labelOpensource, "setting/label/opensource");
    BRLS_BIND(brls::BooleanCell, cellHideBar, "cell/hideBottomBar");
};