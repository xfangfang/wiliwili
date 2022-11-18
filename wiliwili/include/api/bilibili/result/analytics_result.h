//
// Created by fang on 2022/11/18.
//

#pragma once

#include "nlohmann/json.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace analytics {

typedef std::unordered_map<std::string, std::string> Params;

class Event {
public:
    std::string name;
    Params params;
    Event() = default;
    Event(std::string name, Params params = {}) : name(name), params(params) {}
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Event, name, params);

class Property {
public:
    std::string value;
    Property() = default;
    Property(std::string value) : value(value) {}
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Property, value);

class Package {
public:
    std::string client_id, user_id, timestamp_micros;
    std::unordered_map<std::string, Property> user_properties;
    std::vector<Event> events;

    Package() {
#ifdef __APPLE__
        user_properties.insert(std::make_pair("platform", Property("macOS")));
#endif
#ifdef __linux__
        user_properties.insert(std::make_pair("platform", Property("Linux")));
#endif
#ifdef _WIN32
        user_properties.insert(std::make_pair("platform", Property("Windows")));
#endif
#ifdef __SWITCH__
        user_properties.insert(std::make_pair("platform", Property("NX")));
#endif
    }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Package, client_id, user_id, user_properties,
                                   events, timestamp_micros);

}  // namespace analytics