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

#include <borealis.hpp>

class CaptionedImage : public brls::Box
{
  public:
    CaptionedImage();

    void onChildFocusGained(brls::View* directChild, brls::View* focusedView) override;
    void onChildFocusLost(brls::View* directChild, brls::View* focusedView) override;

    static brls::View* create();

  private:
    BRLS_BIND(brls::Image, image, "image");
    BRLS_BIND(brls::Label, label, "label");
};
