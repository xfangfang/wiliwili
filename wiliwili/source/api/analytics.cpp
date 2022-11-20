//
// Created by fang on 2022/11/18.
//

#include "analytics.h"

#include "cpr/cpr.h"
#include "utils/config_helper.hpp"
#include "utils/number_helper.hpp"
#include "fmt/format.h"
#include "borealis/core/logger.hpp"
#include "borealis/core/i18n.hpp"

using namespace brls::literals;

namespace analytics {

void Analytics::report(std::string event) { this->report(Event(event)); }

void Analytics::report(Event event) {
    events_mutex.lock();
    events.emplace_back(event);
    events_mutex.unlock();

    send();
}

void Analytics::send() {
    Package package;
    events_mutex.lock();
    package.events = events;
    events.clear();
    events_mutex.unlock();
    if (events.empty()) return;

    package.client_id = ProgramConfig::instance().getClientID();
    package.user_id   = ProgramConfig::instance().getUserID();
    package.timestamp_micros =
        std::to_string(wiliwili::getUnixTime() * 1000000);
    package.user_properties.insert(
        {std::make_pair("version", Property(app_version))});
    nlohmann::json content(package);
    brls::Logger::verbose("report event: {}", content.dump());

    cpr::PostCallback(
        [](cpr::Response r) {
            brls::Logger::verbose("report event: status code: {}",
                                  r.status_code);
        },
        cpr::Parameters{
            {"api_secret", GA_KEY},
            {"measurement_id", GA_ID},
        },
#ifndef VERIFY_SSL
        cpr::VerifySsl{false},
#endif
        cpr::Url{GA_URL},
        cpr::Header{{"User-Agent", "wiliwili/" + app_version},
                    {"Content-Type", "application/json"}},
        cpr::Body{content.dump()}, cpr::Timeout{4000});
}

Analytics::Analytics() {
    brls::Logger::debug("Analytics url: {}", GA_URL);
    this->app_version = APPVersion::instance().getVersionStr();
}

}  // namespace analytics