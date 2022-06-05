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

#pragma once

#include <borealis.hpp>

class SettingsTab : public brls::Box
{
  public:
    SettingsTab();
    ~SettingsTab();

    BRLS_BIND(brls::RadioCell, radio, "radio");
    BRLS_BIND(brls::BooleanCell, boolean, "boolean");
    BRLS_BIND(brls::SelectorCell, selector, "selector");
    BRLS_BIND(brls::InputCell, input, "input");
    BRLS_BIND(brls::InputNumericCell, inputNumeric, "inputNumeric");

    static brls::View* create();
};
