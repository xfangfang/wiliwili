//
// Created by fang on 2023/3/4.
//

#include "fragment/player_setting.hpp"
#include "view/danmaku_core.hpp"
#include "view/button_close.hpp"
#include "view/mpv_core.hpp"
#include "view/subtitle_core.hpp"
#include "view/video_view.hpp"
#include "view/selector_cell.hpp"
#include "utils/config_helper.hpp"
#include "utils/shader_helper.hpp"
#include "utils/number_helper.hpp"
#include "activity/player_activity.hpp"

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

    closebtn->registerClickAction([](...) {
        brls::Application::popActivity();
        return true;
    });
}

PlayerSetting::~PlayerSetting() {
    brls::Logger::debug("Fragment PlayerSetting: delete");
}

brls::View* PlayerSetting::create() { return new PlayerSetting(); }

bool PlayerSetting::isTranslucent() { return true; }

brls::View* PlayerSetting::getDefaultFocus() {
    return this->settings->getDefaultFocus();
}

void PlayerSetting::setupCustomShaders() {
    if (!ShaderHelper::instance().isAvailable()) {
        auto* cell = new brls::RadioCell();
        cell->title->setText("wiliwili/player/setting/common/wiki"_i18n);
        cell->registerClickAction([](...) {
            brls::Application::getPlatform()->openBrowser(
                "https://github.com/xfangfang/wiliwili/wiki");
            return true;
        });
        shaderBox->addView(cell);
        auto* hint = new brls::Label();
        hint->setText(
            "https://github.com/xfangfang/wiliwili/wiki#自定义着色器");
        hint->setFontSize(12);
        hint->setMargins(10, 0, 0, 10);
        hint->setTextColor(brls::Application::getTheme().getColor("font/grey"));
        shaderBox->addView(hint);
        return;
    }

    auto pack = ShaderHelper::instance().getShaderPack();
    for (auto& p : pack.profiles) {
        brls::Logger::debug("name: {}", p.name);

        auto* cell = new brls::RadioCell();
        cell->title->setText(p.name);
        if (ShaderHelper::currentShader == p.name) cell->setSelected(true);

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
            return true;
        });

        shaderBox->addView(cell);
    }
}

void PlayerSetting::setupCommonSetting() {
    auto& conf = ProgramConfig::instance();

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

    btnExitFullscreen->init(
        "wiliwili/setting/app/playback/exit_fullscreen"_i18n,
        conf.getBoolOption(SettingItem::PLAYER_EXIT_FULLSCREEN_ON_END),
        [](bool value) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::PLAYER_EXIT_FULLSCREEN_ON_END, value);
            VideoView::EXIT_FULLSCREEN_ON_END = value;
        });

    /// Player bottom bar
    btnProgress->init("wiliwili/setting/app/playback/player_bar"_i18n,
                      conf.getBoolOption(SettingItem::PLAYER_BOTTOM_BAR),
                      [](bool value) {
                          ProgramConfig::instance().setSettingItem(
                              SettingItem::PLAYER_BOTTOM_BAR, value);
                          MPVCore::BOTTOM_BAR = value;
                      });

    /// Auto Sleep
    std::string min                     = "wiliwili/home/common/min"_i18n;
    std::vector<std::string> optionList = {"hints/off"_i18n, "15 " + min,
                                           "30 " + min,      "60 " + min,
                                           "90 " + min,      "120 " + min};
    btnSleep->init("wiliwili/setting/app/playback/sleep"_i18n, optionList, 0,
                   [](int data) {
                       std::vector<int> time = {-1, 15, 30, 60, 90, 120};
                       if (data == 0)
                           MPVCore::CLOSE_TIME = 0;
                       else
                           MPVCore::CLOSE_TIME =
                               wiliwili::getUnixTime() + time[data] * 60;
                       return true;
                   });

/// Fullscreen
#if defined(__linux__) || defined(_WIN32)
    btnFullscreen->init(
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
    btnFullscreen->setVisibility(brls::Visibility::GONE);
#endif
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
        if (SubtitleCore::instance().getCurrentSubtitleId() == s.id_str)
            cell->setSelected(true);
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
            return true;
        });

        subtitleBox->addView(cell);
    }
}

void PlayerSetting::draw(NVGcontext* vg, float x, float y, float width,
                         float height, brls::Style style,
                         brls::FrameContext* ctx) {
    static size_t updateTime = 0;
    size_t now               = wiliwili::getUnixTime();
    if (now != updateTime) {
        updateTime = now;
        if (MPVCore::CLOSE_TIME == 0 || now > MPVCore::CLOSE_TIME) {
            btnSleep->detail->setText("hints/off"_i18n);
        } else {
            btnSleep->detail->setText(
                wiliwili::sec2Time(MPVCore::CLOSE_TIME - now));
        }
    }
    Box::draw(vg, x, y, width, height, style, ctx);
}
