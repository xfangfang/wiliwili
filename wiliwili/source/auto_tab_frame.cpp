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

        <AutoSidebar
            id="brls/tab_frame/sidebar"
            width="@style/brls/tab_frame/sidebar_width"
            height="auto" />

        <!-- Content will be injected here with grow="1.0" -->

    </brls:Box>
)xml";

View* AutoTabFrame::create() {
    return new AutoTabFrame();
}

AutoTabFrame::AutoTabFrame() {
//    410.0f, 100.0f
    Logger::info("create AutoTabFrame");
    this->inflateFromXMLString(autoTabFrameContentXML);
}

void AutoTabFrame::addTab(std::string label, TabViewCreator creator)
{
    this->sidebar->addItem(label, [this, creator](brls::View* view) {
        AutoSidebarItem* sidebarItem = (AutoSidebarItem*) view;

        // Only trigger when the sidebar item gains focus
        if (!view->isFocused())
            return;

        // Remove the existing tab if it exists
        if (this->activeTab)
        {
            this->removeView(this->activeTab, false); // will call willDisappear but not delete
            this->activeTab = nullptr;
        }

        // Add the new tab
        View* newContent = sidebarItem->getAttachedView();
        if(!newContent){
            newContent = creator();
            sidebarItem->setAttachedView(newContent);
        }
        if (!newContent)
            return;

        newContent->setGrow(1.0f);
        this->addView(newContent); // addView calls willAppear

        this->activeTab = newContent;

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
}

void AutoTabFrame::focusTab(int position)
{
    Application::giveFocus(this->sidebar->getItem(position));
}

void AutoTabFrame::clearTabs()
{
    this->sidebar->clearItems();
}

void AutoTabFrame::addSeparator()
{
    this->sidebar->addSeparator();
}

void AutoTabFrame::handleXMLElement(tinyxml2::XMLElement* element)
{
    std::string name = element->Name();

    if (name == "Tab")
    {
        const tinyxml2::XMLAttribute* labelAttribute = element->FindAttribute("label");

        if (!labelAttribute)
            fatal("\"label\" attribute missing from \"" + name + "\" tab");

        std::string label = View::getStringXMLAttributeValue(labelAttribute->Value());

        tinyxml2::XMLElement* viewElement = element->FirstChildElement();

        if (viewElement)
        {
            this->addTab(label, [viewElement] {
                return View::createFromXMLElement(viewElement);
            });

            if (viewElement->NextSiblingElement())
                fatal("\"Tab\" can only contain one child element");
        }
        else
        {
            this->addTab(label, [] { return nullptr; });
        }
    }
    else if (name == "Separator")
    {
        this->addSeparator();
    }
    else
    {
        fatal("Unknown child element \"" + name + "\" for \"brls:Tab\"");
    }
}


/**
 * auto sidebar
 */


View * AutoSidebar::create() {
    return new AutoSidebar();
}

AutoSidebar::AutoSidebar()
{
    Style style = Application::getStyle();

    this->setScrollingBehavior(ScrollingBehavior::CENTERED);
    this->setBackground(ViewBackground::SIDEBAR);

    // Create content box
    this->contentBox = new Box(Axis::COLUMN);

    this->contentBox->setPadding(
            style["brls/sidebar/padding_top"],
            10,
            style["brls/sidebar/padding_bottom"],
            10);

    this->setContentView(this->contentBox);
    this->setScrollingIndicatorVisible(false);
    Logger::info("create AutoSidebar");
}

void AutoSidebar::addItem(std::string label, GenericEvent::Callback focusCallback)
{
    AutoSidebarItem* item = new AutoSidebarItem();
    item->setGroup(&this->group);
    item->setLabel(label);
    item->getActiveEvent()->subscribe(focusCallback);

    this->contentBox->addView(item);
}

AutoSidebarItem* AutoSidebar::getItem(int position)
{
    return dynamic_cast<AutoSidebarItem*>(this->contentBox->getChildren()[position]);
}

void AutoSidebar::addSeparator()
{
    this->contentBox->addView(new SidebarSeparator());
}

void AutoSidebar::clearItems()
{
    this->contentBox->clearViews();
    group.clear();
}

void AutoSidebar::onChildFocusLost(View *directChild, View *focusedView) {
    ScrollingFrame::onChildFocusLost(directChild, focusedView);
    Logger::error("AutoSidebar: onChildFocusLost");
}

void AutoSidebar::onFocusLost() {
    Box::onFocusLost();
    Logger::info("AutoSidebar: onFocusLost");
}

View *AutoSidebar::getNextFocus(FocusDirection direction, View *currentView) {

    View * next = ScrollingFrame::getNextFocus(direction, currentView);
    if(next && direction == FocusDirection::RIGHT){
        Logger::info("AutoSidebar to right");
        this->setWidth(100.0f);
    }
    return next;
}

void AutoSidebar::onChildFocusGained(View *directChild, View *focusedView) {
    ScrollingFrame::onChildFocusGained(directChild, focusedView);
    Logger::info("AutoSidebar onChildFocusGained");
    //200
    this->setWidth(100.0f);
}


/**
 * auto sidebar item
 */

const std::string autoSidebarItemXML = R"xml(
    <brls:Box
        width="auto"
        height="@style/brls/sidebar/item_height"
        focusable="true" >

        <brls:Rectangle
            id="brls/sidebar/item_accent"
            width="@style/brls/sidebar/item_accent_rect_width"
            height="auto"
            visibility="invisible"
            color="@theme/brls/sidebar/active_item"
            marginTop="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginBottom="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginLeft="@style/brls/sidebar/item_accent_margin_sides"
            marginRight="@style/brls/sidebar/item_accent_margin_sides" />

        <brls:Label
            id="brls/sidebar/item_label"
            width="auto"
            height="auto"
            grow="1.0"
            fontSize="@style/brls/sidebar/item_font_size"
            marginTop="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginBottom="@style/brls/sidebar/item_accent_margin_top_bottom"
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
        this->label->setTextColor(theme["brls/sidebar/active_item"]);
    }
    else
    {
        this->accent->setVisibility(Visibility::INVISIBLE);
        this->label->setTextColor(theme["brls/text"]);
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
