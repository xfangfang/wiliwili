//
// Created by fang on 2022/8/22.
//

#include "activity/setting_activity.hpp"
#include "activity/hint_activity.hpp"
#include "view/text_box.hpp"
#include "utils/config_helper.hpp"
#include "borealis/views/applet_frame.hpp"

const std::string OPENSOURCE = "--------------------------------\nFFmpeg\n--------------------------------\nOfficial site:    https://www.ffmpeg.org\nModified version: https://github.com/xfangfang/FFmpeg\n\nCopyright (c) FFmpeg developers and contributers\n\nLicensed under LGPLv2.1 or later\n\n\n--------------------------------\nmpv\n--------------------------------\nOfficial site:    https://mpv.io\nModified version: https://github.com/xfangfang/mpv\n\nCopyright (c) mpv developers and contributers\n\nLicensed under GPL-2.0 or LGPLv2.1\n\n\n--------------------------------\nborealis\n--------------------------------\nhttps://github.com/natinusala/borealis\nCopyright (c) 2019-2022, natinusala and contributers\n\nModifications for touch and recycler list support\nhttps://github.com/XITRIX/borealis\nCopyright (c) XITRIX \n\nModified version: https://github.com/xfangfang/borealis\n\nLicensed under Apache-2.0 license\n\n\n--------------------------------\npystring\n--------------------------------\nhttps://github.com/imageworks/pystring\n\nCopyright (c) imageworks and contributers\n\nLicensed under BCD-3-Clause license\n\n\n--------------------------------\nQR-Code-generator\n--------------------------------\nOfficial site: https://www.nayuki.io/page/qr-code-generator-library\nhttps://github.com/nayuki/QR-Code-generator\n\nCopyright © 2020 Project Nayuki.\n\nLicensed under MIT license\n\n\n--------------------------------\nlunasvg\n--------------------------------\nhttps://github.com/sammycage/lunasvg\n\nCopyright (c) 2020 Nwutobo Samuel Ugochukwu.\n\nLicensed under MIT license\n\n\n--------------------------------\nQR-Code-generator\n--------------------------------\nOfficial site: https://docs.libcpr.org\nhttps://github.com/libcpr/cpr\n\nCopyright (c) 2017-2021 Huu Nguyen\nCopyright (c) 2022 libcpr and many other contributors\n\nLicensed under MIT license\n\n\n--------------------------------\nnx\n--------------------------------\nhttps://github.com/switchbrew/libnx\n\nCopyright 2017-2018 libnx Authors\n\nPublic domain\n\n\n--------------------------------\ndevkitPro\n--------------------------------\nhttps://devkitpro.org\n\nCopyright devkitPro Authors\n\nPublic domain\n";


SettingActivity::SettingActivity() {
    brls::Logger::debug("SettingActivity: create");
}

void SettingActivity::onContentAvailable() {
    brls::Logger::debug("SettingActivity: onContentAvailable");

    btnTutorialOpenApp->registerClickAction([](...) -> bool{
        brls::Application::pushActivity(new HintActivity());
        return true;
    });

    labelOpensource->setText(OPENSOURCE);

    auto& conf = ProgramConfig::instance();

    cellHideBar->init("隐藏下方提示栏", conf.getSettingItem(SettingItem::HIDE_BOTTOM_BAR, false),
                      [this](bool value){
                          ProgramConfig::instance().setSettingItem(SettingItem::HIDE_BOTTOM_BAR, value);
                          // 更新设置
                          brls::AppletFrame::HIDE_BOTTOM_BAR = value;
                          // 设置当前底栏
                          ((brls::AppletFrame*)this->getContentView())->setFooterVisibility(value ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
                          // 设置MainActivity底栏
                          ((brls::AppletFrame*)brls::Application::getActivitiesStack()[0]->getContentView())->setFooterVisibility(value ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
    });
}

SettingActivity::~SettingActivity() {
    brls::Logger::debug("SettingActivity: delete");
}