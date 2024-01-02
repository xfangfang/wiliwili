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

            GA("player_setting", {{"shader", cell->title->getFullText()}});
            return true;
        });

        shaderBox->addView(cell);
    }
}

void PlayerSetting::setupCommonSetting() {
    auto& conf  = ProgramConfig::instance();
    auto locale = brls::Application::getLocale();

    /// Upload history record
    btnHistory->init(
        "wiliwili/setting/app/playback/report"_i18n,
        conf.getSettingItem(SettingItem::HISTORY_REPORT, true), [](bool value) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::HISTORY_REPORT, value);
            VideoDetail::REPORT_HISTORY = value;
            GA("player_setting", {{"history", value ? "true" : "false"}});
        });

    /// Skip opening credits
    btnSkip->init(
        "wiliwili/player/setting/common/skip_opening_credits"_i18n,
        conf.getBoolOption(SettingItem::PLAYER_SKIP_OPENING_CREDITS),
        [](bool value) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::PLAYER_SKIP_OPENING_CREDITS, value);
            BasePlayerActivity::PLAYER_SKIP_OPENING_CREDITS = value;
            std::string hint =
                value ? "wiliwili/player/setting/common/skip_hint1"_i18n
                      : "wiliwili/player/setting/common/skip_hint2"_i18n;
            MPV_CE->fire(VideoView::HINT, (void*)hint.c_str());
            GA("player_setting", {{"skip", value ? "true" : "false"}});
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
                GA("player_setting", {{"strategy", optionList[data]}});
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
            GA("player_setting", {{"exit_fs", value ? "true" : "false"}});
        });

    /// Player bottom bar
    btnProgress->init(
        "wiliwili/setting/app/playback/player_bar"_i18n,
        conf.getBoolOption(SettingItem::PLAYER_BOTTOM_BAR), [](bool value) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::PLAYER_BOTTOM_BAR, value);
            VideoView::BOTTOM_BAR = value;
            GA("player_setting", {{"bottom_bar", value ? "true" : "false"}});
        });

    /// Player mirror
    btnMirror->init(
        "wiliwili/player/setting/common/mirror"_i18n, MPVCore::VIDEO_MIRROR,
        [](bool value) {
            MPVCore::VIDEO_MIRROR = !MPVCore::VIDEO_MIRROR;
            MPVCore::instance().command_async(
                "set", "vf", MPVCore::VIDEO_MIRROR ? "hflip" : "");
            GA("player_setting", {{"mirror", value ? "true" : "false"}});
        });

    /// Player aspect
    btnAspect->init("wiliwili/player/setting/aspect/header"_i18n,
                    {"wiliwili/player/setting/aspect/auto"_i18n, "4:3", "16:9"},
                    conf.getStringOptionIndex(SettingItem::PLAYER_ASPECT),
                    [this](int value) {
                        auto option = ProgramConfig::instance().getOptionData(
                            SettingItem::PLAYER_ASPECT);
                        auto& aspect = option.optionList[value];
                        auto seasonSetting =
                            ProgramConfig::instance().getSeasonCustom(
                                std::to_string(seasonId));
                        if (seasonSetting.player_aspect.empty()) {
                            // 只有在不是番剧，或者没有自定义番剧设置时，才会直接修改当前的视频比例
                            MPVCore::instance().setAspect(aspect);
                        }
                        ProgramConfig::instance().setSettingItem(
                            SettingItem::PLAYER_ASPECT, aspect);
                        GA("player_setting", {{"aspect", aspect}});
                    });

    /// Player Highlight progress bar
    btnHighlight->init(
        "wiliwili/player/setting/common/highlight"_i18n,
        VideoView::HIGHLIGHT_PROGRESS_BAR, [](bool value) {
            ProgramConfig::instance().setSettingItem(
                SettingItem::PLAYER_HIGHLIGHT_BAR, value);
            VideoView::HIGHLIGHT_PROGRESS_BAR = value;
            GA("player_setting", {{"highlight", value ? "true" : "false"}});
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
                    GA("player_setting", {{"sleep", "-1"}});
                } else {
                    MPVCore::CLOSE_TIME =
                        wiliwili::getUnixTime() + timeList[data] * 60;
                    GA("player_setting", {{"sleep", timeList[data]}});
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
            GA("player_setting", {{"fullscreen", value ? "true" : "false"}});
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

            GA("player_setting", {{value ? "subtitle-on" : "subtitle-off",
                                   cell->title->getFullText()}});
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

void PlayerSetting::hideSkipOpeningCreditsSetting() {
    btnSkip->setVisibility(brls::Visibility::GONE);
}

void PlayerSetting::setBangumiCustomSetting(const std::string& title,
                                            unsigned int id) {
    if (id == 0) return;
    seasonId = id;

    bangumiHeader->setTitle(title);
    bangumiHeader->setVisibility(brls::Visibility::VISIBLE);
    bangumiBox->setVisibility(brls::Visibility::VISIBLE);
    btnCustomAspect->setVisibility(brls::Visibility::VISIBLE);
    btnClip->setVisibility(brls::Visibility::VISIBLE);

    /// 番剧自定义数据
    auto seasonSetting = ProgramConfig::instance().getSeasonCustom(seasonId);

    std::unordered_map<std::string, int> aspectMap = {
        {"", 0}, {"-1", 1}, {"4:3", 2}, {"16:9", 3}};
    int aspect = 0;
    if (aspectMap.find(seasonSetting.player_aspect) != aspectMap.end()) {
        aspect = aspectMap[seasonSetting.player_aspect];
    }
    btnCustomAspect->init(
        "wiliwili/player/setting/aspect/header"_i18n,
        {"wiliwili/player/setting/aspect/no"_i18n,
         "wiliwili/player/setting/aspect/auto"_i18n, "4:3", "16:9"},
        aspect, [this, seasonSetting](int value) {
            std::vector<std::string> aspectOption = {"", "-1", "4:3", "16:9"};
            auto setting = ProgramConfig::instance().getSeasonCustom(seasonId);
            setting.player_aspect = aspectOption[value];
            ProgramConfig::instance().addSeasonCustomSetting(seasonId, setting);
            if (setting.player_aspect.empty()) {
                // 如果设置为空，则使用全局设置
                setting.player_aspect =
                    ProgramConfig::instance().getSettingItem(
                        SettingItem::PLAYER_ASPECT, std::string{"-1"});
            }
            MPVCore::instance().setAspect(setting.player_aspect);
            GA("season_custom_setting", {{"aspect", setting.player_aspect}});
        });
    if (seasonSetting.custom_clip) {
        btnClipStart->setVisibility(brls::Visibility::VISIBLE);
        btnClipEnd->setVisibility(brls::Visibility::VISIBLE);
    }
    btnClip->init(
        "wiliwili/player/setting/season/clip"_i18n, seasonSetting.custom_clip,
        [this](bool value) {
            auto setting = ProgramConfig::instance().getSeasonCustom(seasonId);
            setting.custom_clip = value;
            ProgramConfig::instance().addSeasonCustomSetting(seasonId, setting);
            std::string hint;
            if (value) {
                hint = "wiliwili/player/setting/common/skip_hint3"_i18n;
                btnClipStart->setVisibility(brls::Visibility::VISIBLE);
                btnClipEnd->setVisibility(brls::Visibility::VISIBLE);
            } else {
                hint = "wiliwili/player/setting/common/skip_hint4"_i18n;
                btnClipStart->setVisibility(brls::Visibility::GONE);
                btnClipEnd->setVisibility(brls::Visibility::GONE);
            }
            MPV_CE->fire(VideoView::HINT, (void*)hint.c_str());
            GA("season_custom_setting",
               {{"custom_clip", value ? "true" : "false"}});
        });
    btnClipStart->init(
        "wiliwili/player/setting/season/clip_start"_i18n,
        seasonSetting.clip_start,
        [this](int value) {
            auto setting = ProgramConfig::instance().getSeasonCustom(seasonId);
            setting.clip_start = value;
            ProgramConfig::instance().addSeasonCustomSetting(seasonId, setting);
        },
        "wiliwili/player/setting/season/unit"_i18n, 4);
    btnClipEnd->init(
        "wiliwili/player/setting/season/clip_end"_i18n, seasonSetting.clip_end,
        [this](int value) {
            auto setting = ProgramConfig::instance().getSeasonCustom(seasonId);
            setting.clip_end = value;
            ProgramConfig::instance().addSeasonCustomSetting(seasonId, setting);
        },
        "wiliwili/player/setting/season/unit"_i18n, 4);
}