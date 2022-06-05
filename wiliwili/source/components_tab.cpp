/*
    Copyright 2021 natinusala

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

#include "components_tab.hpp"

#include "pokemon_view.hpp"

ComponentsTab::ComponentsTab()
{
    // Inflate the tab from the XML file
    this->inflateFromXMLRes("xml/tabs/components.xml");

    // Bind the button click to a method using the macro (just for the sake of showcasing it, it's overkill in this situation)
    BRLS_REGISTER_CLICK_BY_ID("button_primary", this->onPrimaryButtonClicked);

    // Get a handle to the button and register the action directly
    brls::Button* highlightButton = (brls::Button*)this->getView("button_highlight");
    highlightButton->registerAction(
        "Honk", brls::BUTTON_A, [](brls::View* view) { return true; }, false, false, brls::SOUND_HONK);

    progress->setText(std::to_string((int)(slider->getProgress() * 100)));
    slider->getProgressEvent()->subscribe([this](float progress) {
        this->progress->setText(std::to_string((int)(progress * 100)));
    });
}

int selected = 0;
bool ComponentsTab::onPrimaryButtonClicked(brls::View* view)
{
    //    brls::AppletFrame* frame = new brls::AppletFrame(PokemonView::create());
    //    frame->setFooterVisibility(brls::Visibility::GONE);
    //    brls::Application::pushActivity(new brls::Activity(frame));

    brls::Dropdown* dropdown = new brls::Dropdown(
        "Test", std::vector<std::string> { "Test 1", "Test 2", "Test 3", "Test 4", "Test 5", "Test 6", "Test 7", "Test 8", "Test 9", "Test 10", "Test 11", "Test 12", "Test 13" }, [](int _selected) {
            selected = _selected;
        },
        selected);
    //    brls::Dropdown* dropdown = new brls::Dropdown(
    //        "Test", std::vector<std::string> { "Test 1", "Test 2", "Test 3" }, [](int _selected) {
    //            selected = _selected;
    //        },
    //        selected);
    brls::Application::pushActivity(new brls::Activity(dropdown));
    brls::Logger::info("Clicked");
    return true;
}

brls::View* ComponentsTab::create()
{
    // Called by the XML engine to create a new ComponentsTab
    return new ComponentsTab();
}
