//
// Created by fang on 2022/11/19.
//

#include <cstdlib>
#include <fmt/format.h>
#include <cpr/cpr.h>
#include <pystring.h>
#include <borealis/core/i18n.hpp>
#include <borealis/core/application.hpp>
#include <borealis/core/thread.hpp>
#include <borealis/views/dialog.hpp>

#include "utils/config_helper.hpp"
#include "utils/dialog_helper.hpp"
#include "api/bilibili/util/http.hpp"
#include "fragment/latest_update.hpp"

using namespace brls::literals;

#define STR_IMPL(x) #x
#define STR(x) STR_IMPL(x)

APPVersion::APPVersion() {
    git_commit = std::string(STR(BUILD_TAG_SHORT));
    git_tag    = std::string(STR(BUILD_TAG_VERSION));
    major      = atoi(STR(BUILD_VERSION_MAJOR));
    minor      = atoi(STR(BUILD_VERSION_MINOR));
    revision   = atoi(STR(BUILD_VERSION_REVISION));
}

std::string APPVersion::getVersionStr() {
    return fmt::format("{}.{}.{}", major, minor, revision);
}

std::string APPVersion::getPlatform() {
#ifdef IOS
    return "iOS";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(PS4)
    return "PS4";
#elif defined(__linux__)
    if (getenv("SteamDeck")) return "SteamDeck";
    return "Linux";
#elif defined(__WINRT__)
    return "UWP";
#elif defined(_WIN32)
    return "Windows";
#elif defined(__SWITCH__)
#ifdef BOREALIS_USE_DEKO3D
    return "NX-deko3d";
#else
    return "NX";
#endif
#elif defined(__PSV__)
    return "PSVita";
#else
    return "Unknown";
#endif
}

std::string APPVersion::getPackageName() {
    return std::string{STR(BUILD_PACKAGE_NAME)};
}

bool APPVersion::needUpdate(std::string latestVersion) {
    if (latestVersion.length() < 5) brls::Application::quit();
    if (latestVersion[0] == 'v')
        latestVersion = latestVersion.substr(1, latestVersion.length() - 1);
    std::vector<std::string> v;
    pystring::split(latestVersion, v, ".");
    if (v.size() < 3) {
        brls::Logger::error("Cannot parse version info");
        return false;
    }
    if (atoi(v[0].c_str()) > major) return true;
    if (atoi(v[1].c_str()) > minor) return true;
    if (atoi(v[2].c_str()) > revision) return true;
    return false;
}

void APPVersion::checkUpdate(int delay, bool showUpToDateDialog) {
    brls::Threading::delay(delay, [showUpToDateDialog]() {
        std::string url = ProgramConfig::instance().getSettingItem(
            SettingItem::CUSTOM_UPDATE_API, APPVersion::RELEASE_API);

        cpr::GetCallback(
            [showUpToDateDialog](cpr::Response r) {
                try {
                    nlohmann::json res = nlohmann::json::parse(r.text);
                    auto info = res.get<ReleaseNote>();
                    if (!APPVersion::instance().needUpdate(info.tag_name)) {
                        brls::Logger::info("App is up to date");
                        if (showUpToDateDialog) {
                            brls::sync([]() {
                                DialogHelper::showDialog(
                                    "wiliwili/setting/tools/others/up2date"_i18n);
                            });
                        }
                        return;
                    }
                    brls::sync([info]() {
                        auto container = new LatestUpdate(info);
                        auto dialog = new brls::Dialog((brls::Box*)container);
                        dialog->open();
                    });
                } catch (const std::exception& e) {
                    brls::Logger::error("check update failed: {} {} {}",
                                        r.status_code, r.text.c_str(),
                                        e.what());
                }
            },
            bilibili::HTTP::VERIFY,
            bilibili::HTTP::PROXIES,
            cpr::Url{url}, cpr::Timeout{10000});
    });
}