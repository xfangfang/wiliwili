//
// Created by fang on 2023/3/4.
//

#include "fragment/player_setting.hpp"
#include "view/danmaku_core.hpp"
#include "view/button_close.hpp"
#include "view/mpv_core.hpp"
#include "view/subtitle_core.hpp"
#include "view/video_view.hpp"
#include "utils/config_helper.hpp"
#include "utils/shader_helper.hpp"
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
        shaderHeader->setVisibility(brls::Visibility::GONE);
        shaderBox->setVisibility(brls::Visibility::GONE);
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