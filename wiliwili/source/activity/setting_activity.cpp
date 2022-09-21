//
// Created by fang on 2022/8/22.
//

#include "activity/setting_activity.hpp"
#include "activity/hint_activity.hpp"
#include "activity/player_activity.hpp"
#include "fragment/setting_network.hpp"
#include "view/text_box.hpp"
#include "utils/config_helper.hpp"
#include "utils/cache_helper.hpp"
#include "borealis/views/applet_frame.hpp"

using namespace brls::literals;

const std::string OPENSOURCE =
    "--------------------------------\n"
    "FFmpeg\n"
    "--------------------------------\n"
    "Official site:    https://www.ffmpeg.org\n"
    "Modified version: https://github.com/xfangfang/FFmpeg\n\n"
    "Copyright (c) FFmpeg developers and contributors\n\n"
    "Licensed under LGPLv2.1 or later\n\n\n"
    "--------------------------------\n"
    "mpv\n"
    "--------------------------------\n"
    "Official site:    https://mpv.io\n"
    "Modified version: https://github.com/xfangfang/mpv\n\n"
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
    "QR-Code-generator\n"
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
        brls::Application::pushActivity(new PlayerActivity("BV18W4y1q72C"));
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

    btnNetworkChecker->registerClickAction([](...) -> bool {
        auto dialog = new brls::Dialog((brls::Box*)new SettingNetwork());
        dialog->addButton("hints/ok"_i18n, []() {});
        dialog->open();
        return true;
    });

    btnReleaseChecker->title->setText(
        fmt::format("检查更新 (当前版本: {})", "version/version"_i18n));
    btnReleaseChecker->registerClickAction([](...) -> bool {
        brls::Application::getPlatform()->openBrowser(
            "https://github.com/xfangfang/wiliwili/releases/latest");
        return true;
    });

    labelOpensource->setText(OPENSOURCE);

    auto& conf = ProgramConfig::instance();

    cellHideBar->init(
        "隐藏下方提示栏",
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

    int themeData = conf.getSettingItem(SettingItem::APP_THEME, 0);
    selectorTheme->init(
        "主题配色 (需要重启)", {"跟随系统", "浅色", "深色"}, themeData,
        [themeData](int data) {
            if (themeData == data) return false;
            auto dialog =
                new brls::Dialog("即将退出应用, 设置内容将在下次启动后生效");
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

    selectorTexture->init(
        "图片纹理缓存数量", {"100", "200(默认)", "300", "400", "500"},
        conf.getSettingItem(SettingItem::TEXTURE_CACHE_NUM, 200) / 100 - 1,
        [](int data) {
            int num = 100 * data + 100;
            ProgramConfig::instance().setSettingItem(
                SettingItem::TEXTURE_CACHE_NUM, num);
            TextureCache::instance().cache.setCapacity(num);
        });

    btnHistory->init("上传历史播放记录",
                     conf.getSettingItem(SettingItem::HISTORY_REPORT, true),
                     [](bool value) {
                         ProgramConfig::instance().setSettingItem(
                             SettingItem::HISTORY_REPORT, value);
                         VideoDetail::REPORT_HISTORY = value;
                     });

    btnProgress->init("播放器下方固定显示进度条",
                      conf.getSettingItem(SettingItem::PLAYER_BOTTOM_BAR, true),
                      [](bool value) {
                          ProgramConfig::instance().setSettingItem(
                              SettingItem::PLAYER_BOTTOM_BAR, value);
                          MPVCore::BOTTOM_BAR = value;
                      });
}

SettingActivity::~SettingActivity() {
    brls::Logger::debug("SettingActivity: delete");
}