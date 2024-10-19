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

#include <borealis/views/rectangle.hpp>
#include <utility>

#include "view/auto_tab_frame.hpp"
#include "view/svg_image.hpp"
#include "view/button_refresh.hpp"

/**
 * auto tab frame
 */
using namespace brls::literals;

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

brls::View* AutoTabFrame::create() { return new AutoTabFrame(); }

AutoTabFrame::AutoTabFrame() {
    this->inflateFromXMLString(autoTabFrameContentXML);

    BRLS_REGISTER_ENUM_XML_ATTRIBUTE("sidebarPosition", AutoTabBarPosition, this->setSideBarPosition,
                                     {
                                         {"top", AutoTabBarPosition::TOP},
                                         {"right", AutoTabBarPosition::RIGHT},
                                         {"left", AutoTabBarPosition::LEFT},
                                     });

    // this only works with "sidebarPosition == left"
    // and It must be set before you set the sidebarPosition
    this->registerFloatXMLAttribute("sidebarWidth", [this](float value) { this->sidebarWidth = value; });

    this->registerFloatXMLAttribute("tabFontSize", [this](float value) { this->setFontSize(value); });

    this->registerFloatXMLAttribute("tabHeight", [this](float value) { this->sidebar->setHeight(value); });

    this->registerColorXMLAttribute("tabBackgroundColor",
                                    [this](NVGcolor value) { this->sidebar->setBackgroundColor(value); });

    this->registerColorXMLAttribute("tabItemDefaultBackgroundColor",
                                    [this](NVGcolor value) { this->setItemDefaultBackgroundColor(value); });

    this->registerColorXMLAttribute("tabItemActiveBackgroundColor",
                                    [this](NVGcolor value) { this->setItemActiveBackgroundColor(value); });

    this->registerColorXMLAttribute("tabItemActiveTextColor",
                                    [this](NVGcolor value) { this->setItemActiveTextColor(value); });

    // defaultTab: default is 0
    this->registerFloatXMLAttribute("defaultTab", [this](float value) { this->setDefaultTabIndex(value); });

    // default is true, only load pages on demand
    this->registerBoolXMLAttribute("demandMode", [this](bool value) { this->setDemandMode(value); });

    this->registerBoolXMLAttribute("disableNavigationRight", [this](bool value) { disableNavigationRight = value; });

    this->registerBoolXMLAttribute("disableNavigationDown", [this](bool value) { disableNavigationDown = value; });

    this->sidebar->setAxis(brls::Axis::COLUMN);
    this->sidebar->setPadding(32, 10, 47, 10);

    // Create Refresh button
    this->refreshButton = new ButtonRefresh();
    this->refreshButton->detach();
    this->refreshButton->setVisibility(brls::Visibility::GONE);
    this->refreshButton->registerClickAction([this](...) {
        if (this->refreshAction) this->refreshAction();
        return true;
    });
    // 使用 Box 的 addView，添加一个 detached button
    brls::Box::addView(this->refreshButton);
}

void AutoTabFrame::refresh() {
    this->refreshButton->startRotate();
    if (this->refreshAction) this->refreshAction();
}

void AutoTabFrame::setRefreshAction(const std::function<void()>& event) {
    this->refreshAction = event;
    this->refreshButton->setVisibility(brls::Visibility::VISIBLE);
}

void AutoTabFrame::setTabChangedAction(const std::function<void(size_t)>& event) { this->tabChangedAction = event; }

void AutoTabFrame::setDemandMode(bool value) { this->isDemandMode = value; }

void AutoTabFrame::setSideBarPosition(AutoTabBarPosition position) {
    this->tabBarPosition = position;
    switch (position) {
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
            this->sidebar->setWidth(sidebarWidth);
            break;
        case AutoTabBarPosition::LEFT:
            this->setAxis(brls::Axis::ROW);
            this->setDirection(brls::Direction::LEFT_TO_RIGHT);
            this->setHorizontalMode(false);
            this->sidebar->setWidth(sidebarWidth);
            break;
        default:;
    }
    this->invalidate();
}

AutoTabBarPosition AutoTabFrame::getSideBarPosition() { return this->tabBarPosition; }

int AutoTabFrame::getActiveIndex() { return this->group.getActiveIndex(); }

void AutoTabFrame::addTab(AutoSidebarItem* tab, TabViewCreator creator) {
    tab->setDefaultBackgroundColor(this->tabItemBackgroundColor);
    tab->setActiveBackgroundColor(this->tabItemActiveBackgroundColor);
    tab->setActiveTextColor(this->tabItemActiveTextColor);

    this->addItem(tab, std::move(creator), [this](brls::View* view) {
        auto* sidebarItem = (AutoSidebarItem*)view;

        // Only trigger when the sidebar item gains focus
        if (!view->isFocused()) return;

        // Add the new tab
        View* newContent = sidebarItem->getAttachedView();
        if (!newContent) {
            newContent = sidebarItem->createAttachedView();
        }

        if (newContent == this->getActiveTab()) return;

        this->setTabAttachedView(newContent);

        if (this->tabChangedAction) this->tabChangedAction(sidebarItem->getCurrentIndex());
    });
    auto isDefaultTab = this->sidebar->getChildren().size() - 1 == this->getDefaultTabIndex();

    if (isDefaultTab || !isDemandMode) {
        auto* item = (AutoSidebarItem*)this->sidebar->getChildren()[this->sidebar->getChildren().size() - 1];
        View* newContent      = item->getAttachedView();
        if (!newContent) {
            newContent = item->createAttachedView();
        }

        if (isDefaultTab) {
            this->group.setActive(item);
            this->setTabAttachedView(newContent);
        }
    }
}

void AutoTabFrame::focusTab(int position) { brls::Application::giveFocus(this->getItem(position)); }

void AutoTabFrame::focus2NextTab() {
    size_t sideBarNum = this->sidebar->getChildren().size();
    if (sideBarNum == 0) return;

    int currentIndex = this->group.getActiveIndex();
    if (currentIndex < 0) {
        // not found
        this->focusTab(0);
    } else if (sideBarNum == 1) {
        // shake highlight
        if (this->isHorizontal)
            brls::Application::getCurrentFocus()->shakeHighlight(brls::FocusDirection::RIGHT);
        else
            brls::Application::getCurrentFocus()->shakeHighlight(brls::FocusDirection::DOWN);
    } else if (currentIndex + 1 >= (int)sideBarNum) {
        // loop
        this->focusTab(0);
    } else {
        this->focusTab(currentIndex + 1);
    }
}

void AutoTabFrame::focus2LastTab() {
    size_t sideBarNum = this->sidebar->getChildren().size();
    if (sideBarNum == 0) return;

    int currentIndex = this->group.getActiveIndex();
    if (currentIndex < 0) {
        // not found
        this->focusTab(0);
    } else if (sideBarNum == 1) {
        // shake highlight
        if (this->isHorizontal)
            brls::Application::getCurrentFocus()->shakeHighlight(brls::FocusDirection::LEFT);
        else
            brls::Application::getCurrentFocus()->shakeHighlight(brls::FocusDirection::UP);
    } else if (currentIndex == 0) {
        // loop
        this->focusTab(sideBarNum - 1);
    } else {
        this->focusTab(currentIndex - 1);
    }
}

void AutoTabFrame::clearTabs() { this->clearItems(); }

void AutoTabFrame::clearTab(const std::string& name, bool onlyFirst) {
    for (auto& i : this->sidebar->getChildren()) {
        AutoSidebarItem* item = dynamic_cast<AutoSidebarItem*>(i);
        if (item && (item->getLabel() == name)) {
            // maybe something wrong will happen ?
            if (item->isFocused()) {
                this->setLastFocusedView(nullptr);
                brls::Application::giveFocus(this);
            }
            this->sidebar->removeView(item, true);
            this->group.removeView(item);

            if (onlyFirst) break;
        }
    }
}

bool AutoTabFrame::isHaveTab(const std::string& name) {
    for (auto& i : this->sidebar->getChildren()) {
        AutoSidebarItem* item = dynamic_cast<AutoSidebarItem*>(i);
        if (item && (item->getLabel() == name)) return true;
    }
    return false;
}

AutoSidebarItem* AutoTabFrame::getTab(const std::string& name) {
    for (auto& i : this->sidebar->getChildren()) {
        AutoSidebarItem* item = dynamic_cast<AutoSidebarItem*>(i);
        if (item && (item->getLabel() == name)) return item;
    }
    return nullptr;
}

AutoSidebarItem* AutoTabFrame::getTab(size_t value) {
    size_t index = 0;
    for (auto& i : this->sidebar->getChildren()) {
        AutoSidebarItem* item = dynamic_cast<AutoSidebarItem*>(i);
        if (item && index == value) {
            return item;
        }
        index++;
    }
    return nullptr;
}

void AutoTabFrame::handleXMLElement(tinyxml2::XMLElement* element) {
    std::string name = element->Name();

    if (name == "Tab") {
        const tinyxml2::XMLAttribute* labelAttribute = element->FindAttribute("label");
        const tinyxml2::XMLAttribute* styleAttribute = element->FindAttribute("style");

        if (!labelAttribute) brls::fatal("\"label\" attribute missing from \"" + name + "\" tab");

        std::string tabStyle = "accent";

        if (styleAttribute) tabStyle = View::getStringXMLAttributeValue(styleAttribute->Value());

        AutoSidebarItem* item = new AutoSidebarItem();
        item->applyXMLAttribute("style", tabStyle);
        item->applyXMLAttributes(element);

        tinyxml2::XMLElement* viewElement = element->FirstChildElement();

        if (viewElement) {
            this->addTab(item, [viewElement] { return View::createFromXMLElement(viewElement); });

            if (viewElement->NextSiblingElement()) brls::fatal("\"Tab\" can only contain one child element");
        } else {
            this->addTab(item, [] { return nullptr; });
        }
    } else {
        brls::fatal("Unknown child element \"" + name + "\" for \"brls:Tab\"");
    }
}

AutoTabFrame::~AutoTabFrame() {
    brls::Logger::debug("delete AutoTabFrame");
    if (this->activeTab) {
        // 直接移除activeTab，销毁的工作交给其对应的 AutoSidebarItem 来处理
        this->getChildren().erase(this->getChildren().begin() + 1);
        this->activeTab = nullptr;
    }
}

void AutoTabFrame::setTabAttachedView(brls::View* newContent) {
    // Remove the existing tab if it exists
    if (this->activeTab) {
        // will call willDisappear but not delete
        this->removeView(this->activeTab, false);
        // onHide will be called
        auto v = dynamic_cast<AttachedView*>(this->activeTab);
        if (v) v->onHide();
        this->activeTab = nullptr;
    }
    if (!newContent) {
        return;
    }
    newContent->setGrow(1.0f);
    this->addView(newContent);  // addView calls willAppear
    this->activeTab = newContent;
    // onHide will be called
    auto v = dynamic_cast<AttachedView*>(this->activeTab);
    if (v) v->onShow();
}

void AutoTabFrame::setDefaultTabIndex(size_t index) { this->sidebar->setDefaultFocusedIndex(index); }

size_t AutoTabFrame::getDefaultTabIndex() { return this->sidebar->getDefaultFocusedIndex(); }

brls::View* AutoTabFrame::getNextFocus(brls::FocusDirection direction, brls::View* currentView) {
    // Do not navigate, except through sidebar area
    if (currentView != this->sidebar) {
        if (disableNavigationDown && direction == brls::FocusDirection::DOWN) {
            return nullptr;
        }
        if (disableNavigationRight && direction == brls::FocusDirection::RIGHT) {
            return nullptr;
        }
    }

    void* parentUserData = currentView->getParentUserData();

    // Return nullptr immediately if focus direction mismatches the box axis (clang-format refuses to split it in multiple lines...)
    if ((this->getAxis() == brls::Axis::ROW && direction != brls::FocusDirection::LEFT &&
         direction != brls::FocusDirection::RIGHT) ||
        (this->getAxis() == brls::Axis::COLUMN && direction != brls::FocusDirection::UP &&
         direction != brls::FocusDirection::DOWN)) {
        View* next = getParentNavigationDecision(this, nullptr, direction);
        if (!next && hasParent()) next = getParent()->getNextFocus(direction, this);
        return next;
    }

    // Traverse the children
    size_t offset = 1;  // which way we are going in the children list

    if ((this->getAxis() == brls::Axis::ROW && direction == brls::FocusDirection::LEFT &&
         tabBarPosition == AutoTabBarPosition::LEFT) ||
        (this->getAxis() == brls::Axis::ROW && direction == brls::FocusDirection::RIGHT &&
         tabBarPosition == AutoTabBarPosition::RIGHT) ||
        (this->getAxis() == brls::Axis::COLUMN && direction == brls::FocusDirection::UP)) {
        offset = -1;
    }

    size_t currentFocusIndex = *((size_t*)parentUserData) + offset;
    View* currentFocus       = nullptr;

    while (!currentFocus && currentFocusIndex >= 0 && currentFocusIndex < this->getChildren().size()) {
        currentFocus = this->getChildren()[currentFocusIndex]->getDefaultFocus();
        currentFocusIndex += offset;
    }

    currentFocus = getParentNavigationDecision(this, currentFocus, direction);
    if (!currentFocus && hasParent()) currentFocus = getParent()->getNextFocus(direction, this);
    return currentFocus;
}

void AutoTabFrame::setFontSize(float size) {
    this->itemFontSize = size;
    for (auto item : this->sidebar->getChildren()) {
        ((AutoSidebarItem*)item)->setFontSize(size);
    }
}

float AutoTabFrame::getFontSize() { return this->itemFontSize; }

void AutoTabFrame::setHorizontalMode(bool value) {
    this->isHorizontal = value;
    for (auto item : this->sidebar->getChildren()) {
        ((AutoSidebarItem*)item)->setHorizontalMode(value);
    }
    if (value) {
        this->sidebar->setPadding(8, 20, 8, 20);
        this->sidebar->setAxis(brls::Axis::ROW);
    } else {
        this->sidebar->setPadding(32, 10, 47, 10);
        this->sidebar->setAxis(brls::Axis::COLUMN);
    }

    this->invalidate();
}

bool AutoTabFrame::getHorizontalMode() { return this->isHorizontal; }

void AutoTabFrame::addItem(AutoSidebarItem* tab, TabViewCreator creator, brls::GenericEvent::Callback focusCallback) {
    tab->setAttachedViewCreator(creator);
    tab->setHorizontalMode(this->isHorizontal);
    tab->setGroup(&this->group);
    tab->getActiveEvent()->subscribe(focusCallback);

    this->sidebar->addView(tab);
}

AutoSidebarItem* AutoTabFrame::getItem(int position) {
    return dynamic_cast<AutoSidebarItem*>(this->sidebar->getChildren()[position]);
}

void AutoTabFrame::clearItems() {
    this->setTabAttachedView(nullptr);
    this->sidebar->clearViews();
    this->group.clear();
    this->setLastFocusedView(nullptr);
}

brls::Box* AutoTabFrame::getSidebar() { return this->sidebar; }

brls::View* AutoTabFrame::getActiveTab() { return this->activeTab; }

void AutoTabFrame::focus2Sidebar(View* tabView) {
    AutoTabFrame* frame = dynamic_cast<AutoTabFrame*>(tabView->getParent());
    if (frame && frame->isOnTop) {
        brls::Application::giveFocus(frame->getSidebar());
    }
}

void AutoTabFrame::willAppear(bool resetState) {
    this->isOnTop = true;
    Box::willAppear(resetState);
}

void AutoTabFrame::willDisappear(bool resetState) {
    this->isOnTop = false;
    Box::willDisappear(resetState);
}

void AutoTabFrame::setItemDefaultBackgroundColor(NVGcolor c) {
    tabItemBackgroundColor = c;
    for (auto i : sidebar->getChildren()) {
        AutoSidebarItem* item = dynamic_cast<AutoSidebarItem*>(i);
        if (item) {
            item->setDefaultBackgroundColor(c);
        }
    }
}

void AutoTabFrame::setItemActiveBackgroundColor(NVGcolor c) {
    tabItemActiveBackgroundColor = c;
    for (auto i : sidebar->getChildren()) {
        AutoSidebarItem* item = dynamic_cast<AutoSidebarItem*>(i);
        if (item) {
            item->setActiveBackgroundColor(c);
        }
    }
}

void AutoTabFrame::setItemActiveTextColor(NVGcolor c) {
    tabItemActiveTextColor = c;
    for (auto i : sidebar->getChildren()) {
        AutoSidebarItem* item = dynamic_cast<AutoSidebarItem*>(i);
        if (item) {
            item->setActiveTextColor(c);
        }
    }
}

void AutoTabFrame::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style, brls::FrameContext* ctx) {
    //todo: 最后绘制刷新按钮

    if (this->sidebar && this->sidebar->getChildren().size() == 0) {
        // Draw skeleton screen
        // Only fit to home_bangumi and home_cinema page

        brls::Time curTime = brls::getCPUTimeUsec() / 1000;
        float p            = (curTime % 1000) * 1.0 / 1000;
        p                  = fabs(0.5 - p) + 0.25;

        float padding      = 20;
        auto drawWidth     = width - 3 * padding;
        auto drawHeight    = height - padding;
        auto drawX         = x + padding + getMarginLeft();
        auto drawY         = y + padding;
        auto sidebarHeight = this->sidebar->getHeight() - 10;

        if (this->isHorizontal) {
            drawHeight -= sidebarHeight;
            drawY += sidebarHeight;
        }

        NVGcolor end = skeletonBackground;
        end.a        = p;
        NVGpaint paint =
            nvgLinearGradient(vg, drawX, drawY, drawX + drawWidth, drawY + drawHeight, a(skeletonBackground), a(end));
        nvgBeginPath(vg);
        nvgFillPaint(vg, paint);
        nvgRoundedRect(vg, drawX, drawY, drawWidth, drawHeight, 6);
        nvgFill(vg);

        if (!this->isHorizontal) return;

        // draw sidebar items
        const unsigned int num       = 6;
        const unsigned int itemWidth = 80;
        drawY                        = y + 10;
        drawX                        = x + padding;
        padding                      = 10;

        for (size_t i = 0; i < num; i++) {
            paint = nvgLinearGradient(vg, drawX, drawY, drawX + itemWidth, drawY + sidebarHeight, a(skeletonBackground),
                                      a(end));
            nvgBeginPath(vg);
            nvgFillPaint(vg, paint);
            nvgRoundedRect(vg, drawX, drawY, itemWidth, sidebarHeight, 6);
            nvgFill(vg);
            drawX += padding + itemWidth;
        }
    }

    Box::draw(vg, x, y, width, height, style, ctx);
}

void AutoTabFrame::onLayout() {
    View::onLayout();
    if (this->refreshButton) this->refreshButton->setDetachedPosition(getWidth() - 80, getHeight() - 80);
}

/**
 * auto sidebar item
 */

const std::string autoSidebarItemXML = R"xml(
    <brls:Box
        wireframe="false"
        width="auto"
        direction="rightToLeft"
        height="auto"
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
                width="34"
                height="34"/>

            <brls:Label
                wireframe="false"
                id="autoSidebar/item_label"
                width="auto"
                height="auto"
                fontSize="22"
                marginBottom="5"
                horizontalAlign="center"/>

            <brls:Label
                wireframe="false"
                id="autoSidebar/subtitle_label"
                singleLine="true"
                width="auto"
                minWidth="80"
                height="auto"
                fontSize="12"
                textColor="#80808080"
                positionType="absolute"
                positionTop="-12"
                horizontalAlign="center"/>
        </brls:Box>

        <brls:Rectangle
            id="autoSidebar/item_accent"
            width="@style/brls/sidebar/item_accent_rect_width"
            height="auto"
            visibility="invisible"
            marginTop="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginBottom="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginLeft="@style/brls/sidebar/item_accent_margin_sides"
            marginRight="@style/brls/sidebar/item_accent_margin_sides" />

    </brls:Box>
)xml";

const std::string autoSidebarItemPlainXML = R"xml(
    <brls:Box
        highlightCornerRadius="8"
        cornerRadius="4"
        hideHighlightBackground="true"
        wireframe="false"
        width="auto"
        direction="rightToLeft"
        height="auto"
        marginLeft="8"
        marginRight="8"
        focusable="true" >

        <brls:Box
            wireframe="false"
            grow="1.0"
            width="auto"
            height="auto"
            justifyContent="center"
            alignItems="center"
            axis="column"
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

            <brls:Label
                wireframe="false"
                id="autoSidebar/subtitle_label"
                width="auto"
                height="auto"
                fontSize="12"
                positionType="absolute"
                positionTop="-12"
                horizontalAlign="center"/>

        </brls:Box>

        <brls:Rectangle
            id="autoSidebar/item_accent"
            width="@style/brls/sidebar/item_accent_rect_width"
            height="auto"
            visibility="gone"
            color="#FF6699"
            marginTop="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginBottom="@style/brls/sidebar/item_accent_margin_top_bottom"
            marginLeft="@style/brls/sidebar/item_accent_margin_sides"
            marginRight="@style/brls/sidebar/item_accent_margin_sides" />

    </brls:Box>
)xml";

const std::string autoSidebarItemInlineXML = R"xml(
<brls:Box
    wireframe="false"
    width="auto"
    height="auto"
    direction="rightToLeft"
    focusable="true" >

    <brls:Box
        wireframe="false"
        grow="1.0"
        width="auto"
        height="auto"
        alignItems="center"
        justifyContent="flexEnd"
        axis="row"
        marginLeft="@style/brls/sidebar/item_accent_margin_sides"
        marginTop="@style/brls/sidebar/item_accent_margin_top_bottom"
        marginBottom="@style/brls/sidebar/item_accent_margin_top_bottom"
        marginRight="@style/brls/sidebar/item_accent_margin_sides"
        id="autoSidebar/item_label_box">

        <brls:Label
            visibility="gone"
            id="autoSidebar/subtitle_label"/>

        <brls:Label
            wireframe="false"
            id="autoSidebar/item_label"
            width="auto"
            height="auto"
            fontSize="22"
            marginLeft="12"
            singleLine="true"/>

        <SVGImage
            wireframe="false"
            id="autoSidebar/item_icon"
            shrink="0"
            width="34"
            height="34"/>

    </brls:Box>

    <brls:Rectangle
        id="autoSidebar/item_accent"
        width="@style/brls/sidebar/item_accent_rect_width"
        height="auto"
        visibility="invisible"
        marginTop="@style/brls/sidebar/item_accent_margin_top_bottom"
        marginBottom="@style/brls/sidebar/item_accent_margin_top_bottom"
        marginLeft="@style/brls/sidebar/item_accent_margin_sides"
        marginRight="@style/brls/sidebar/item_accent_margin_sides" />

</brls:Box>
)xml";

AutoSidebarItem::AutoSidebarItem() : Box(brls::Axis::ROW) {
    this->registerStringXMLAttribute("label", [this](std::string value) {
        this->label->setText(value);
        return true;
    });

    this->registerFloatXMLAttribute("fontSize", [this](float value) {
        this->label->setFontSize(value);
        return true;
    });

    this->registerFilePathXMLAttribute("icon", [this](std::string value) {
        this->iconDefault = value;
        this->icon->setVisibility(brls::Visibility::VISIBLE);
        this->icon->setImageFromSVGFile(value);
    });

    this->registerFilePathXMLAttribute("iconActivate", [this](std::string value) { this->iconActivate = value; });

    BRLS_REGISTER_ENUM_XML_ATTRIBUTE("style", AutoTabBarStyle, this->setTabStyle,
                                     {
                                         {"accent", AutoTabBarStyle::ACCENT},
                                         {"plain", AutoTabBarStyle::PLAIN},
                                         {"inline", AutoTabBarStyle::INLINE},
                                     });

    this->setFocusSound(brls::SOUND_FOCUS_SIDEBAR);

    this->registerAction(
        "hints/ok"_i18n, brls::BUTTON_A,
        [this](View* view) {
            if (this->attachedView) brls::Application::giveFocus(this->attachedView);
            return true;
        },
        false, false, brls::SOUND_CLICK_SIDEBAR);

    this->addGestureRecognizer(
        new brls::TapGestureRecognizer([this](brls::TapGestureStatus status, brls::Sound* soundToPlay) {
            if (this->active) return;

            this->playClickAnimation(status.state != brls::GestureState::UNSURE);

            switch (status.state) {
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

void AutoSidebarItem::setTabStyle(AutoTabBarStyle style) {
    if (style == AutoTabBarStyle::NONE) brls::fatal("SidebarItem style cannot be set to \"None\"");
    if (this->tabStyle != AutoTabBarStyle::NONE) return;

    this->tabStyle = style;
    switch (style) {
        case AutoTabBarStyle::PLAIN:
            this->inflateFromXMLString(autoSidebarItemPlainXML);
            break;
        case AutoTabBarStyle::INLINE:
            this->inflateFromXMLString(autoSidebarItemInlineXML);
            break;
        default:
            this->inflateFromXMLString(autoSidebarItemXML);
    }
}

void AutoSidebarItem::setActive(bool active) {
    if (active == this->active) return;

    brls::Theme theme = brls::Application::getTheme();

    if (active) {
        this->activeEvent.fire(this);
        if (this->tabStyle == AutoTabBarStyle::ACCENT || this->tabStyle == AutoTabBarStyle::INLINE )
            this->accent->setVisibility(brls::Visibility::VISIBLE);
        else if (this->tabStyle == AutoTabBarStyle::PLAIN) {
            this->setBackgroundColor(this->tabItemActiveBackgroundColor);
        }

        this->label->setTextColor(this->tabItemActiveTextColor);

        if (this->icon->getVisibility() == brls::Visibility::VISIBLE) {
            if (!this->iconActivate.empty())
                this->icon->setImageFromSVGFile(this->iconActivate);
            else if (!this->iconDefault.empty())
                this->icon->setImageFromSVGFile(this->iconDefault);
        }
    } else {
        if (this->tabStyle == AutoTabBarStyle::ACCENT || this->tabStyle == AutoTabBarStyle::INLINE)
            this->accent->setVisibility(brls::Visibility::INVISIBLE);
        else if (this->tabStyle == AutoTabBarStyle::PLAIN) {
            this->setBackgroundColor(this->tabItemBackgroundColor);
        }
        this->label->setTextColor(theme["brls/text"]);

        if (this->icon->getVisibility() == brls::Visibility::VISIBLE && !this->iconDefault.empty())
            this->icon->setImageFromSVGFile(this->iconDefault);
    }

    this->active = active;
}

bool AutoSidebarItem::isActive() { return this->active; };

void AutoSidebarItem::onFocusGained() {
    Box::onFocusGained();

    if (this->group) this->group->setActive(this);
}

void AutoSidebarItem::onFocusLost() { Box::onFocusLost(); }

void AutoSidebarItem::setGroup(AutoSidebarItemGroup* group) {
    this->group = group;

    if (group) group->add(this);
}

brls::GenericEvent* AutoSidebarItem::getActiveEvent() { return &this->activeEvent; }

void AutoSidebarItem::setLabel(std::string text) { this->label->setText(text); }

void AutoSidebarItem::setSubtitle(std::string text) { this->subtitle->setText(text); }

std::string AutoSidebarItem::getLabel() { return this->label->getFullText(); }

brls::View* AutoSidebarItem::AutoSidebarItem::getAttachedView() { return this->attachedView; }

brls::View* AutoSidebarItem::createAttachedView() {
    if (!this->attachedView && this->attachedViewCreator) {
        this->attachedView = this->attachedViewCreator();
        AttachedView* v    = dynamic_cast<AttachedView*>(this->attachedView);
        if (v) {
            v->setTabBar(this);
            v->onCreate();
        }
    }
    if (!this->attachedView) {
        brls::fatal("AutoSidebarItem create attached View error");
    }
    this->attachedView->registerAction(
        "hints/back"_i18n, brls::BUTTON_B,
        [this](View* view) {
            if (brls::Application::getInputType() == brls::InputType::TOUCH)
                this->dismiss();
            else
                brls::Application::giveFocus(this);
            return true;
        },
        false, false, brls::SOUND_BACK);
    return this->attachedView;
}

brls::View* AutoSidebarItem::getView(std::string id) {
    View* v = Box::getView(id);
    if (v) return v;
    if (this->attachedView) {
        View* result = this->attachedView->getView(id);
        if (result) return result;
    }
    return nullptr;
}

void AutoSidebarItem::setFontSize(float size) {
    if (this->icon->getVisibility() == brls::Visibility::VISIBLE) {
        size -= 10;
        if (size < 8) size = 8;
    }
    this->label->setFontSize(size);
}

void AutoSidebarItem::setHorizontalMode(bool value) {
    if (value) {
        this->setAxis(brls::Axis::COLUMN);
        this->setPadding(0, 10, 0, 10);
        if (this->tabStyle == AutoTabBarStyle::ACCENT) {
            this->accent->setSize(brls::Size(View::AUTO, 4));
            this->accent->setMarginTop(0);
            this->icon_box->setMarginRight(0);
            this->icon_box->setMarginBottom(0);
        }
    } else {
        this->setAxis(brls::Axis::ROW);
        if (this->tabStyle == AutoTabBarStyle::ACCENT) {
            this->accent->setSize(brls::Size(4, View::AUTO));
            this->accent->setMarginTop(9);
            this->icon_box->setMarginBottom(9);
            this->icon_box->setMarginRight(8);
            this->setPadding(0, 0, 0, 0);
        } else if (this->tabStyle == AutoTabBarStyle::PLAIN) {
            this->setPadding(8, 0, 8, 0);
            this->setMargins(8, 0, 8, 0);
        }
    }
}

size_t AutoSidebarItem::getCurrentIndex() { return *((size_t*)this->getParentUserData()); }

void AutoSidebarItem::setAttachedViewCreator(TabViewCreator creator) { this->attachedViewCreator = creator; }

AutoSidebarItem::~AutoSidebarItem() {
    brls::Logger::debug("del AutoSidebarItem: {}", this->label->getFullText());
    if (this->attachedView) {
        this->attachedView->setParent(nullptr);
        if (!this->attachedView->isPtrLocked()) {
            delete this->attachedView;
        } else {
            this->attachedView->freeView();
        }
        this->attachedView = nullptr;
    }
}

AutoTabBarStyle AutoSidebarItem::getTabStyle(std::string value) {
    std::unordered_map<std::string, AutoTabBarStyle> enumMap = {
        {"accent", AutoTabBarStyle::ACCENT},
        {"plain", AutoTabBarStyle::PLAIN},
    };
    if (enumMap.count(value) > 0)
        return enumMap[value];
    else
        brls::fatal("Illegal value \"" + value + "\" for AutoSidebarItem attribute \"style\"");
}

void AutoSidebarItem::setDefaultBackgroundColor(NVGcolor c) {
    tabItemBackgroundColor = c;
    this->setBackgroundColor(this->tabItemBackgroundColor);
}

void AutoSidebarItem::setActiveBackgroundColor(NVGcolor c) {
    tabItemActiveBackgroundColor = c;
    if (this->isActive()) {
        this->setBackgroundColor(this->tabItemActiveBackgroundColor);
    }
}

void AutoSidebarItem::setActiveTextColor(NVGcolor c) {
    tabItemActiveTextColor = c;
    this->accent->setColor(c);
    if (this->isActive()) {
        this->label->setTextColor(c);
    }
}

/**
 * auto sidebar group
 */

void AutoSidebarItemGroup::add(AutoSidebarItem* item) { this->items.push_back(item); }

void AutoSidebarItemGroup::setActive(AutoSidebarItem* active) {
    for (AutoSidebarItem* item : this->items) {
        if (item == active)
            item->setActive(true);
        else
            item->setActive(false);
    }
}

void AutoSidebarItemGroup::clear() { this->items.clear(); }

void AutoSidebarItemGroup::removeView(AutoSidebarItem* view) {
    for (auto it = this->items.begin(); it != this->items.end(); it++) {
        if (*it == view) {
            this->items.erase(it);
            break;
        }
    }
}

int AutoSidebarItemGroup::getActiveIndex() {
    for (AutoSidebarItem* item : this->items) {
        if (item->isActive()) return item->getCurrentIndex();
    }
    return -1;
}

/**
 * AttachedView
 */

void AttachedView::setTabBar(AutoSidebarItem* view) { this->tab = view; }
AutoSidebarItem* AttachedView::getTabBar() { return this->tab; }

void AttachedView::onCreate() {}

void AttachedView::onShow() {}

void AttachedView::onHide() {}

void AttachedView::registerTabAction(std::string hintText, enum brls::ControllerButton button,
                                     brls::ActionListener action, bool hidden, bool allowRepeating, enum brls::Sound sound) {
    this->registerAction(hintText, button, action, hidden, allowRepeating, sound);
    if (this->tab) this->tab->registerAction(hintText, button, action, hidden, allowRepeating, sound);
}

AttachedView::AttachedView() { this->setGrow(1); }

AttachedView::~AttachedView() { brls::Logger::debug("delete AttachedView"); }