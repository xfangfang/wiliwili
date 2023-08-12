//
// Created by fang on 2022/11/18.
//

#include "analytics.h"

#include <utility>

#include "cpr/cpr.h"
#include "utils/config_helper.hpp"
#include "utils/number_helper.hpp"
#include "fmt/format.h"
#include "borealis/core/thread.hpp"
#include "borealis/core/logger.hpp"
#include "borealis/core/i18n.hpp"

using namespace brls::literals;

namespace analytics {

void Analytics::report(std::string event) {
    this->report(Event(std::move(event)));
}

void Analytics::report(std::string event, Params params) {
    Event e{event};
    e.params = params;
    this->report(e);
}

void Analytics::report(const Event& event) {
    events_mutex.lock();
    events.emplace_back(event);
    events_mutex.unlock();
}

void Analytics::send() {
    Package package;
    events_mutex.lock();
    if (events.size() > REPORT_MAX_NUM) {
        package.events.insert(package.events.end(), events.begin(),
                              events.begin() + REPORT_MAX_NUM);
        events.erase(events.begin(), events.begin() + REPORT_MAX_NUM);
    } else {
        package.events = events;
        events.clear();
    }
    events_mutex.unlock();
    if (package.events.empty()) return;

    package.client_id = this->client_id;
    package.user_id   = ProgramConfig::instance().getUserID();
    package.timestamp_micros = std::to_string(wiliwili::getUnixTime()) + "000000";
    nlohmann::json content(package);
    brls::Logger::verbose("report event: {}", content.dump());

    cpr::PostCallback(
        [](const cpr::Response& r) {
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
        cpr::Cookies{{"_ga", client_id}}, cpr::Body{content.dump()},
        cpr::Timeout{4000});
}

Analytics::Analytics() {
    brls::Logger::debug("Analytics url: {}", GA_URL);
    this->app_version = APPVersion::instance().getVersionStr();
    this->client_id   = "GA1.3." + ProgramConfig::instance().getClientID();

    reportTimer.setCallback([]() {
        brls::Threading::async([]() { Analytics::instance().send(); });
    });
    reportTimer.start(10000);
}

Analytics::~Analytics() { reportTimer.stop(); }

}  // namespace analytics