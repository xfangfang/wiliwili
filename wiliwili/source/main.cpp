

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <borealis.hpp>

//#define NO_GA
#include "analytics.h"

#include "utils/config_helper.hpp"
#include "utils/thread_helper.hpp"
#include "activity/main_activity.hpp"
#include "activity/hint_activity.hpp"
//#include "activity/setting_activity.hpp"
//#include "activity/splash_activity.hpp"
//#include "activity/search_activity.hpp"
//#include "activity/pgc_index_activity.hpp"
//#include "activity/player_activity.hpp"
//#include "activity/live_player_activity.hpp"

//#define DISK_LOG

using namespace brls::literals;  // for _i18n

int main(int argc, char* argv[]) {
    // Set min_threads and max_threads of http thread pool
    curl_global_init(CURL_GLOBAL_DEFAULT);
    cpr::async::startup(THREAD_POOL_MIN_THREAD_NUM, THREAD_POOL_MAX_THREAD_NUM,
                        std::chrono::milliseconds(5000));

    // Set log level
    brls::Logger::setLogLevel(brls::LogLevel::LOG_INFO);
    brls::Logger::debug("std::thread::hardware_concurrency(): {}",
                        std::thread::hardware_concurrency());

#ifdef DISK_LOG
    std::filesystem::create_directories(
        ProgramConfig::instance().getConfigDir());
    std::ofstream logFile(ProgramConfig::instance().getConfigDir() +
                          "/log.txt");
    brls::Logger::getLogEvent()->subscribe(
        [&logFile](std::string log) { logFile << log << std::endl; });
#endif

    // Load cookies and settings from disk
    ProgramConfig::instance().init();

    // Init the app and i18n
    if (!brls::Application::init()) {
        brls::Logger::error("Unable to init application");
        return EXIT_FAILURE;
    }
    //    brls::Application::getPlatform()->forceEnableGamePlayRecording();
    brls::Application::getPlatform()->exitToHomeMode(true);

    brls::Application::createWindow("wiliwili");

    // Have the application register an action on every activity that will quit when you press BUTTON_START
    brls::Application::setGlobalQuit(false);

    brls::Logger::info("createWindow done");

    // Register custom view\theme\style
    Register::initCustomView();
    Register::initCustomTheme();
    Register::initCustomStyle();

    if (brls::Application::getPlatform()->isApplicationMode()) {
        brls::Application::pushActivity(new MainActivity());
        // Use these activities to debug
        //        brls::Application::pushActivity(new PlayerActivity("BV18W4y1q72C"));  // wiliwili介绍
        //        brls::Application::pushActivity(new PlayerActivity("BV1dx411c7Av"));  // flv拼接视频
        //        brls::Application::pushActivity(new PlayerActivity("BV15z4y1Z734"));  // 4K HDR 视频
        //        brls::Application::pushActivity(new PlayerActivity("BV1PN4y1G7u2"));  // up主视频自动跳转番剧
        //        brls::Application::pushActivity(new PlayerActivity("BV1sK411s7zq"));  // 多P视频测试
        //        brls::Application::pushActivity(new PlayerActivity("BV1A44y1u7PF"), brls::TransitionAnimation::NONE); // 测试FFMPEG在switch上的bug（加载时间过长）
        //        brls::Application::pushActivity(new PlayerActivity("BV1U3411c7Qx"), brls::TransitionAnimation::NONE); // 测试长标题
        //        brls::Application::pushActivity(new PlayerActivity("BV1fG411W7Px"), brls::TransitionAnimation::NONE); // 测试弹幕
        //        brls::Application::pushActivity(new SearchActivity("哈利波特")); // 测试搜索影片
        //        brls::Application::pushActivity(new SplashActivity()); // 首屏页面（暂时未使用）
        //        brls::Application::pushActivity(new HintActivity());   // 应用开启教程页面
        //        brls::Application::pushActivity(new PGCIndexActivity("/page/home/pgc/more?type=2&index_type=2&area=2&order=2&season_status=-1&season_status=3,6")); // 影片分类索引
        //        brls::Application::pushActivity(new SettingActivity());     //  设置页面
        //        brls::Application::pushActivity(new LiveActivity(1942240)); // 直播页面
    } else {
        brls::Application::pushActivity(new HintActivity());
    }

    GA("open_app")
    APPVersion::instance().checkUpdate();

    // Run the app
    while (brls::Application::mainLoop()) {
    }

    brls::Logger::debug("main loop done");
    cpr::async::cleanup();
    curl_global_cleanup();

#ifdef DISK_LOG
    logFile.close();
#endif

    // Check whether restart is required
    ProgramConfig::instance().checkRestart(argv);

    // Exit
    return EXIT_SUCCESS;
}
