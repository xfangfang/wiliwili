//
// Created by fang on 2022/5/26.
//

#pragma once

#include "nlohmann/json.hpp"

using namespace std;

namespace bilibili {

    class UserSimpleResult {
    public:
        int mid;
        string name;
        string face;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserSimpleResult, mid, name, face);

}