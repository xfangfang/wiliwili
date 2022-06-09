/*
    Copyright 2022 xfangfang
    Copyright 2019-2021 natinusala
    Copyright 2019 WerWolv
    Copyright 2019 p-sam

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

#include "auto_tab_frame.hpp"

/**
 * auto tab frame
 */

const std::string autoTabFrameContentXML = R"xml(
    <brls:Box
        width="auto"
        height="auto"
        axis="row">

        <brls:Box
            wireframe="false"
            id="auto_tab_frame/auto_sidebar"
            width="100"
            height="auto" />

        <!-- Content will be injected here with grow="1.0" -->

    </brls:Box>
)xml";

View* AutoTabFrame::create() {
    return new AutoTabFrame();
}


AutoTabFrame::AutoTabFrame() {
    this->inflateFromXMLString(autoTabFrameContentXML);

    BRLS_REGISTER_ENUM_XML_ATTRIBUTE(
            "sidebarPosition", AutoTabBarPosition, this->setSideBarPosition,
            {
                { "top", AutoTabBarPosition::TOP },
                { "right", AutoTabBarPosition::RIGHT },
                { "left", AutoTabBarPosition::LEFT },
            });

    this->registerFloatXMLAttribute("tabFontSize", [this](float value){
       this->setFontSize(value);
    });

    // defaultTab: default is 0
    this->registerFloatXMLAttribute("defaultTab", [this](float value){
        this->setDefaultTabIndex(value);
    });

    this->sidebar->setBackground(ViewBackground::SIDEBAR);
    this->sidebar->setAxis(Axis::COLUMN);
    this->sidebar->setPadding(32,10,47,10);
}

void AutoTabFrame::setSideBarPosition(AutoTabBarPosition position){
    switch (position)
    {
        case AutoTabBarPosition::TOP:
            this->setAxis(Axis::COLUMN);
            this->setDirection(Direction::LEFT_TO_RIGHT);
            this->setHorizontalMode(true);
            this->sidebar->setWidthPercentage(100);
            break;
        case AutoTabBarPosition::RIGHT:
            this->setAxis(Axis::ROW);
            this->setDirection(Direction::RIGHT_TO_LEFT);
            this->setHorizontalMode(false);
            this->sidebar->setWidth(100);
            break;
        case AutoTabBarPosition::LEFT:
            this->setAxis(Axis::ROW);
            this->setDirection(Direction::LEFT_TO_RIGHT);
            this->setHorizontalMode(false);
            this->sidebar->setWidth(100);
            break;
    }
    this->tabBarPosition = position;
    this->invalidate();
}

void AutoTabFrame::addTab(std::string label, TabViewCreator creator, std::string icon, std::string iconActivate)
{
    this->addItem(label, icon, iconActivate, creator, [this](brls::View* view) {
        AutoSidebarItem* sidebarItem = (AutoSidebarItem*) view;

        // Only trigger when the sidebar item gains focus
        if (!view->isFocused())
            return;

        // Add the new tab
        View* newContent = sidebarItem->getAttachedView();

        this->setTabAttachedView(newContent);

        if (!newContent)
            return;

        newContent->registerAction(
                "hints/back"_i18n, BUTTON_B, [this](View* view) {
                    if (Application::getInputType() == InputType::TOUCH)
                        this->dismiss();
                    else
                        Application::giveFocus(this->sidebar);
                    return true;
                },
                false, false, SOUND_BACK);
    });
    if(this->sidebar->getChildren().size() - 1 == this->getDefaultTabIndex() ){
        AutoSidebarItem* item = (AutoSidebarItem*)this->sidebar->getChildren()[this->getDefaultTabIndex()];
        item->setActive(true);
        this->setTabAttachedView(item->getAttachedView());
    }
}

void AutoTabFrame::focusTab(int position)
{
    Application::giveFocus(this->getItem(position));
}

void AutoTabFrame::clearTabs()
{
    this->clearItems();
}

void AutoTabFrame::handleXMLElement(tinyxml2::XMLElement* element)
{
    std::string name = element->Name();

    if (name == "Tab")
    {
        const tinyxml2::XMLAttribute* labelAttribute = element->FindAttribute("label");
        const tinyxml2::XMLAttribute* iconAttribute = element->FindAttribute("icon");
        const tinyxml2::XMLAttribute* iconActivateAttribute = element->FindAttribute("iconActivate");

        if (!labelAttribute)
            fatal("\"label\" attribute missing from \"" + name + "\" tab");

        std::string label = View::getStringXMLAttributeValue(labelAttribute->Value());

        std::string icon, iconActivate;

        if(iconAttribute)
            icon = View::getFilePathXMLAttributeValue(iconAttribute->Value());

        if(iconActivateAttribute)
            iconActivate = View::getFilePathXMLAttributeValue(iconActivateAttribute->Value());

        tinyxml2::XMLElement* viewElement = element->FirstChildElement();

        if (viewElement)
        {
            this->addTab(label, [viewElement] {
                return View::createFromXMLElement(viewElement);
            }, icon, iconActivate);

            if (viewElement->NextSiblingElement())
                fatal("\"Tab\" can only contain one child element");
        }
        else
        {
            this->addTab(label, [] { return nullptr; }, icon, iconActivate);
        }
    }
    else
    {
        fatal("Unknown child element \"" + name + "\" for \"brls:Tab\"");
    }
}


/**
 * auto sidebar item
 */

const std::string autoSidebarItemXML = R"xml(
    <brls:Box
        width="auto"
        direction="rightToLeft"
        height="@style/brls/sidebar/item_height"
        focusable="true" >

        <brls:Box
            wireframe="false"
            grow="1.0"
            width="auto"
            height="auto"
            justifyContent="center"
            alignItems="center"
            axis="column"
            marginTop="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginBottom="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginRight="@style/brls/sidebar/item_accent_margin_sides"
            id="autoSidebar/item_label_box">

            <SVGImage
                wireframe="false"
                visibility="gone"
                id="autoSidebar/item_icon"
                width="26"
                height="26"/>

            <brls:Label
                wireframe="false"
                id="autoSidebar/item_label"
                width="auto"
                height="auto"
                fontSize="22"
                horizontalAlign="center"/>
        </brls:Box>

        <brls:Rectangle
            id="autoSidebar/item_accent"
            width="@style/brls/sidebar/item_accent_rect_width"
            height="auto"
            visibility="invisible"
            color="#FF6699"
            marginTop="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginBottom="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginLeft="@style/brls/sidebar/item_accent_margin_sides"
            marginRight="@style/brls/sidebar/item_accent_margin_sides" />

    </brls:Box>
)xml";

AutoSidebarItem::AutoSidebarItem()
        : Box(Axis::ROW)
{
    this->inflateFromXMLString(autoSidebarItemXML);

    this->registerStringXMLAttribute("label", [this](std::string value) {
        this->setLabel(value);
    });

    this->setFocusSound(SOUND_FOCUS_SIDEBAR);

    this->registerAction(
            "hints/ok"_i18n, BUTTON_A, [](View* view) {
                Application::onControllerButtonPressed(BUTTON_NAV_RIGHT, false);
                return true;
            },
            false, false, SOUND_CLICK_SIDEBAR);

    this->addGestureRecognizer(new TapGestureRecognizer([this](TapGestureStatus status, Sound* soundToPlay) {
        if (this->active)
            return;

        this->playClickAnimation(status.state != GestureState::UNSURE);

        switch (status.state)
        {
            case GestureState::UNSURE:
                *soundToPlay = SOUND_FOCUS_SIDEBAR;
                break;
            case GestureState::FAILED:
            case GestureState::INTERRUPTED:
                *soundToPlay = SOUND_TOUCH_UNFOCUS;
                break;
            case GestureState::END:
                *soundToPlay = SOUND_CLICK_SIDEBAR;
                Application::giveFocus(this);
                break;
        }
    }));
}

void AutoSidebarItem::setActive(bool active)
{
    if (active == this->active)
        return;

    Theme theme = Application::getTheme();

    if (active)
    {
        this->activeEvent.fire(this);

        this->accent->setVisibility(Visibility::VISIBLE);
        this->label->setTextColor(nvgRGB(255, 102, 153));
        if(this->icon->getVisibility() == brls::Visibility::VISIBLE){
            this->icon->setImageFromSVGFile(this->iconActivate);
        }
    }
    else
    {
        this->accent->setVisibility(Visibility::INVISIBLE);
        this->label->setTextColor(theme["brls/text"]);
        if(this->icon->getVisibility() == brls::Visibility::VISIBLE){
            this->icon->setImageFromSVGFile(this->iconDefault);
        }
    }

    this->active = active;
}

void AutoSidebarItem::onFocusGained()
{
    Box::onFocusGained();

    if (this->group)
        this->group->setActive(this);

    Logger::info("AutoSidebarItem: onFocusGained");
}

void AutoSidebarItem::onFocusLost()
{
    Box::onFocusLost();

    Logger::info("AutoSidebarItem: onFocusLost");
}

void AutoSidebarItem::setGroup(AutoSidebarItemGroup* group)
{
    this->group = group;

    if (group)
        group->add(this);
}

GenericEvent* AutoSidebarItem::getActiveEvent()
{
    return &this->activeEvent;
}

void AutoSidebarItem::setLabel(std::string label)
{
    this->label->setText(label);
}


/**
 * auto sidebar group
 */

void AutoSidebarItemGroup::add(AutoSidebarItem* item)
{
    this->items.push_back(item);
}

void AutoSidebarItemGroup::setActive(AutoSidebarItem* active)
{
    for (AutoSidebarItem* item : this->items)
    {
        if (item == active)
            item->setActive(true);
        else
            item->setActive(false);
    }
}

void AutoSidebarItemGroup::clear()
{
    this->items.clear();
}
