//
// Created by fang on 2023/1/10.
//

#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/views/cells/cell_bool.hpp>

#include "fragment/player_danmaku_setting.hpp"
#include "view/danmaku_core.hpp"
#include "view/button_close.hpp"
#include "view/selector_cell.hpp"
#include "utils/config_helper.hpp"
#include "utils/string_helper.hpp"

using namespace brls::literals;

PlayerDanmakuSetting::PlayerDanmakuSetting() {
    this->inflateFromXMLRes("xml/fragment/player_danmaku_setting.xml");
    brls::Logger::debug("Fragment PlayerDanmakuSetting: create");

    this->registerAction("hints/cancel"_i18n, brls::BUTTON_B, [](...) {
        brls::Application::popActivity();
        return true;
    });

    closebtn->registerClickAction([](...) {
        brls::Application::popActivity();
        return true;
    });

    this->cancel->registerClickAction([](...) {
        brls::Application::popActivity();
        return true;
    });
    this->cancel->addGestureRecognizer(new brls::TapGestureRecognizer(this->cancel));

    auto& conf = ProgramConfig::instance();

#if defined(BOREALIS_USE_D3D11) || defined(BOREALIS_USE_OPENGL) && !defined(__PSV__)
    this->cellMask->init("wiliwili/player/danmaku/filter/mask"_i18n, DanmakuCore::DANMAKU_SMART_MASK, [](bool data) {
        DanmakuCore::DANMAKU_SMART_MASK = data;
        DanmakuCore::save();
        return true;
    });
#else
    this->cellMask->setVisibility(brls::Visibility::GONE);
#endif
    this->cellTop->init("wiliwili/player/danmaku/filter/top"_i18n, DanmakuCore::DANMAKU_FILTER_SHOW_TOP, [](bool data) {
        DanmakuCore::DANMAKU_FILTER_SHOW_TOP = data;
        DanmakuCore::save();
        DanmakuCore::instance().refresh();
        return true;
    });
    this->cellBottom->init("wiliwili/player/danmaku/filter/bottom"_i18n, DanmakuCore::DANMAKU_FILTER_SHOW_BOTTOM,
                           [](bool data) {
                               DanmakuCore::DANMAKU_FILTER_SHOW_BOTTOM = data;
                               DanmakuCore::save();
                               DanmakuCore::instance().refresh();
                               return true;
                           });
    this->cellScroll->init("wiliwili/player/danmaku/filter/scroll"_i18n, DanmakuCore::DANMAKU_FILTER_SHOW_SCROLL,
                           [](bool data) {
                               DanmakuCore::DANMAKU_FILTER_SHOW_SCROLL = data;
                               DanmakuCore::save();
                               DanmakuCore::instance().refresh();
                               return true;
                           });
    this->cellColor->init("wiliwili/player/danmaku/filter/color"_i18n, DanmakuCore::DANMAKU_FILTER_SHOW_COLOR,
                          [](bool data) {
                              DanmakuCore::DANMAKU_FILTER_SHOW_COLOR = data;
                              DanmakuCore::save();
                              DanmakuCore::instance().refresh();
                              return true;
                          });
    this->cellAdvanced->init("wiliwili/player/danmaku/filter/advanced"_i18n, DanmakuCore::DANMAKU_FILTER_SHOW_ADVANCED,
                             [](bool data) {
                                 DanmakuCore::DANMAKU_FILTER_SHOW_ADVANCED = data;
                                 DanmakuCore::save();
                                 DanmakuCore::instance().refresh();
                                 return true;
                             });

    std::vector<std::string> levels;
    for (size_t i = 1; i <= 10; i++)
        levels.emplace_back(wiliwili::format("wiliwili/player/danmaku/filter/level_n"_i18n, i));
    this->cellLevel->init("wiliwili/player/danmaku/filter/level"_i18n, levels,
                          (int)conf.getIntOptionIndex(SettingItem::DANMAKU_FILTER_LEVEL), [](int data) {
                              DanmakuCore::DANMAKU_FILTER_LEVEL = data + 1;
                              DanmakuCore::save();
                              DanmakuCore::instance().refresh();
                              return true;
                          });

    this->cellArea->init("wiliwili/player/danmaku/style/area"_i18n,
                         {"wiliwili/player/danmaku/style/area_1_4"_i18n, "wiliwili/player/danmaku/style/area_2_4"_i18n,
                          "wiliwili/player/danmaku/style/area_3_4"_i18n, "wiliwili/player/danmaku/style/area_4_4"_i18n},
                         (int)conf.getIntOptionIndex(SettingItem::DANMAKU_STYLE_AREA), [](int data) {
                             DanmakuCore::DANMAKU_STYLE_AREA = 25 + data * 25;
                             DanmakuCore::save();
                             DanmakuCore::instance().refresh();
                             return true;
                         });

    auto alpha = ProgramConfig::instance().getOptionData(SettingItem::DANMAKU_STYLE_ALPHA);
    this->cellAlpha->init("wiliwili/player/danmaku/style/alpha"_i18n, alpha.optionList,
                          (int)conf.getIntOptionIndex(SettingItem::DANMAKU_STYLE_ALPHA), [alpha](int data) {
                              DanmakuCore::DANMAKU_STYLE_ALPHA = alpha.rawOptionList[data];
                              DanmakuCore::save();
                              DanmakuCore::instance().refresh();
                              return true;
                          });

    auto font = ProgramConfig::instance().getOptionData(SettingItem::DANMAKU_STYLE_FONTSIZE);
    this->cellFontsize->init("wiliwili/player/danmaku/style/fontsize"_i18n, font.optionList,
                             (int)conf.getIntOptionIndex(SettingItem::DANMAKU_STYLE_FONTSIZE), [font](int data) {
                                 DanmakuCore::DANMAKU_STYLE_FONTSIZE = font.rawOptionList[data];
                                 DanmakuCore::save();
                                 DanmakuCore::instance().refresh();
                                 return true;
                             });

    auto height = ProgramConfig::instance().getOptionData(SettingItem::DANMAKU_STYLE_LINE_HEIGHT);
    this->cellLineHeight->init("wiliwili/player/danmaku/style/line_height"_i18n, height.optionList,
                               (int)conf.getIntOptionIndex(SettingItem::DANMAKU_STYLE_LINE_HEIGHT), [height](int data) {
                                   DanmakuCore::DANMAKU_STYLE_LINE_HEIGHT = height.rawOptionList[data];
                                   DanmakuCore::save();
                                   DanmakuCore::instance().refresh();
                                   return true;
                               });

    auto speed = ProgramConfig::instance().getOptionData(SettingItem::DANMAKU_STYLE_SPEED);
    this->cellSpeed->init("wiliwili/player/danmaku/style/speed"_i18n,
                          {
                              "wiliwili/player/danmaku/style/speed_slow_plus"_i18n,
                              "wiliwili/player/danmaku/style/speed_slow"_i18n,
                              "wiliwili/player/danmaku/style/speed_moderate"_i18n,
                              "wiliwili/player/danmaku/style/speed_fast"_i18n,
                              "wiliwili/player/danmaku/style/speed_fast_plus"_i18n,
                          },
                          (int)conf.getIntOptionIndex(SettingItem::DANMAKU_STYLE_SPEED), [speed](int data) {
                              DanmakuCore::DANMAKU_STYLE_SPEED = speed.rawOptionList[data];
                              DanmakuCore::save();
                              DanmakuCore::instance().refresh();
                              return true;
                          });

    this->cellBackground->init(
        "wiliwili/player/danmaku/style/font"_i18n,
        {"wiliwili/player/danmaku/style/font_stroke"_i18n, "wiliwili/player/danmaku/style/font_incline"_i18n,
         "wiliwili/player/danmaku/style/font_shadow"_i18n, "wiliwili/player/danmaku/style/font_pure"_i18n},
        conf.getStringOptionIndex(SettingItem::DANMAKU_STYLE_FONT), [](int data) {
            auto& conf                      = ProgramConfig::instance();
            DanmakuCore::DANMAKU_STYLE_FONT = DanmakuFontStyle{data};
            conf.setSettingItem(SettingItem::DANMAKU_STYLE_FONT,
                                conf.getOptionData(SettingItem::DANMAKU_STYLE_FONT).optionList[data]);
        });

    auto perf = conf.getOptionData(SettingItem::DANMAKU_RENDER_QUALITY);
    this->cellRenderPerf->init("wiliwili/player/danmaku/performance/render"_i18n, perf.optionList,
                               (int)conf.getIntOptionIndex(SettingItem::DANMAKU_RENDER_QUALITY), [perf](int data) {
                                   DanmakuCore::DANMAKU_RENDER_QUALITY = perf.rawOptionList[data];
                                   DanmakuCore::save();
                               });
}

PlayerDanmakuSetting::~PlayerDanmakuSetting() { brls::Logger::debug("Fragment PlayerDanmakuSetting: delete"); }

brls::View* PlayerDanmakuSetting::create() { return new PlayerDanmakuSetting(); }

bool PlayerDanmakuSetting::isTranslucent() { return true; }

brls::View* PlayerDanmakuSetting::getDefaultFocus() { return this->settings->getDefaultFocus(); }
