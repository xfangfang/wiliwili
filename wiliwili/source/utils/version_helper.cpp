//
// Created by fang on 2022/11/19.
//

#include <stdlib.h>
#include "fmt/format.h"
#include "cpr/cpr.h"
#include "borealis.hpp"
#include "pystring.h"
#include "utils/config_helper.hpp"
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

bool APPVersion::needUpdate(std::string latestVersion) {
    if (latestVersion.length() < 5) brls::Application::quit();
    if (latestVersion[0] == 'v')
        latestVersion = latestVersion.substr(1, latestVersion.length() - 1);
    std::vector<std::string> v = pystring::split(latestVersion, ".");
    if (v.size() < 3) {
        brls::Logger::error("Cannot parse version info");
        return false;
    }
    if (major > atoi(v[0].c_str())) return false;
    if (minor > atoi(v[1].c_str())) return false;
    if (revision > atoi(v[2].c_str())) return false;
    return true;
}

void APPVersion::checkUpdate(int delay) {
    brls::Threading::delay(delay, []() {
        std::string url = ProgramConfig::instance().getSettingItem(
            SettingItem::CUSTOM_UPDATE_API, APPVersion::RELEASE_API);

        cpr::GetCallback(
            [](cpr::Response r) {
                try {
                    nlohmann::json res = nlohmann::json::parse(r.text);
                    std::string latestVersion =
                        res.at("tag_name").get<std::string>();
                    if (!APPVersion::instance().needUpdate(latestVersion)) {
                        brls::Logger::info("App is up to date");
                        return;
                    }

                    brls::sync([res]() {
                        auto updateName    = res.at("name").get<std::string>();
                        auto updateContent = res.at("body").get<std::string>();
                        updateContent =
                            pystring::replace(updateContent, "\r\n", "\n");
                        auto author_name =
                            res.at("author").at("login").get<std::string>();
                        auto author_avatar = res.at("author")
                                                 .at("avatar_url")
                                                 .get<std::string>();
                        auto publish_date =
                            res.at("published_at").get<std::string>();

                        auto container = new LatestUpdate(
                            updateName, updateContent, author_name,
                            author_avatar, publish_date);
                        auto dialog = new brls::Dialog((brls::Box*)container);
                        dialog->open();
                    });
                } catch (const std::exception& e) {
                    brls::Logger::error("check update failed: {} {} {}",
                                        r.status_code, r.text.c_str(),
                                        e.what());
                }
            },
            cpr::Url{url}, cpr::Timeout{10000});
    });
}