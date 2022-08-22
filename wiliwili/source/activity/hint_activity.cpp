//
// Created by fang on 2022/8/21.
//

#include "activity/hint_activity.hpp"
#include "view/gallery_view.hpp"

using namespace brls::literals;

HintActivity::HintActivity() {
    brls::Logger::debug("HintActivityActivity: create");
}

void HintActivity::onContentAvailable() {
    brls::Logger::debug("HintActivityActivity: onContentAvailable");

    gallery->setData({
        {"pictures/hint_game_1.png",    "wiliwili/hint1"_i18n},
        {"pictures/hint_game_2.png",    "wiliwili/hint2"_i18n},
        {"pictures/hint_hbmenu.png",    "wiliwili/hint3"_i18n},
        {"pictures/hint_wiliwili.png",  "wiliwili/hint4"_i18n},
    });
}

HintActivity::~HintActivity() {
    brls::Logger::debug("HintActivityActivity: delete");
}