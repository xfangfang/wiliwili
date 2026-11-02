/**

██     ██ ██ ██      ██ ██     ██ ██ ██      ██
██     ██ ██ ██      ██ ██     ██ ██ ██      ██
██  █  ██ ██ ██      ██ ██  █  ██ ██ ██      ██
██ ███ ██ ██ ██      ██ ██ ███ ██ ██ ██      ██
 ███ ███  ██ ███████ ██  ███ ███  ██ ███████ ██

 Licensed under the GPL-3.0 license
*/

#include <borealis.hpp>

#include "utils/config_helper.hpp"
#include "utils/stage.hpp"
#include "utils/activity_helper.hpp"
#include "utils/ps5_decoder.hpp"
#include "view/mpv_core.hpp"

#if defined(PS5) && defined(BOREALIS_USE_VULKAN)
#include <borealis/platforms/driver/vulkan.hpp>
#endif

#ifdef IOS
#include <SDL2/SDL_main.h>
#endif

#ifdef PS5
#include <csignal>
// The console unloads a title through its system service.
extern "C" int sceSystemServiceLoadExec(const char* path, const char* argv[]);
// The handle the platform kept for the console's own log (runtime_shims.c).
extern "C" int wiliwili_console_log;
// Writes how far the start-up got, before the log can be written (runtime_shims.c).
extern "C" void wiliwili_mark(const char *stage);
// Microsecond sleep used by the spin-loops that keep the process alive.
extern "C" int sceKernelUsleep(unsigned microseconds);
// Shows on-screen system notification (runtime_shims.c).
extern "C" void wiliwili_ps5_notify(const char *message);
// Ends the process without the exit handlers, for when the system service
// that unloads a title refused to take it.
extern "C" void _exit(int status);
#endif

int main(int argc, char* argv[]) {
#ifdef PS5
    wiliwili_mark("main");
    signal(SIGPIPE, SIG_IGN);
#endif
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "-d") == 0) {
            brls::Logger::setLogLevel(brls::LogLevel::LOG_DEBUG);
        } else if (std::strcmp(argv[i], "-v") == 0) {
            brls::Application::enableDebuggingView(true);
        } else if (std::strcmp(argv[i], "-t") == 0) {
            MPVCore::TERMINAL = true;
        } else if (std::strcmp(argv[i], "-o") == 0) {
            const char* path = (i + 1 < argc) ? argv[++i] : "wiliwili.log";
            brls::Logger::setLogOutput(std::fopen(path, "w+"));
        }
    }

#ifdef PS5
    // The console title cannot take command line options; keep a debug log to
    // diagnose the remaining network layers through the crash reporter fetch.
    brls::Logger::setLogLevel(brls::LogLevel::LOG_DEBUG);
    // The log is a file on the console's storage and nothing removes it: bound
    // it so a long session cannot fill the drive with debug lines.
    brls::Logger::setLogFileLimit(16 * 1024 * 1024);
    // Warnings and errors go to the console's own log as well, so they are
    // visible while the title runs (port 3232 of the console) and are not lost
    // when the title stops without reaching the file.
    brls::Logger::setConsoleLogHandle(wiliwili_console_log);
    // The queue the console's hardware video decoder runs on has to be made
    // before the application asks for full access, and the crash reporter has
    // to be there to say what went wrong.
    wiliwili_mark("decoder prepare");
    ps5decoder::wiliwili_ps5_decoder_prepare();
#endif

    try {
        // Load cookies and settings
#ifdef PS5
        wiliwili_mark("config init");
#endif
        ProgramConfig::instance().init();
#ifdef PS5
        wiliwili_mark("config done");
#endif

#ifdef PS5
        // The console's hardware video decoder, with the test streams that ship
        // with the application. It is off by default: the chip gives up on work
        // it does not like, and that must not disturb the application.
        if (ProgramConfig::instance().getBoolOption(SettingItem::PS5_HWDEC_TEST)) {
            ps5decoder::Decoder::selfTest();
        }
#endif

        // Init the app and i18n
#ifdef PS5
        // Ask for a ten bit PQ swapchain when the user turned physical HDR on,
        // before the platform creates one: the display only switches to HDR10
        // if the framebuffers are registered as BGR10A2 BT.2100 PQ.
#if defined(BOREALIS_USE_VULKAN)
        brls::VulkanContext::HDR_SWAPCHAIN = MPVCore::HDR_OUTPUT;
#endif
        wiliwili_mark("app init");
#endif
        if (!brls::Application::init()) {
            brls::Logger::error("Unable to init application");
#ifdef PS5
            // On PS5 returning from main() terminates the process before the log
            // is flushed and before crash-last.txt can be written, so the user
            // gets CE-108255-1 with no diagnostic file.  Spin instead so the log
            // stays readable (the same pattern as catchReturnFromMain in
            // scripts/ps5/title/runtime_shims.c).
            wiliwili_mark("init failed");
            wiliwili_ps5_notify("wiliwili: Application::init() failed! Check logs.");
            fflush(nullptr);
            for (;;)
                sceKernelUsleep(100000);
#else
            return EXIT_FAILURE;
#endif
        }
#ifdef PS5
        wiliwili_mark("app inited");
#endif

#ifdef PS5
        // A watch over the main loop: a title that stops answering does not
        // write anything itself, so the watch says where it stopped.
        stage::startWatch();
        brls::Application::getRunLoopEvent()->subscribe([]() {
            stage::Marker marker("run loop");
            stage::tick();
        });
#endif

        // Return directly to the desktop when closing the application (only for NX)
        brls::Application::getPlatform()->exitToHomeMode(true);

#ifdef PS5
        wiliwili_mark("create window");
#endif
        brls::Application::createWindow("wiliwili");
        brls::Logger::info("createWindow done");
#ifdef PS5
        wiliwili_mark("window created");
#endif

        // Register custom view\theme\style
#ifdef PS5
        wiliwili_mark("custom views");
#endif
        Register::initCustomView();
        Register::initCustomTheme();
        Register::initCustomStyle();

        brls::Application::getPlatform()->disableScreenDimming(false);

        if (brls::Application::getPlatform()->isApplicationMode()) {
#ifdef PS5
            wiliwili_mark("main intent");
#endif
            Intent::openMain();
        } else {
            Intent::openHint();
        }

        GA("open_app", {{"version", APPVersion::instance().getVersionStr()},
                        {"language", brls::Application::getLocale()},
                        {"window", fmt::format("{}x{}", brls::Application::windowWidth, brls::Application::windowHeight)}})
        APPVersion::instance().checkUpdate();

        // Run the app
        // brls::Application::setLimitedFPS(60);
#ifdef PS5
        wiliwili_mark("main loop");
#endif
        while (brls::Application::mainLoop()) {
        }

        brls::Logger::info("mainLoop done");

#ifdef PS5
        brls::Logger::info("leaving the title");
        fflush(nullptr);
        wiliwili_mark("leaving title");

        // The console unloads a title through its system service. Without this
        // call the title's window stays on the screen and the console shows a
        // black frame instead of the home screen - the console's own dialog
        // asks whether to close the program, and then nothing happens.
        int rc = sceSystemServiceLoadExec("exit", nullptr);
        // Reached only when the call failed: say so, and end the process the
        // short way rather than leaving a window behind that nothing answers.
        brls::Logger::error("sceSystemServiceLoadExec exit returned {:#x}", rc);
        fflush(nullptr);
        _exit(0);
#else
        // Cleanup curl and Check whether restart is required
        ProgramConfig::instance().exit(argv);
#endif
    } catch (const std::exception& e) {
        brls::Logger::error("Unhandled exception in main: {}", e.what());
#ifdef PS5
        char toast[256];
        std::snprintf(toast, sizeof(toast), "wiliwili unhandled:\n%s", e.what());
        wiliwili_ps5_notify(toast);
        fflush(nullptr);
        for (;;)
            sceKernelUsleep(100000);
#endif
        return EXIT_FAILURE;
    } catch (...) {
        brls::Logger::error("Unhandled unknown exception in main");
#ifdef PS5
        wiliwili_ps5_notify("wiliwili: unhandled unknown exception");
        fflush(nullptr);
        for (;;)
            sceKernelUsleep(100000);
#endif
        return EXIT_FAILURE;
    }

    // Exit
    return EXIT_SUCCESS;
}

#ifdef __WINRT__
#include <borealis/core/main.hpp>
#endif
