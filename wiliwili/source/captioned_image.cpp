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

#include "captioned_image.hpp"

CaptionedImage::CaptionedImage()
{
    // Load the XML file and inflate ourself with its content
    // The top-level Box in the XML corresponds to us, and every XML child
    // is added to our children (and the attributes are applied)
    // The CaptionedImage instance basically becomes what's written in the XML
    this->inflateFromXMLRes("xml/views/captioned_image.xml");

    // The label stays hidden until focused, so hide it right away
    this->label->hide([] {});

    // Forward Image and Label XML attributes
    this->forwardXMLAttribute("scalingType", this->image);
    this->forwardXMLAttribute("image", this->image);
    this->forwardXMLAttribute("focusUp", this->image);
    this->forwardXMLAttribute("focusRight", this->image);
    this->forwardXMLAttribute("focusDown", this->image);
    this->forwardXMLAttribute("focusLeft", this->image);
    this->forwardXMLAttribute("imageWidth", this->image, "width");
    this->forwardXMLAttribute("imageHeight", this->image, "height");

    this->forwardXMLAttribute("caption", this->label, "text");

    this->addGestureRecognizer(new brls::TapGestureRecognizer(this, brls::TapGestureConfig(false, brls::SOUND_NONE, brls::SOUND_NONE, brls::SOUND_NONE)));
}

void CaptionedImage::onChildFocusGained(brls::View* directChild, brls::View* focusedView)
{
    // Called when a child of ours gets focused, in that case it's the Image

    Box::onChildFocusGained(directChild, focusedView);

    this->label->show([] {});
}

void CaptionedImage::onChildFocusLost(brls::View* directChild, brls::View* focusedView)
{
    // Called when a child of ours losts focused, in that case it's the Image

    Box::onChildFocusLost(directChild, focusedView);

    this->label->hide([] {});
}

brls::View* CaptionedImage::create()
{
    // Called by the XML engine to create a new CaptionedImage
    return new CaptionedImage();
}
