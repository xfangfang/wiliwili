//
// Created by fang on 2022/11/18.
//

#include "analytics.h"

#include <utility>
#include <cstdlib>
#include <fmt/format.h>
#include <cpr/cpr.h>
#include "utils/config_helper.hpp"
#include "utils/number_helper.hpp"
#include "api/bilibili/util/http.hpp"
#include <borealis/core/thread.hpp>
#include <borealis/core/logger.hpp>
#include <borealis/core/i18n.hpp>

using namespace brls::literals;

namespace analytics {

void Analytics::report(const std::string& event) { this->report(Event{event}); }

void Analytics::report(const std::string& event, const Params& params) { this->report(Event{event, params}); }

void Analytics::report(const Event& event) {
    events_mutex.lock();
    events.emplace_back(event);
    events_mutex.unlock();
}

void Analytics::send() {
    Package package;
    events_mutex.lock();
    if (events.size() > REPORT_MAX_NUM) {
        package.events.insert(package.events.end(), events.begin(), events.begin() + REPORT_MAX_NUM);
        events.erase(events.begin(), events.begin() + REPORT_MAX_NUM);
    } else {
        package.events = events;
        events.clear();
    }
    events_mutex.unlock();
    if (package.events.empty()) return;

    package.client_id        = this->client_id;
    package.user_id          = ProgramConfig::instance().getUserID();
    package.timestamp_micros = std::to_string(wiliwili::getUnixTime()) + "000000";
    package.insertUserProperties({
        {"git", APPVersion::instance().git_tag},
        {"platform", APPVersion::instance().getPlatform()},
        {"device", APPVersion::instance().getPlatform()},
    });
    for (auto& i : package.events) {
        i.params["engagement_time_msec"] = 100;
        i.params["session_id"]           = this->client_id;
        i.params["git"]                  = APPVersion::instance().git_tag;
        i.params["platform"]             = APPVersion::instance().getPlatform();
        i.params["user"]                 = ProgramConfig::instance().getUserID();
    }
    nlohmann::json content(package);
    auto content_str = content.dump();
    brls::Logger::debug("report event: {}", content_str);

    cpr::PostCallback(
        [](const cpr::Response& r) {
            if (r.status_code != 204) {
                brls::Logger::error("report event error: {} {}", r.status_code, r.error.message);
            }
        },
        cpr::Parameters{
            {"api_secret", GA_KEY},
            {"measurement_id", GA_ID},
        },
        bilibili::HTTP::VERIFY, bilibili::HTTP::PROXIES, cpr::Url{GA_URL},
        cpr::Header{{"User-Agent", "wiliwili/" + app_version}, {"Content-Type", "application/json"}},
        cpr::Cookies{{"_ga", client_id}}, cpr::Body{content_str}, cpr::Timeout{4000});
}

Analytics::Analytics() {
    brls::Logger::debug("Analytics url: {}", GA_URL);
    this->app_version = APPVersion::instance().getVersionStr();
    this->client_id   = "GA1.3." + ProgramConfig::instance().getClientID();

    reportTimer.setCallback([]() { brls::Threading::async([]() { Analytics::instance().send(); }); });
    reportTimer.start(10000);
}

Analytics::~Analytics() { reportTimer.stop(); }

}  // namespace analytics