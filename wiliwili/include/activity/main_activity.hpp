/*
    Copyright 2020-2021 natinusala

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

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>

class CustomButton;
class AutoTabFrame;

class MainActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/main.xml");

    void onContentAvailable() override;

    ~MainActivity();

    static void openSetting();

private:
    BRLS_BIND(CustomButton, settingBtn, "main/setting");
    BRLS_BIND(AutoTabFrame, tabFrame, "main/tabFrame");
};
