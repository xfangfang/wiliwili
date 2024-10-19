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

#include <functional>
#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>
#include <borealis/core/application.hpp>

namespace brls {
class Label;
class Rectangle;
}  // namespace brls

typedef std::function<brls::View*(void)> TabViewCreator;

enum class AutoTabBarPosition { TOP, LEFT, RIGHT };

enum class AutoTabBarStyle { ACCENT, PLAIN, INLINE, NONE };

class AutoSidebarItemGroup;
class AttachedView;
class SVGImage;
class ButtonRefresh;

class AutoSidebarItem : public brls::Box {
public:
    AutoSidebarItem();

    void onFocusGained() override;
    void onFocusLost() override;

    void setGroup(AutoSidebarItemGroup* group);

    void setLabel(std::string label);

    void setSubtitle(std::string label);

    std::string getLabel();

    void setActive(bool active);

    bool isActive();

    void setFontSize(float size);

    void setHorizontalMode(bool value);

    size_t getCurrentIndex();

    View* getAttachedView();

    View* createAttachedView();

    View* getView(std::string id) override;

    void setAttachedViewCreator(TabViewCreator creator);

    ~AutoSidebarItem() override;

    brls::GenericEvent* getActiveEvent();

    static AutoTabBarStyle getTabStyle(std::string value);

    void setTabStyle(AutoTabBarStyle style);

    void setDefaultBackgroundColor(NVGcolor c);

    void setActiveBackgroundColor(NVGcolor c);

    void setActiveTextColor(NVGcolor c);

private:
    BRLS_BIND(brls::Rectangle, accent, "autoSidebar/item_accent");
    BRLS_BIND(brls::Label, label, "autoSidebar/item_label");
    BRLS_BIND(brls::Label, subtitle, "autoSidebar/subtitle_label");
    BRLS_BIND(Box, icon_box, "autoSidebar/item_label_box");
    BRLS_BIND(SVGImage, icon, "autoSidebar/item_icon");

    brls::GenericEvent activeEvent;

    AutoSidebarItemGroup* group = nullptr;

    AutoTabBarStyle tabStyle = AutoTabBarStyle::NONE;

    NVGcolor tabItemBackgroundColor       = nvgRGBA(0, 0, 0, 0);
    NVGcolor tabItemActiveBackgroundColor = nvgRGBA(0, 0, 0, 0);
    NVGcolor tabItemActiveTextColor       = brls::Application::getTheme()["brls/text"];

    bool active                        = false;
    View* attachedView                 = nullptr;
    TabViewCreator attachedViewCreator = nullptr;
    std::string iconDefault, iconActivate;
};

class AutoSidebarItemGroup {
public:
    void add(AutoSidebarItem* item);
    void setActive(AutoSidebarItem* item);
    void clear();
    void removeView(AutoSidebarItem* view);
    int getActiveIndex();

private:
    std::vector<AutoSidebarItem*> items;
};

class AttachedView : public brls::Box {
public:
    AttachedView();

    void setTabBar(AutoSidebarItem* view);
    AutoSidebarItem* getTabBar();

    ~AttachedView() override;

    virtual void onCreate();

    virtual void onShow();

    virtual void onHide();

    View* getDefaultFocus() override { return brls::Box::getDefaultFocus(); }

    void registerTabAction(std::string hintText, enum brls::ControllerButton button, brls::ActionListener action,
                           bool hidden = false, bool allowRepeating = false, enum brls::Sound sound = brls::SOUND_NONE);

private:
    AutoSidebarItem* tab = nullptr;
};

class AutoTabFrame : public brls::Box {
public:
    AutoTabFrame();
    void handleXMLElement(tinyxml2::XMLElement* element) override;
    void addTab(AutoSidebarItem* tab, TabViewCreator creator);
    void focusTab(int position);
    void clearTabs();
    void clearTab(const std::string& name, bool onlyFirst = true);
    bool isHaveTab(const std::string& name);
    AutoSidebarItem* getTab(const std::string& name);
    AutoSidebarItem* getTab(size_t index);

    static brls::View* create();
    ~AutoTabFrame() override;

    void setTabAttachedView(brls::View* newContent);

    void setDefaultTabIndex(size_t index);

    size_t getDefaultTabIndex();

    brls::View* getNextFocus(brls::FocusDirection direction, brls::View* currentView) override;

    void setItemDefaultBackgroundColor(NVGcolor c);

    void setItemActiveBackgroundColor(NVGcolor c);

    void setItemActiveTextColor(NVGcolor c);

    void setFontSize(float size);

    float getFontSize();

    void setHorizontalMode(bool value);

    bool getHorizontalMode();

    void setDemandMode(bool value);

    void addItem(AutoSidebarItem* tab, TabViewCreator creator, brls::GenericEvent::Callback focusCallback);

    AutoSidebarItem* getItem(int position);

    void clearItems();

    Box* getSidebar();

    View* getActiveTab();

    void focus2NextTab();
    void focus2LastTab();

    static void focus2Sidebar(View* tabView);

    virtual void willAppear(bool resetState = false) override;

    virtual void willDisappear(bool resetState = false) override;

    bool isOnTop = false;

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    void onLayout() override;

    /**
     * Setting the position of sidebar.
     * Only set once.
     */
    void setSideBarPosition(AutoTabBarPosition position);

    AutoTabBarPosition getSideBarPosition();

    int getActiveIndex();

    void refresh();

    void setRefreshAction(const std::function<void()>& event);

    void setTabChangedAction(const std::function<void(size_t)>& event);

private:
    BRLS_BIND(Box, sidebar, "auto_tab_frame/auto_sidebar");

    View* activeTab                   = nullptr;
    AutoTabBarPosition tabBarPosition = AutoTabBarPosition::LEFT;

    AutoSidebarItemGroup group;
    bool isHorizontal  = false;
    bool isDemandMode  = true;  // load pages on demand
    float itemFontSize = 22;
    float sidebarWidth = 100;

    bool disableNavigationRight = false;
    bool disableNavigationDown  = false;

    ButtonRefresh* refreshButton        = nullptr;
    std::function<void()> refreshAction = nullptr;

    std::function<void(size_t)> tabChangedAction = nullptr;

    NVGcolor skeletonBackground           = brls::Application::getTheme()["color/grey_3"];
    NVGcolor tabItemBackgroundColor       = nvgRGBA(0, 0, 0, 0);
    NVGcolor tabItemActiveBackgroundColor = nvgRGBA(0, 0, 0, 0);
    NVGcolor tabItemActiveTextColor       = brls::Application::getTheme()["brls/text"];
};
