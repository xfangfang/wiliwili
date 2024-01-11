//
// Created by fang on 2022/11/18.
//

#pragma once

#include "nlohmann/json.hpp"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace analytics {

typedef nlohmann::json Params;

class Event {
public:
    std::string name;
    Params params;
    Event() = default;
    explicit Event(std::string name, Params params = {}) : name(std::move(name)), params(std::move(params)) {}
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Event, name, params);

class Property {
public:
    std::string value;
    Property() = default;
    explicit Property(std::string value) : value(std::move(value)) {}
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Property, value);

class Package {
public:
    std::string client_id, user_id, timestamp_micros;
    std::unordered_map<std::string, Property> user_properties;
    std::vector<Event> events;

    void insertUserProperties(std::unordered_map<std::string, std::string> prop) {
        for (auto& i : prop) {
            user_properties.insert({i.first, Property(i.second)});
        }
    }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Package, client_id, user_id, user_properties, events, timestamp_micros);

}  // namespace analytics