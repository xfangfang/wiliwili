#pragma once

#include <borealis/applet_frame.hpp>
#include <borealis/sidebar.hpp>
#include <borealis/application.hpp>
#include <borealis/box_layout.hpp>
#include <borealis/rectangle.hpp>
#include <string>
#include <vector>

using namespace brls::i18n::literals;


// An applet frame containing a sidebar on the left with multiple tabs
class FlexFrame : public brls::AppletFrame
{
  public:
    FlexFrame();

    /**
     * Adds a tab with given label and view
     * All tabs and separators must be added
     * before the TabFrame is itself added to
     * the view hierarchy
     */
    brls::SidebarItem* addTab(std::string label, brls::View* view);
    void addSeparator();
    void setSiderbarWidth(int width);
    void setSiderbarMargins(int top, int right, int bottom, int left);

    brls::View* getDefaultFocus() override;

    virtual bool onCancel() override;

    ~FlexFrame();

  private:
    brls::Sidebar* sidebar;
    brls::BoxLayout* layout;
    brls::View* rightPane = nullptr;

    void switchToView(brls::View* view);
};




FlexFrame::FlexFrame()
    : brls::AppletFrame(false, true)
{
    //Create sidebar
    this->sidebar = new brls::Sidebar();

    // Setup content view
    this->layout = new brls::BoxLayout(brls::BoxLayoutOrientation::HORIZONTAL);
    layout->addView(sidebar);

    this->setContentView(layout);

    this->setTitle("main/name"_i18n);
    this->setIcon(BOREALIS_ASSET("icon/bilibili_128x128.jpg"));
    this->setSiderbarWidth(160);
    this->setSiderbarMargins(40,30,40,30);
}

bool FlexFrame::onCancel()
{
    // Go back to sidebar if not already focused
    if (!this->sidebar->isChildFocused())
    {
        brls::Application::onGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, false);
        return true;
    }

    return brls::AppletFrame::onCancel();
}

void FlexFrame::switchToView(brls::View* view)
{
    if (this->rightPane == view)
        return;

    if (this->layout->getViewsCount() > 1)
    {
        if (this->rightPane)
            this->rightPane->willDisappear(true);
        this->layout->removeView(1, false);
    }

    this->rightPane = view;
    if (this->rightPane != nullptr)
        this->layout->addView(this->rightPane, true, true); // addView() calls willAppear()
}

brls::SidebarItem* FlexFrame::addTab(std::string label, brls::View* view)
{
    brls::SidebarItem* item = this->sidebar->addItem(label, view);
    item->getFocusEvent()->subscribe([this](View* view) {
        if (brls::SidebarItem* item = dynamic_cast<brls::SidebarItem*>(view))
            this->switchToView(item->getAssociatedView());
    });

    // Switch to first one as soon as we add it
    if (!this->rightPane)
    {
        brls::Logger::debug("Switching to the first tab");
        this->switchToView(view);
    }
    return item;
}

void FlexFrame::addSeparator()
{
    this->sidebar->addSeparator();
}

void FlexFrame::setSiderbarWidth(int width)
{
    this->sidebar->setWidth(width);
}

void FlexFrame::setSiderbarMargins(int top, int right, int bottom, int left){
  this->sidebar->setMargins(top,right,bottom,left);
}

brls::View* FlexFrame::getDefaultFocus()
{
    // Try to focus the right pane
    if (this->layout->getViewsCount() > 1)
    {
        brls::View* newFocus = this->rightPane->getDefaultFocus();

        if (newFocus)
            return newFocus;
    }

    // Otherwise focus sidebar
    return this->sidebar->getDefaultFocus();
}

FlexFrame::~FlexFrame()
{
    switchToView(nullptr);

    // Content view is freed by ~AppletFrame()
}
