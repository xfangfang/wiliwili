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

std::string urlEncode(const std::string &in) {
    const char *input = in.c_str();

    std::string working;

    while (*input) {
        const unsigned char c = *input++;
        if (isNonSymbol(c)) {
            working += (char)c;
        } else {
            working += fmt::format("%{:02x}", c);
        }
    }

    return working;
}

};  // namespace wiliwili