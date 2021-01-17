
/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2019-2020  natinusala
    Copyright (C) 2019  p-sam

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <borealis.hpp>
#include <string>
#include <unistd.h>
// #include "video.hpp"
#include "video_player.hpp"
#include "flex_frame.hpp"


namespace i18n = brls::i18n; // for loadTranslations() and getStr()
using namespace i18n::literals; // for _i18n

std::vector<std::string> NOTIFICATIONS = {
    "You have cool hair",
    "I like your shoes",
    "borealis is powered by nanovg",
    "The Triforce is an inside job",
    "Pozznx will trigger in one day and twelve hours",
    "Aurora Borealis? At this time of day, at this time of year, in this part of the gaming market, located entirely within your Switch?!",
    "May I see it?",
    "Hmm, Steamed Hams!"
};

VideoPlayer* player;
int main(int argc, char* argv[])
{
    // Init the app
    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
    brls::Logger::debug(BOREALIS_ASSET("icon/bilibili_128x128.jpg"));
    // char buf[180];
    // getcwd(buf,sizeof(buf));
    // brls::Logger::debug(buf);

    i18n::loadTranslations();
    if (!brls::Application::init("main/name"_i18n))
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    // Create a sample view
    FlexFrame* rootFrame = new FlexFrame();

    rootFrame->setTitle("main/name"_i18n);
    rootFrame->setIcon(BOREALIS_ASSET("icon/bilibili_128x128.jpg"));
    rootFrame->setSiderbarWidth(160);
    rootFrame->setSiderbarMargins(40,30,40,30);


    brls::List* testList = new brls::List();

    brls::ListItem* dialogItem = new brls::ListItem("main/pozznx/open"_i18n);
    dialogItem->getClickEvent()->subscribe([](brls::View* view) {
        brls::Dialog* dialog = new brls::Dialog("main/pozznx/warning"_i18n);

        brls::GenericEvent::Callback closeCallback = [dialog](brls::View* view) {
            dialog->close();
            brls::Application::notify("main/pozznx/canceling"_i18n);
        };

        brls::GenericEvent::Callback continueCallback = [dialog](brls::View* view) {
            dialog->close();
            brls::Application::notify("main/pozznx/running"_i18n);
        };

        dialog->addButton("main/pozznx/cancel"_i18n, closeCallback);
        dialog->addButton("main/pozznx/continue"_i18n, continueCallback);

        dialog->setCancelable(true);

        dialog->open();
    });

    brls::ListItem* notificationItem = new brls::ListItem("main/notify"_i18n);
    notificationItem->getClickEvent()->subscribe([](brls::View* view) {
        std::string notification = NOTIFICATIONS[std::rand() % NOTIFICATIONS.size()];
        brls::Application::notify(notification);
    });

    brls::ListItem* themeItem = new brls::ListItem("main/tv/resolution"_i18n);
    themeItem->setValue("main/tv/automatic"_i18n);

    brls::ListItem* i18nItem = new brls::ListItem(i18n::getStr("main/i18n/title", i18n::getCurrentLocale(), "main/i18n/lang"_i18n));

    brls::SelectListItem* jankItem = new brls::SelectListItem(
        "main/jank/jank"_i18n,
        {
            "main/jank/native"_i18n,
            "main/jank/minimal"_i18n,
            "main/jank/regular"_i18n,
            "main/jank/maximum"_i18n,
            "main/jank/saxophone"_i18n,
            "main/jank/vista"_i18n,
            "main/jank/ios"_i18n,
        });

    brls::ListItem* crashItem = new brls::ListItem("main/divide/title"_i18n, "main/divide/description"_i18n);
    crashItem->getClickEvent()->subscribe([](brls::View* view) { brls::Application::crash("main/divide/crash"_i18n); });

    brls::ListItem* popupItem = new brls::ListItem("popup/open"_i18n);
    popupItem->getClickEvent()->subscribe([](brls::View* view) {
        brls::TabFrame* popupTabFrame = new brls::TabFrame();
        popupTabFrame->addTab("popup/red"_i18n, new brls::Rectangle(nvgRGB(255, 0, 0)));
        popupTabFrame->addTab("popup/green"_i18n, new brls::Rectangle(nvgRGB(0, 255, 0)));
        popupTabFrame->addTab("popup/blue"_i18n, new brls::Rectangle(nvgRGB(0, 0, 255)));
        brls::PopupFrame::open("popup/title"_i18n, BOREALIS_ASSET("icon/borealis.jpg"), popupTabFrame, "popup/subtitle/left"_i18n, "popup/subtitle/right"_i18n);
    });

    brls::SelectListItem* layerSelectItem = new brls::SelectListItem("main/layers/title"_i18n, { "main/layers/layer1"_i18n, "main/layers/layer2"_i18n });

    brls::InputListItem* keyboardItem = new brls::InputListItem("main/keyboard/string/title"_i18n, "main/keyboard/string/default"_i18n, "main/keyboard/string/help"_i18n, "", 16);

    brls::IntegerInputListItem* keyboardNumberItem = new brls::IntegerInputListItem("main/keyboard/number/title"_i18n, 1337, "main/keyboard/number/help"_i18n, "", 10);

    testList->addView(dialogItem);
    testList->addView(notificationItem);
    testList->addView(themeItem);
    testList->addView(i18nItem);
    testList->addView(jankItem);
    testList->addView(crashItem);
    testList->addView(popupItem);
    testList->addView(keyboardItem);
    testList->addView(keyboardNumberItem);

    brls::Label* testLabel = new brls::Label(brls::LabelStyle::REGULAR, "main/more"_i18n, true);
    testList->addView(testLabel);

    brls::ListItem* actionTestItem = new brls::ListItem("main/actions/title"_i18n);
    actionTestItem->registerAction("main/actions/notify"_i18n, brls::Key::L, [] {
        brls::Application::notify("main/actions/triggered"_i18n);
        return true;
    });
    testList->addView(actionTestItem);

    brls::LayerView* testLayers = new brls::LayerView();
    brls::List* layerList1      = new brls::List();
    brls::List* layerList2      = new brls::List();

    layerList1->addView(new brls::Header("main/layers/layer1"_i18n, false));
    layerList1->addView(new brls::ListItem("main/layers/item1"_i18n));
    layerList1->addView(new brls::ListItem("main/layers/item2"_i18n));
    layerList1->addView(new brls::ListItem("main/layers/item3"_i18n));

    layerList2->addView(new brls::Header("main/layers/layer2"_i18n, false));
    layerList2->addView(new brls::ListItem("main/layers/item1"_i18n));
    layerList2->addView(new brls::ListItem("main/layers/item2"_i18n));
    layerList2->addView(new brls::ListItem("main/layers/item3"_i18n));

    testLayers->addLayer(layerList1);
    testLayers->addLayer(layerList2);

    layerSelectItem->getValueSelectedEvent()->subscribe([=](size_t selection) {
        testLayers->changeLayer(selection);
    });

    testList->addView(layerSelectItem);
    player = new VideoPlayer();
    brls::SidebarItem* bar = rootFrame->addTab("test", player);
    bar->registerAction("play", brls::Key::L, [] {
        player->play();
        return true;
    });
    rootFrame->addTab("main/tabs/first"_i18n, testList);
    rootFrame->addTab("main/tabs/second"_i18n, testLayers);
    rootFrame->addSeparator();
    rootFrame->addTab("main/tabs/third"_i18n, new brls::Rectangle(nvgRGB(255, 0, 0)));
    rootFrame->addTab("main/tabs/fourth"_i18n, new brls::Rectangle(nvgRGB(0, 255, 0)));
    // Add the root view to the stack
    // brls::AppletFrame *tf = new brls::AppletFrame(0,0);
    brls::Application::pushView(rootFrame);

    // Run the app
    while (brls::Application::mainLoop()){
    //   player->mpv_render();
    glViewport(0,0,1280,720);
    }

    // Exit
    return EXIT_SUCCESS;
}
