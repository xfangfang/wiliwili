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
#include "view/svg_image.hpp"
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

    this->cancel->registerClickAction([](...) {
        brls::Application::popActivity();
        return true;
    });
    this->cancel->addGestureRecognizer(
        new brls::TapGestureRecognizer(this->cancel));

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
    // TODO Fix: shaders cannot work with deko3d and ps4
#if !defined(_DEBUG) && (defined(BOREALIS_USE_DEKO3D) || defined(PS4))
    // hide shader setting: deko3d and ps4
    auto* cell = new brls::RadioCell();
    cell->title->setText("wiliwili/dialog/not_supported"_i18n);
    shaderBox->addView(cell);
    return;
#else
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
#endif

    auto pack = ShaderHelper::instance().getShaderPack();
    for (auto& p : pack.profiles) {
        brls::Logger::debug("name: {}", p.name);

        auto* cell = new brls::RadioCell();
        cell->title->setText(p.name);
        if (MPVCore::instance().currentShaderProfile == p.name)
            cell->setSelected(true);

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
    auto& conf  = ProgramConfig::instance();
    auto locale = brls::Application::getLocale();

    /// Upload history record
    btnHistory->init("wiliwili/setting/app/playback/report"_i18n,
                     conf.getSettingItem(SettingItem::HISTORY_REPORT, true),
                     [](bool value) {
                         ProgramConfig::instance().setSettingItem(
                             SettingItem::HISTORY_REPORT, value);
                         VideoDetail::REPORT_HISTORY = value;
                     });

    /// player strategy
    int strategyIndex = conf.getIntOption(SettingItem::PLAYER_STRATEGY);
    // 中文比较简洁可以显示出来，其他语言翻译过长，在这里就先不展示了
    bool showStrategy =
        locale == brls::LOCALE_ZH_HANT || locale == brls::LOCALE_ZH_HANS;
    btnPlayStrategy->setText(
        "wiliwili/setting/app/playback/play_strategy"_i18n);
    std::vector<std::string> optionList = {
        "wiliwili/setting/app/playback/auto_play_recommend"_i18n,
        "wiliwili/setting/app/playback/auto_play_next_part"_i18n,
        "wiliwili/setting/app/playback/auto_loop"_i18n,
        "wiliwili/setting/app/playback/auto_no"_i18n};
    btnPlayStrategy->setDetailText(showStrategy ? optionList[strategyIndex]
                                                : " ");
    btnPlayStrategy->registerClickAction([this, optionList,
                                          showStrategy](View* view) {
        auto d = BaseDropdown::text(
            "wiliwili/setting/app/playback/play_strategy"_i18n, optionList,
            [this, optionList, showStrategy](int data) {
                if (showStrategy)
                    btnPlayStrategy->setDetailText(optionList[data]);
                BasePlayerActivity::PLAYER_STRATEGY = data;
                ProgramConfig::instance().setSettingItem(
                    SettingItem::PLAYER_STRATEGY, data);
            },
            ProgramConfig::instance().getIntOption(
                SettingItem::PLAYER_STRATEGY));

        // bottom hint
        auto box = new brls::Box();
        box->setMargins(20, 10, 10, 30);
        box->setAlignItems(brls::AlignItems::CENTER);
        auto icon = new SVGImage();
        icon->setDimensions(12, 13);
        icon->setMarginRight(10);
        icon->setImageFromSVGRes("svg/ico-sprite-info.svg");
        auto hint = new brls::Label();
        hint->setFontSize(18);
        hint->setTextColor(brls::Application::getTheme().getColor("font/grey"));
        hint->setText("wiliwili/setting/app/playback/auto_hint"_i18n);
        box->addView(icon);
        box->addView(hint);
        d->getContentView()->addView(box);
        return true;
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
                          VideoView::BOTTOM_BAR = value;
                      });

    /// Player mirror
    btnMirror->init("wiliwili/player/setting/common/mirror"_i18n,
                    MPVCore::VIDEO_MIRROR, [](bool value) {
                        MPVCore::VIDEO_MIRROR = !MPVCore::VIDEO_MIRROR;
                        MPVCore::instance().command_async(
                            "set", "vf", MPVCore::VIDEO_MIRROR ? "hflip" : "");
                    });

    /// Player aspect
    btnAspect->init("wiliwili/player/setting/aspect/header"_i18n,
                    {"wiliwili/player/setting/aspect/auto"_i18n, "4:3", "16:9"},
                    conf.getStringOptionIndex(SettingItem::PLAYER_ASPECT),
                    [](int value) {
                        auto option = ProgramConfig::instance().getOptionData(
                            SettingItem::PLAYER_ASPECT);
                        auto& aspect = option.optionList[value];
                        MPVCore::instance().setAspect(aspect);
                        ProgramConfig::instance().setSettingItem(
                            SettingItem::PLAYER_ASPECT, aspect);
                    });

    /// Player Highlight progress bar
    btnHighlight->init("wiliwili/player/setting/common/highlight"_i18n,
                       VideoView::HIGHLIGHT_PROGRESS_BAR, [](bool value) {
                           ProgramConfig::instance().setSettingItem(
                               SettingItem::PLAYER_HIGHLIGHT_BAR, value);
                           VideoView::HIGHLIGHT_PROGRESS_BAR = value;
                       });

    /// Auto Sleep
    btnSleep->setText("wiliwili/setting/app/playback/sleep"_i18n);
    btnSleep->setDetailText(getCountdown(wiliwili::getUnixTime()));
    btnSleep->registerClickAction([this](View* view) {
        std::vector<int> timeList           = {15, 30, 60, 90, 120};
        std::string min                     = "wiliwili/home/common/min"_i18n;
        std::vector<std::string> optionList = {
            "15 " + min, "30 " + min, "60 " + min, "90 " + min, "120 " + min};
        bool countdownStarted = MPVCore::CLOSE_TIME != 0 &&
                                wiliwili::getUnixTime() < MPVCore::CLOSE_TIME;
        if (countdownStarted) {
            // 添加关闭选项
            timeList.insert(timeList.begin(), -1);
            optionList.insert(optionList.begin(), "hints/off"_i18n);
        }
        BaseDropdown::text(
            "wiliwili/setting/app/playback/sleep"_i18n, optionList,
            [this, timeList, countdownStarted](int data) {
                if (countdownStarted && data == 0) {
                    MPVCore::CLOSE_TIME = 0;
                } else {
                    MPVCore::CLOSE_TIME =
                        wiliwili::getUnixTime() + timeList[data] * 60;
                }
                btnSleep->setDetailText(getCountdown(wiliwili::getUnixTime()));
            },
            -1);
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
        btnSleep->detail->setText(getCountdown(now));
    }
    Box::draw(vg, x, y, width, height, style, ctx);
}

std::string PlayerSetting::getCountdown(size_t now) {
    if (MPVCore::CLOSE_TIME == 0 || now > MPVCore::CLOSE_TIME) {
        return "hints/off"_i18n;
    } else {
        return wiliwili::sec2Time(MPVCore::CLOSE_TIME - now);
    }
}

void PlayerSetting::hideHistoryCell() {
    btnHistory->setVisibility(brls::Visibility::GONE);
}

void PlayerSetting::hideVideoRelatedCells() {
    btnPlayStrategy->setVisibility(brls::Visibility::GONE);
    btnExitFullscreen->setVisibility(brls::Visibility::GONE);
}

void PlayerSetting::hideSubtitleCells() {
    subtitleBox->setVisibility(brls::Visibility::GONE);
    subtitleHeader->setVisibility(brls::Visibility::GONE);
}

void PlayerSetting::hideBottomLineCells() {
    btnProgress->setVisibility(brls::Visibility::GONE);
}

void PlayerSetting::hideHighlightLineCells() {
    btnHighlight->setVisibility(brls::Visibility::GONE);
}