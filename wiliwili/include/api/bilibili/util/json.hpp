//
// Created by fang on 2024/3/20.
//

#pragma once

#include <nlohmann/json.hpp>

// https://github.com/nlohmann/json/issues/1163#issuecomment-843988837
// ignore null and missing fields
#undef NLOHMANN_JSON_FROM
#define NLOHMANN_JSON_FROM(v1) { auto iter = nlohmann_json_j.find(#v1); if (iter != nlohmann_json_j.end() && !iter->is_null()) iter->get_to(nlohmann_json_t.v1); }