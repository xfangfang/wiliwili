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

#include "view/auto_tab_frame.hpp"
#include "view/svg_image.hpp"

/**
 * auto tab frame
 */
using namespace brls::literals;
using namespace brls;

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

brls::View* AutoTabFrame::create() {
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

    this->sidebar->setBackground(brls::ViewBackground::SIDEBAR);
    this->sidebar->setAxis(brls::Axis::COLUMN);
    this->sidebar->setPadding(32,10,47,10);
}

void AutoTabFrame::setSideBarPosition(AutoTabBarPosition position){
    switch (position)
    {
        case AutoTabBarPosition::TOP:
            this->setAxis(brls::Axis::COLUMN);
            this->setDirection(brls::Direction::LEFT_TO_RIGHT);
            this->setHorizontalMode(true);
            this->sidebar->setWidthPercentage(100);
            break;
        case AutoTabBarPosition::RIGHT:
            this->setAxis(brls::Axis::ROW);
            this->setDirection(brls::Direction::RIGHT_TO_LEFT);
            this->setHorizontalMode(false);
            this->sidebar->setWidth(100);
            break;
        case AutoTabBarPosition::LEFT:
            this->setAxis(brls::Axis::ROW);
            this->setDirection(brls::Direction::LEFT_TO_RIGHT);
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
                "hints/back"_i18n, brls::BUTTON_B, [this](View* view) {
                    if (brls::Application::getInputType() == brls::InputType::TOUCH)
                        this->dismiss();
                    else
                        brls::Application::giveFocus(this->sidebar);
                    return true;
                },
                false, false, brls::SOUND_BACK);
    });
    if(this->sidebar->getChildren().size() - 1 == this->getDefaultTabIndex() ){
        AutoSidebarItem* item = (AutoSidebarItem*)this->sidebar->getChildren()[this->getDefaultTabIndex()];
        item->setActive(true);
        this->setTabAttachedView(item->getAttachedView());
    }
}

void AutoTabFrame::focusTab(int position)
{
    brls::Application::giveFocus(this->getItem(position));
}

void AutoTabFrame::focus2NextTab(){
    int currentIndex = this->group.getActiveIndex();
    if(currentIndex < 0){
        // not found
    }else if(currentIndex + 1 >= this->sidebar->getChildren().size()){
        // shake highlight
        brls::Application::getCurrentFocus()->shakeHighlight(brls::FocusDirection::RIGHT);
    } else {
        this->focusTab(currentIndex + 1);
    }
}

void AutoTabFrame::focus2LastTab(){
    int currentIndex = this->group.getActiveIndex();
    if(currentIndex < 0){
        // not found
    }else if(currentIndex <= 0){
        // shake highlight
        //todo: 更精确地确认抖动方向
        brls::Application::getCurrentFocus()->shakeHighlight(brls::FocusDirection::LEFT);
    } else {
        this->focusTab(currentIndex - 1);
    }
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
            brls::fatal("\"label\" attribute missing from \"" + name + "\" tab");

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
                brls::fatal("\"Tab\" can only contain one child element");
        }
        else
        {
            this->addTab(label, [] { return nullptr; }, icon, iconActivate);
        }
    }
    else
    {
        brls::fatal("Unknown child element \"" + name + "\" for \"brls:Tab\"");
    }
}

AutoTabFrame::~AutoTabFrame(){
    if(this->activeTab){
        this->removeView(this->activeTab, false);
        this->activeTab = nullptr;
    }
}

void AutoTabFrame::setTabAttachedView(brls::View * newContent){
    // Remove the existing tab if it exists
    if (this->activeTab)
    {
        this->removeView(this->activeTab, false); // will call willDisappear but not delete
        this->activeTab = nullptr;
    }
    if(!newContent){
        return;
    }
    newContent->setGrow(1.0f);
    this->addView(newContent); // addView calls willAppear
    this->activeTab = newContent;
}

void AutoTabFrame::setDefaultTabIndex(size_t index){
    this->sidebar->setDefaultFocusedIndex(index);
}

size_t AutoTabFrame::getDefaultTabIndex(){
    return this->sidebar->getDefaultFocusedIndex();
}

brls::View* AutoTabFrame::getNextFocus(brls::FocusDirection direction, brls::View* currentView) {
    void* parentUserData = currentView->getParentUserData();

    // Return nullptr immediately if focus direction mismatches the box axis (clang-format refuses to split it in multiple lines...)
    if ((this->getAxis() == brls::Axis::ROW && direction != brls::FocusDirection::LEFT && \
                direction != brls::FocusDirection::RIGHT) || (this->getAxis() == brls::Axis::COLUMN && \
                direction != brls::FocusDirection::UP && direction != brls::FocusDirection::DOWN)){
        View* next = getParentNavigationDecision(this, nullptr, direction);
        if (!next && hasParent())
            next = getParent()->getNextFocus(direction, this);
        return next;
    }

    // Traverse the children
    size_t offset = 1; // which way we are going in the children list

    if ((this->getAxis() == brls::Axis::ROW && direction == brls::FocusDirection::LEFT && \
                tabBarPosition == AutoTabBarPosition::LEFT) || \
                (this->getAxis() == brls::Axis::ROW && direction == brls::FocusDirection::RIGHT && \
                tabBarPosition == AutoTabBarPosition::RIGHT) || \
                (this->getAxis() == brls::Axis::COLUMN && direction == brls::FocusDirection::UP)){
        offset = -1;
    }

    size_t currentFocusIndex = *((size_t*)parentUserData) + offset;
    View* currentFocus       = nullptr;

    while (!currentFocus && currentFocusIndex >= 0 && currentFocusIndex < this->getChildren().size()){
        currentFocus = this->getChildren()[currentFocusIndex]->getDefaultFocus();
        currentFocusIndex += offset;
    }

    currentFocus = getParentNavigationDecision(this, currentFocus, direction);
    if (!currentFocus && hasParent())
        currentFocus = getParent()->getNextFocus(direction, this);
    return currentFocus;
}

void AutoTabFrame::setFontSize(float size){
    this->itemFontSize = size;
    for(auto item: this->sidebar->getChildren()){
        ((AutoSidebarItem *)item)->setFontSize(size);
    }
}

float AutoTabFrame::getFontSize(){
    return this->itemFontSize;
}

void AutoTabFrame::setHorizontalMode(bool value){
    this->isHorizontal = value;
    for(auto item: this->sidebar->getChildren()){
        ((AutoSidebarItem *)item)->setHorizontalMode(value);
    }
    if(value){
        this->sidebar->setPadding(10,20,10,20);
        this->sidebar->setAxis(brls::Axis::ROW);
    } else {
        this->sidebar->setPadding(32,10,47,10);
        this->sidebar->setAxis(brls::Axis::COLUMN);
    }

    this->invalidate();
}

bool AutoTabFrame::getHorizontalMode(){
    return this->isHorizontal;
}

void AutoTabFrame::addItem(std::string label, std::string icon, std::string iconActivate,
             TabViewCreator creator, brls::GenericEvent::Callback focusCallback){
    AutoSidebarItem* item = new AutoSidebarItem();
    if(!icon.empty()){
        if(!iconActivate.empty())
            item->setIcon(icon, iconActivate);
        else
            item->setIcon(icon, icon);
    }
    item->setAttachedViewCreator(creator);
    item->setFontSize(this->itemFontSize);
    item->setHorizontalMode(this->isHorizontal);
    item->setGroup(&this->group);
    item->setLabel(label);
    item->getActiveEvent()->subscribe(focusCallback);

    this->sidebar->addView(item);
}

AutoSidebarItem* AutoTabFrame::getItem(int position)
{
    return dynamic_cast<AutoSidebarItem*>(this->sidebar->getChildren()[position]);
}

void AutoTabFrame::clearItems()
{
    this->sidebar->clearViews();
    this->group.clear();
}

Box* AutoTabFrame::getSidebar(){
    return this->sidebar;
}

View* AutoTabFrame::getActiveTab(){
    return this->activeTab;
}

void AutoTabFrame::focus2Sidebar(View* tabView) {
    AutoTabFrame* frame = dynamic_cast<AutoTabFrame*>(tabView->getParent());
    if(frame && frame->isOnTop){
        brls::Application::giveFocus(frame->getSidebar());
    }
}

void AutoTabFrame::willAppear(bool resetState) {
    this->isOnTop = true;
    Box::willAppear(resetState);
}

void AutoTabFrame::willDisappear(bool resetState){
    this->isOnTop = false;
    Box::willDisappear(resetState);
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
        : Box(brls::Axis::ROW)
{
    this->inflateFromXMLString(autoSidebarItemXML);

    this->registerStringXMLAttribute("label", [this](std::string value) {
        this->setLabel(value);
    });

    this->setFocusSound(brls::SOUND_FOCUS_SIDEBAR);

    // todo: 点击时根据sidebar位置决定向哪个方向移动
    this->registerAction(
            "hints/ok"_i18n, brls::BUTTON_A, [this](View* view) {
                if(this->attachedView)
                    brls::Application::giveFocus(this->attachedView);
                return true;
            },
            false, false, brls::SOUND_CLICK_SIDEBAR);

    this->addGestureRecognizer(new brls::TapGestureRecognizer([this](brls::TapGestureStatus status, brls::Sound* soundToPlay) {
        if (this->active)
            return;

        this->playClickAnimation(status.state != brls::GestureState::UNSURE);

        switch (status.state)
        {
            case brls::GestureState::UNSURE:
                *soundToPlay = brls::SOUND_FOCUS_SIDEBAR;
                break;
            case brls::GestureState::FAILED:
            case brls::GestureState::INTERRUPTED:
                *soundToPlay = brls::SOUND_TOUCH_UNFOCUS;
                break;
            case brls::GestureState::END:
                *soundToPlay = brls::SOUND_CLICK_SIDEBAR;
                brls::Application::giveFocus(this);
                break;
            default:
                break;
        }
    }));
}

void AutoSidebarItem::setActive(bool active)
{
    if (active == this->active)
        return;

    brls::Theme theme = brls::Application::getTheme();

    if (active)
    {
        this->activeEvent.fire(this);

        this->accent->setVisibility(brls::Visibility::VISIBLE);
        this->label->setTextColor(nvgRGB(255, 102, 153));
        if(this->icon->getVisibility() == brls::Visibility::VISIBLE){
            this->icon->setImageFromSVGFile(this->iconActivate);
        }
    }
    else
    {
        this->accent->setVisibility(brls::Visibility::INVISIBLE);
        this->label->setTextColor(theme["brls/text"]);
        if(this->icon->getVisibility() == brls::Visibility::VISIBLE){
            this->icon->setImageFromSVGFile(this->iconDefault);
        }
    }

    this->active = active;
}

bool AutoSidebarItem::isActive(){
    return this->active;
};

void AutoSidebarItem::onFocusGained()
{
    Box::onFocusGained();

    if (this->group)
        this->group->setActive(this);

    brls::Logger::info("AutoSidebarItem: onFocusGained");
}

void AutoSidebarItem::onFocusLost()
{
    Box::onFocusLost();

    brls::Logger::info("AutoSidebarItem: onFocusLost");
}

void AutoSidebarItem::setGroup(AutoSidebarItemGroup* group)
{
    this->group = group;

    if (group)
        group->add(this);
}

brls::GenericEvent* AutoSidebarItem::getActiveEvent()
{
    return &this->activeEvent;
}

void AutoSidebarItem::setLabel(std::string label)
{
    this->label->setText(label);
}

brls::View* AutoSidebarItem::AutoSidebarItem::getAttachedView(){
    if(!this->attachedView && this->attachedViewCreator){
        this->attachedView = this->attachedViewCreator();
        AttachedView* v = dynamic_cast<AttachedView*>(this->attachedView);
        if(v){
            v->setTabBar(this);
            v->onCreate();
        }
    }
    return this->attachedView;
}

void AutoSidebarItem::setIcon(std::string icon_default, std::string icon_activate){
    this->iconDefault = icon_default;
    this->iconActivate = icon_activate;

    this->icon->setImageFromSVGFile(icon_default);
    this->icon->setVisibility(brls::Visibility::VISIBLE);
}

void AutoSidebarItem::setFontSize(float size){
    if(this->icon->getVisibility() == brls::Visibility::VISIBLE){
        size -= 10;
        if(size < 8)
            size = 8;
    }
    this->label->setFontSize(size);
}

void AutoSidebarItem::setHorizontalMode(bool value){
    if(value){
        this->setAxis(brls::Axis::COLUMN);
        this->setPaddingLeft(10);
        this->setPaddingRight(10);
        this->icon_box->setMarginRight(0);
        this->icon_box->setMarginBottom(0);
        this->accent->setSize(brls::Size(View::AUTO, 4));
        this->accent->setMarginTop(0);
    } else {
        this->setAxis(brls::Axis::ROW);
        this->setPaddingLeft(0);
        this->setPaddingRight(0);
        this->icon_box->setMarginBottom(9);
        this->icon_box->setMarginRight(8);
        this->accent->setSize(brls::Size(4, View::AUTO));
        this->accent->setMarginTop(9);
    }
}

size_t AutoSidebarItem::getCurrentIndex(){
    return *((size_t*)this->getParentUserData());
}

void AutoSidebarItem::setAttachedViewCreator(TabViewCreator creator){
    this->attachedViewCreator = creator;
}

AutoSidebarItem::~AutoSidebarItem(){
    if(this->attachedView)
        delete this->attachedView;
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

int AutoSidebarItemGroup::getActiveIndex(){
    for (AutoSidebarItem* item : this->items){
        if(item->isActive())
            return item->getCurrentIndex();
    }
    return -1;
}


/**
* AttachedView
*/

void AttachedView::setTabBar(AutoSidebarItem* view){
    this->tab = view;
}
AutoSidebarItem* AttachedView::getTabBar(){
    return this->tab;
}

void AttachedView::onCreate(){}

void AttachedView::registerTabAction(std::string hintText, enum brls::ControllerButton button,
        brls::ActionListener action, bool hidden, bool allowRepeating, enum Sound sound)
{
    this->registerAction(hintText, button, action, hidden, allowRepeating, sound);
    if(this->tab)
        this->tab->registerAction(hintText, button, action, hidden, allowRepeating, sound);
}