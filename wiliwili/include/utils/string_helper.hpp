//
// Created by fang on 2023/1/16.
//

#pragma once

#include <string>
#include <cstdlib>
#include <fmt/format.h>

namespace wiliwili {

std::string urlEncode(const std::string &in);

std::string base64Encode(const std::string &in);

int base64Decode(const std::string &in, std::string &out);

std::string decompressGzipData(const std::string &compressedData);

template <typename... Args>
inline std::string format(fmt::string_view fmt, Args &&...args) {
    return fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
}

std::string toUpper( const std::string & str, std::string::size_type length );

};  // namespace wiliwili