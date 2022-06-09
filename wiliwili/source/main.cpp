/*
    Copyright 2020-2021 natinusala
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

// Switch include only necessary for demo videos recording
#ifdef __SWITCH__
#include <switch.h>
#include <twili.h>
#endif

#include <cstdio>
#include <cstdlib>

#include <borealis.hpp>
#include <string>

#include "captioned_image.hpp"
#include "components_tab.hpp"
#include "activity/main_activity.hpp"
#include "pokemon_view.hpp"
#include "recycling_list_tab.hpp"
#include "settings_tab.hpp"
#include "auto_tab_frame.hpp"
#include "view/video_view.hpp"
#include "video_list_tab.hpp"
#include "view/net_image.hpp"
#include "view/user_info.hpp"
#include "view/video_grid.hpp"
#include "view/video_card.hpp"
#include "view/text_box.hpp"
#include "view/qr_image.hpp"
#include "view/svg_image.hpp"
#include "view/up_user_small.hpp"

#include "fragment/home_tab.hpp"
#include "fragment/dynamic_tab.hpp"
#include "fragment/mine_tab.hpp"


#include "activity/player_activity.hpp"


using namespace brls::literals; // for _i18n

int main(int argc, char* argv[])
{
    // Enable recording for Twitter memes
#ifdef __SWITCH__
    appletInitializeGamePlayRecording();
//    appletSetWirelessPriorityMode(AppletWirelessPriorityMode_OptimizedForWlan)
//    twiliInitialize();
//    twiliBindStdio();

    cpr::async::startup(1, 2);
#endif

    // Set min_threads and max_threads of cpr thread pool
    cpr::async::startup(1, 1);

    // Set log level
    // We recommend to use INFO for real apps
    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);

    brls::Logger::info("Application::init()");

    // Init the app and i18n
    if (!brls::Application::init())
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    brls::Logger::info("Application::init() done");

    brls::Application::createWindow("demo/title"_i18n);

    brls::Logger::info("Application::createWindow() done");

    //todo: Add splash

    // Have the application register an action on every activity that will quit when you press BUTTON_START
    brls::Application::setGlobalQuit(false);

    // Register custom views (including tabs, which are views)
    brls::Application::registerXMLView("CaptionedImage", CaptionedImage::create);
    brls::Application::registerXMLView("RecyclingListTab", RecyclingListTab::create);
    brls::Application::registerXMLView("ComponentsTab", ComponentsTab::create);
    brls::Application::registerXMLView("PokemonView", PokemonView::create);
    brls::Application::registerXMLView("SettingsTab", SettingsTab::create);
    brls::Application::registerXMLView("AutoTabFrame", AutoTabFrame::create);
    brls::Application::registerXMLView("VideoView", VideoView::create);
    brls::Application::registerXMLView("VideoListTab", VideoListTab::create);
    brls::Application::registerXMLView("NetImage", NetImage::create);
    brls::Application::registerXMLView("QRImage", QRImage::create);
    brls::Application::registerXMLView("SVGImage", SVGImage::create);
    brls::Application::registerXMLView("UserInfoView", UserInfoView::create);
    brls::Application::registerXMLView("VideoGrid", VideoGrid::create);
    brls::Application::registerXMLView("VideoCardView", VideoCardView::create);
    brls::Application::registerXMLView("TextBox", TextBox::create);
    brls::Application::registerXMLView("UpUserSmall", UpUserSmall::create);

    // Register fragments
    brls::Application::registerXMLView("HomeTab", HomeTab::create);
    brls::Application::registerXMLView("DynamicTab", DynamicTab::create);
    brls::Application::registerXMLView("MineTab", MineTab::create);

    // Add custom values to the theme
    brls::getLightTheme().addColor("captioned_image/caption", nvgRGB(2, 176, 183));
    brls::getDarkTheme().addColor("captioned_image/caption", nvgRGB(51, 186, 227));

    // Add custom values to the style
    brls::getStyle().addMetric("about/padding_top_bottom", 50);
    brls::getStyle().addMetric("about/padding_sides", 75);
    brls::getStyle().addMetric("about/description_margin", 50);

//    brls::Application::enableDebuggingView(true);

    auto* main = new MainActivity();
    // Create and push the main activity to the stack
    brls::Logger::error("pushActivity 1");

    brls::Application::pushActivity(main);

    brls::Logger::error("pushActivity 2");


//    auto* video = new VideoDetailActivity("BV1Z44y1b747");
//    brls::Application::pushActivity(video);

    // Run the app
    while (brls::Application::mainLoop())
        ;


    #ifdef __SWITCH__
//        twiliExit();
    #endif

    // Exit
    return EXIT_SUCCESS;
}
