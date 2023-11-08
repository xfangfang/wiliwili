//
// Created by fang on 2022/11/18.
//

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

#include "borealis/core/timer.hpp"
#include "borealis/core/singleton.hpp"
#include "bilibili/result/analytics_result.h"

namespace analytics {

#define STR_IMPL(x) #x
#define STR(x) STR_IMPL(x)

#ifdef ANALYTICS
// custom Google Analytics id/key
const std::string GA_ID  = STR(ANALYTICS_ID);
const std::string GA_KEY = STR(ANALYTICS_KEY);
const std::string GA_URL = "https://www.google-analytics.com/mp/collect";
#else
// default Google Analytics id/key
const std::string GA_ID  = "G-YE1PE9VDBY";
const std::string GA_KEY = "fmMCjnX1Sam815PDdrOPQA";
const std::string GA_URL = "https://www.google-analytics.com/mp/collect";
#endif

#ifdef NO_GA
#define GA(a, ...) void(a);
#define GA_SEND void();
#else
#define GA(a, ...) analytics::Analytics::instance().report(a, ##__VA_ARGS__);
#define GA_SEND analytics::Analytics::instance().send();
#endif /* NO_GA */

class Event;
class Package;

class Analytics : public brls::Singleton<Analytics> {
public:
    const size_t REPORT_MAX_NUM = 25;
    std::vector<Event> events;
    std::mutex events_mutex;
    brls::RepeatingTimer reportTimer;
    std::string app_version;
    std::string client_id;

    void report(const Event& event);

    void report(const std::string& event);

    void report(const std::string& event, const Params& params);

    void send();

    Analytics();
    ~Analytics();
};

}  // namespace analytics