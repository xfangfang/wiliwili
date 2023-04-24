//
// Created by fang on 2023/1/16.
//

#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include "fmt/format.h"

namespace wiliwili {

inline bool isNonSymbol(unsigned char c) {
    if (c == '\0') return true;
    return (c >= 48 && c <= 57) || (c >= 65 && c <= 90) ||
        (c >= 97 && c <= 122);
}

std::string urlEncode(const std::string &in);

template <typename... Args> inline std::string format(fmt::string_view fmt, Args&&... args) {
    return fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
}

};  // namespace wiliwili