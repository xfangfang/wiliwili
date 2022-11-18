//
// Created by fang on 2022/11/18.
//

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

#include "utils/singleton.hpp"
#include "bilibili/result/analytics_result.h"

namespace analytics {

#define STR_IMPL(x) #x
#define STR(x) STR_IMPL(x)

#ifdef ANALYTICS
const std::string GA_ID  = STR(ANALYTICS_ID);
const std::string GA_KEY = STR(ANALYTICS_KEY);
const std::string GA_URL = "https://www.google-analytics.com/mp/collect";
#else
const std::string GA_ID  = "";
const std::string GA_KEY = "";
const std::string GA_URL = "http://httpbin.org/post";
#endif

#ifdef ANALYTICS
#ifdef NO_GA
#define GA(a) void(a);
#else
#define GA(a) analytics::Analytics::instance().report(a);
#endif /* NO_GA */
#else
#define GA(a) void(a);
#endif /* ANALYTICS */

class Event;
class Package;

class Analytics : public Singleton<Analytics> {
public:
    std::vector<Event> events;
    std::mutex events_mutex;
    std::string app_version = "";

    void report(Event event);

    void report(std::string event);

    void send();

    Analytics();
};

}  // namespace analytics