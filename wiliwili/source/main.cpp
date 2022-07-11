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

#include "bilibili.h"
#include "utils/config_helper.hpp"


#include "view/auto_tab_frame.hpp"
#include "view/video_view.hpp"
#include "view/net_image.hpp"
#include "view/user_info.hpp"
#include "view/text_box.hpp"
#include "view/qr_image.hpp"
#include "view/svg_image.hpp"
#include "view/up_user_small.hpp"
#include "view/recycling_grid.hpp"
#include "view/grid_dropdown.hpp"

#include "fragment/home_tab.hpp"
#include "fragment/dynamic_tab.hpp"
#include "fragment/mine_tab.hpp"
#include "fragment/home_recommends.hpp"
#include "fragment/home_hots_all.hpp"
#include "fragment/home_hots_history.hpp"
#include "fragment/home_hots_weekly.hpp"
#include "fragment/home_hots_rank.hpp"
#include "fragment/home_hots.hpp"


#include "activity/main_activity.hpp"

using namespace brls::literals; // for _i18n

ProgramConfig ConfigHelper::programConfig = ConfigHelper::readProgramConf();

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
    cpr::async::startup(1, 2);

    // Set log level
    // We recommend to use INFO for real apps
    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);

    brls::Logger::info("Application::init()");

    // Load Cookies for bilibili
    Cookie cookie = ConfigHelper::programConfig.getCookie();
    for(auto c : cookie){
        brls::Logger::error("cookie: {}:{}", c.first, c.second);
    }
    // set bilibili cookie and cookie update callback
    bilibili::BilibiliClient::init(cookie,[](Cookie newCookie){
        ProgramConfig config;
        config.setCookie(newCookie);
        ConfigHelper::saveProgramConf(config);
    });

    // Init the app and i18n
    if (!brls::Application::init())
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    brls::Logger::info("Application::init() done");

    brls::Application::createWindow("wiliwili/title"_i18n);

    brls::Logger::info("Application::createWindow() done");

    //todo: Add splash

    // Have the application register an action on every activity that will quit when you press BUTTON_START
    brls::Application::setGlobalQuit(false);

    // Register extended views
    brls::Application::registerXMLView("AutoTabFrame", AutoTabFrame::create);
    brls::Application::registerXMLView("RecyclingGrid", RecyclingGrid::create);
    brls::Application::registerXMLView("VideoView", VideoView::create);
    brls::Application::registerXMLView("NetImage", NetImage::create);
    brls::Application::registerXMLView("QRImage", QRImage::create);
    brls::Application::registerXMLView("SVGImage", SVGImage::create);
    brls::Application::registerXMLView("TextBox", TextBox::create);

    // Register custom views
    brls::Application::registerXMLView("UserInfoView", UserInfoView::create);
    brls::Application::registerXMLView("UpUserSmall", UpUserSmall::create);

    // Register fragments
    brls::Application::registerXMLView("HomeTab", HomeTab::create);
    brls::Application::registerXMLView("DynamicTab", DynamicTab::create);
    brls::Application::registerXMLView("MineTab", MineTab::create);
    brls::Application::registerXMLView("HomeRecommends", HomeRecommends::create);
    brls::Application::registerXMLView("HomeHotsAll", HomeHotsAll::create);
    brls::Application::registerXMLView("HomeHotsHistory", HomeHotsHistory::create);
    brls::Application::registerXMLView("HomeHotsWeekly", HomeHotsWeekly::create);
    brls::Application::registerXMLView("HomeHotsRank", HomeHotsRank::create);
    brls::Application::registerXMLView("HomeHots", HomeHots::create);


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
    brls::Application::pushActivity(main);

    // Run the app
    while (brls::Application::mainLoop()){
//        std::this_thread::sleep_for(std::chrono::microseconds(33));
    }


    #ifdef __SWITCH__
//        twiliExit();
    #endif

    // Exit
    return EXIT_SUCCESS;
}
