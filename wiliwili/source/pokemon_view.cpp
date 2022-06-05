/*
    Copyright 2021 XITRIX

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "pokemon_view.hpp"

#include <borealis/core/i18n.hpp>

using namespace brls::literals;

bool dismissView(brls::View* view, PokemonView* pock)
{
    return true;
}

PokemonView::PokemonView(Pokemon pokemon)
    : pokemon(pokemon)
{
    // Inflate the tab from the XML file
    this->inflateFromXMLRes("xml/views/pokemon.xml");

    auto dismissAction = [this](View* view) {
        this->dismiss();
        return true;
    };

    brls::Label* label = new brls::Label();
    label->setText(brls::Hint::getKeyIcon(brls::ControllerButton::BUTTON_RB) + " Закрыть");
    label->setFontSize(24);
    label->setMargins(0, 12, 0, 12);

    brls::Box* holder = new brls::Box();
    holder->addView(label);
    holder->setFocusable(true);
    holder->addGestureRecognizer(new brls::TapGestureRecognizer(holder));

    holder->registerClickAction(dismissAction);
    holder->registerAction("Close", brls::ControllerButton::BUTTON_RB, dismissAction, true);
    registerAction("Close", brls::ControllerButton::BUTTON_RB, dismissAction, true);

    getAppletFrameItem()->title = pokemon.name;
    getAppletFrameItem()->setIconFromRes("img/pokemon/" + pokemon.id + ".png");
    getAppletFrameItem()->setHintView(holder);
    image->setImageFromRes("img/pokemon/" + pokemon.id + ".png");

    description->setText("It's a pokemon with name: " + pokemon.name + "\nCollect them all to became a Shaman king!");

    this->getView("close_button")->registerAction(
        "hints/ok"_i18n, brls::BUTTON_A, [this](brls::View* view) {
            this->dismiss();
            return true;
        },
        false, false, brls::SOUND_BACK);

    brls::Logger::error("create pokemon");
}

brls::View* PokemonView::create()
{
    // Called by the XML engine to create a new ComponentsTab
    return new PokemonView();
}
