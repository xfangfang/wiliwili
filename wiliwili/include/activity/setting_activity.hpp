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
    BRLS_BIND(TextBox, labelOpensource, "setting/label/opensource");
    BRLS_BIND(brls::BooleanCell, cellHideBar, "cell/hideBottomBar");
};