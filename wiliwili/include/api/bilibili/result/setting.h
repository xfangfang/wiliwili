//
// Created by fang on 2022/9/19.
//

#pragma once

#include "nlohmann/json.hpp"

namespace bilibili {

class UnixTimeResult {
public:
    size_t now;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UnixTimeResult, now);

}  // namespace bilibili