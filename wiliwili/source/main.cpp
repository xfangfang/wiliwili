

// Switch include only necessary for demo videos recording
#ifdef __SWITCH__
#include <switch.h>
//#include <twili.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <borealis.hpp>
#include <string>

#include "utils/config_helper.hpp"
#include "activity/main_activity.hpp"
#include "activity/search_activity.hpp"
//#include "activity/splash_activity.hpp"
#include "activity/player_activity.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <unistd.h>

using namespace brls::literals; // for _i18n

int main(int argc, char* argv[])
{
    // Enable recording for Twitter memes
#ifdef __SWITCH__

    appletInitializeGamePlayRecording();
//    appletSetWirelessPriorityMode(AppletWirelessPriorityMode_OptimizedForWlan)
//    twiliInitialize();
//    twiliBindStdio();
#endif
    // Set min_threads and max_threads of http thread pool
    curl_global_init(CURL_GLOBAL_DEFAULT);

    cpr::async::startup(1, THREAD_POOL_MAX_THREAD_NUM);
//    cpr::async::startup(1, 1);

    // Set log level
    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);

    brls::Logger::debug("std::thread::hardware_concurrency(): {}", std::thread::hardware_concurrency());



//    std::filesystem::create_directories(ConfigHelper::getConfigDir());
//    std::ofstream logFile(ConfigHelper::getConfigDir() + "/log.txt");
//    brls::Logger::getLogEvent()->subscribe([&logFile](std::string log) {
//        logFile << log << std::endl;
//    });


    brls::Logger::info("Application::init()");

    // Init the app and i18n
    if (!brls::Application::init())
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    brls::Logger::info("Application::init() done");

    brls::Application::createWindow("wiliwili/title"_i18n);
    //todo: Add splash

    brls::Logger::info("Application::createWindow() done");

    // Have the application register an action on every activity that will quit when you press BUTTON_START
    brls::Application::setGlobalQuit(false);

    // Load Cookies for bilibili from disk
    ConfigHelper::init();

    // Register custom view\theme\style
    Register::initCustomView();
    Register::initCustomTheme();
    Register::initCustomStyle();


    // Create and push the main activity to the stack
    brls::Application::pushActivity(new MainActivity());
//    brls::Application::pushActivity(new SplashActivity());
//    brls::Application::pushActivity(new PlayerActivity("BV1A44y1u7PF"));

    // Run the app
    while (brls::Application::mainLoop()){
//        std::this_thread::sleep_for(std::chrono::microseconds(33));
    }

    brls::Logger::debug("main loop done");
    cpr::async::cleanup();
    curl_global_cleanup();
//    logFile.close();

    #ifdef __SWITCH__
//        twiliExit();
    #endif

    // Exit
    return EXIT_SUCCESS;
}
