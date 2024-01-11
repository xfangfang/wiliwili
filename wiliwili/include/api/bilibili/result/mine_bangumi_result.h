//
// Created by fang on 2023/1/18.
//

#pragma once

#include <vector>

#include "nlohmann/json.hpp"
#include "home_pgc_result.h"

namespace bilibili {

class BangumiCollectionWrapper {
public:
    size_t pn, ps, total;
    PGCItemListResult list;
};

inline void from_json(const nlohmann::json& nlohmann_json_j, BangumiCollectionWrapper& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, pn, ps, total, list));
}

};  // namespace bilibili