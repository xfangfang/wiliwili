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

#pragma once

#include <borealis.hpp>
#include "view/svg_image.hpp"

typedef std::function<brls::View*(void)> TabViewCreator;
using namespace brls;

enum class AutoTabBarPosition
{
    TOP,
    LEFT,
    RIGHT
};


class AutoSidebarItemGroup;


class AutoSidebarItem : public Box
{
public:
    AutoSidebarItem();

    void onFocusGained() override;
    void onFocusLost() override;

    void setGroup(AutoSidebarItemGroup* group);

    void setLabel(std::string label);

    void setActive(bool active);

    void setIcon(std::string icon_default, std::string icon_activate){
        this->iconDefault = icon_default;
        this->iconActivate = icon_activate;

        this->icon->setImageFromSVGFile(icon_default);
        this->icon->setVisibility(brls::Visibility::VISIBLE);
    }

    void setAttachedView(View* v){
        if(this->attachedView){
            Logger::error("You are already set attached view");
            return;
        }
        this->attachedView = v;
    }

    void setFontSize(float size){
        if(this->icon->getVisibility() == brls::Visibility::VISIBLE){
            size -= 10;
            if(size < 8)
                size = 8;
        }
        this->label->setFontSize(size);
    }

    void setHorizontalMode(bool value){
        if(value){
            this->setAxis(Axis::COLUMN);
            this->setPaddingLeft(10);
            this->setPaddingRight(10);
            this->icon_box->setMarginRight(0);
            this->icon_box->setMarginBottom(0);
            this->accent->setSize(Size(View::AUTO, 4));
            this->accent->setMarginTop(0);
        } else {
            this->setAxis(Axis::ROW);
            this->setPaddingLeft(0);
            this->setPaddingRight(0);
            this->icon_box->setMarginBottom(9);
            this->icon_box->setMarginRight(8);
            this->accent->setSize(Size(4, View::AUTO));
            this->accent->setMarginTop(9);
        }
    }

    size_t getCurrentIndex(){
        return *((size_t*)this->getParentUserData());
    }

    View* getAttachedView(){
        if(!this->attachedView && this->attachedViewCreator)
            this->attachedView = this->attachedViewCreator();

        return this->attachedView;
    }

    void setAttachedViewCreator(TabViewCreator creator){
        this->attachedViewCreator = creator;
    }

    ~AutoSidebarItem(){
        if(this->attachedView)
            delete this->attachedView;
    }

    GenericEvent* getActiveEvent();

private:
    BRLS_BIND(Rectangle, accent, "autoSidebar/item_accent");
    BRLS_BIND(Label, label, "autoSidebar/item_label");
    BRLS_BIND(Box, icon_box, "autoSidebar/item_label_box");
    BRLS_BIND(SVGImage, icon, "autoSidebar/item_icon");

    GenericEvent activeEvent;

    AutoSidebarItemGroup* group;

    bool active = false;
    View* attachedView = nullptr;
    TabViewCreator attachedViewCreator = nullptr;
    std::string iconDefault, iconActivate;
};


class AutoSidebarItemGroup
{
public:
    void add(AutoSidebarItem* item);
    void setActive(AutoSidebarItem* item);
    void clear();

private:
    std::vector<AutoSidebarItem*> items;
};


class AutoTabFrame : public Box{
public:
    AutoTabFrame();
    void setSideBarPosition(AutoTabBarPosition position);
    void handleXMLElement(tinyxml2::XMLElement* element) override;
    void addTab(std::string label, TabViewCreator creator, std::string icon= "", std::string iconActivate= "");
    void focusTab(int position);
    void clearTabs();
    static View* create();
    ~AutoTabFrame(){
        if(this->activeTab){
            this->removeView(this->activeTab, false);
            this->activeTab = nullptr;
        }
    }

    void setTabAttachedView(View * newContent){
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

    void setDefaultTabIndex(size_t index){
        this->sidebar->setDefaultFocusedIndex(index);
    }

    size_t getDefaultTabIndex(){
        return this->sidebar->getDefaultFocusedIndex();
    }

    View* getNextFocus(FocusDirection direction, View* currentView) override
    {
        void* parentUserData = currentView->getParentUserData();

        // Return nullptr immediately if focus direction mismatches the box axis (clang-format refuses to split it in multiple lines...)
        if ((this->getAxis() == Axis::ROW && direction != FocusDirection::LEFT && direction != FocusDirection::RIGHT) || (this->getAxis() == Axis::COLUMN && direction != FocusDirection::UP && direction != FocusDirection::DOWN))
        {
            View* next = getParentNavigationDecision(this, nullptr, direction);
            if (!next && hasParent())
                next = getParent()->getNextFocus(direction, this);
            return next;
        }

        // Traverse the children
        size_t offset = 1; // which way we are going in the children list

        if ((this->getAxis() == Axis::ROW && direction == FocusDirection::LEFT && tabBarPosition == AutoTabBarPosition::LEFT) ||
            (this->getAxis() == Axis::ROW && direction == FocusDirection::RIGHT && tabBarPosition == AutoTabBarPosition::RIGHT) ||
            (this->getAxis() == Axis::COLUMN && direction == FocusDirection::UP))
        {
            offset = -1;
        }

        size_t currentFocusIndex = *((size_t*)parentUserData) + offset;
        View* currentFocus       = nullptr;

        while (!currentFocus && currentFocusIndex >= 0 && currentFocusIndex < this->getChildren().size())
        {
            currentFocus = this->getChildren()[currentFocusIndex]->getDefaultFocus();
            currentFocusIndex += offset;
        }

        currentFocus = getParentNavigationDecision(this, currentFocus, direction);
        if (!currentFocus && hasParent())
            currentFocus = getParent()->getNextFocus(direction, this);
        return currentFocus;
    }

    void setFontSize(float size){
        this->itemFontSize = size;
        for(auto item: this->sidebar->getChildren()){
            ((AutoSidebarItem *)item)->setFontSize(size);
        }
    }

    float getFontSize(){
        return this->itemFontSize;
    }

    void setHorizontalMode(bool value){
        this->isHorizontal = value;
        for(auto item: this->sidebar->getChildren()){
            ((AutoSidebarItem *)item)->setHorizontalMode(value);
        }
        if(value){
            this->sidebar->setPaddingTop(10);
            this->sidebar->setPaddingBottom(10);
            this->sidebar->setAxis(Axis::ROW);
        } else {
            this->sidebar->setPaddingTop(32);
            this->sidebar->setPaddingBottom(47);
            this->sidebar->setAxis(Axis::COLUMN);
        }

        this->invalidate();
    }

    bool getHorizontalMode(){
        return this->isHorizontal;
    }

    void addItem(std::string label, std::string icon, std::string iconActivate,
                 TabViewCreator creator, GenericEvent::Callback focusCallback)
    {
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

    AutoSidebarItem* getItem(int position)
    {
        return dynamic_cast<AutoSidebarItem*>(this->sidebar->getChildren()[position]);
    }

    void clearItems()
    {
        this->sidebar->clearViews();
        this->group.clear();
    }

private:
    BRLS_BIND(Box, sidebar, "auto_tab_frame/auto_sidebar");

    View* activeTab = nullptr;
    AutoTabBarPosition tabBarPosition = AutoTabBarPosition::LEFT;

    AutoSidebarItemGroup group;
    bool isHorizontal = false;
    float itemFontSize = 22;
};
