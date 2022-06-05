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

#include "settings_tab.hpp"

bool radioSelected = false;

SettingsTab::SettingsTab()
{
    // Inflate the tab from the XML file
    this->inflateFromXMLRes("xml/tabs/settings.xml");

    radio->title->setText("Radio cell");
    radio->setSelected(radioSelected);
    radio->registerClickAction([this](brls::View* view) {
        radioSelected = !radioSelected;
        this->radio->setSelected(radioSelected);
        return true;
    });

    boolean->title->setText("Switcher");

    selector->init("Selector", { "Test 1", "Test 2", "Test 3", "Test 4", "Test 5", "Test 6", "Test 7", "Test 8", "Test 9", "Test 10", "Test 11", "Test 12", "Test 13" }, 0, [](int selected) {

    });

    input->init(
        "Input text", "", [](std::string text) {

        },
        "Placeholder", "Hint");

    inputNumeric->init(
        "Input number", 2448, [](int number) {

        },
        "Hint");

    brls::Logger::error("create setting");
}

brls::View* SettingsTab::create()
{
    return new SettingsTab();
}

SettingsTab::~SettingsTab() {
    brls::Logger::error("del setting");
}
