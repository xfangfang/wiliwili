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

typedef std::function<brls::View*(void)> TabViewCreator;
using namespace brls;

class AutoSidebarItemGroup;
class AutoSidebarItem;
class AutoSidebar;

class AutoTabFrame : public Box{
public:
    AutoTabFrame();
    void handleXMLElement(tinyxml2::XMLElement* element) override;
    void addTab(std::string label, TabViewCreator creator);
    void focusTab(int position);
    void clearTabs();
    void addSeparator();
    static View* create();
    ~AutoTabFrame(){
        if(this->activeTab){
            this->removeView(this->activeTab, false);
            this->activeTab = nullptr;
        }
    }

private:
    BRLS_BIND(AutoSidebar, sidebar, "brls/tab_frame/sidebar");

    View* activeTab = nullptr;
};

class AutoSidebarItem : public Box
{
public:
    AutoSidebarItem();

    void onFocusGained() override;
    void onFocusLost() override;

    void setGroup(AutoSidebarItemGroup* group);

    void setLabel(std::string label);

    void setActive(bool active);

    void setAttachedView(View* v){
        this->attachedView = v;
    }

    View* getAttachedView(){
        return this->attachedView;
    }

    ~AutoSidebarItem(){
        if(this->attachedView)
            delete this->attachedView;
    }

    GenericEvent* getActiveEvent();

private:
    BRLS_BIND(Rectangle, accent, "brls/sidebar/item_accent");
    BRLS_BIND(Label, label, "brls/sidebar/item_label");

    GenericEvent activeEvent;

    AutoSidebarItemGroup* group;

    bool active = false;
    View* attachedView = nullptr;
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

class AutoSidebar : public ScrollingFrame
{
public:
    AutoSidebar();
    void addItem(std::string label, GenericEvent::Callback focusCallback);
    AutoSidebarItem* getItem(int position);
    void addSeparator();
    void clearItems();
    static View* create();
    void onFocusLost() override;
    void onChildFocusLost(View *directChild, View *focusedView) override;

    void onChildFocusGained(View *directChild, View *focusedView) override;

    View *getNextFocus(FocusDirection direction, View *currentView) override;

private:
    AutoSidebarItemGroup group;
    Box* contentBox;
};
