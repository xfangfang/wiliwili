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

/**
 * 将 6 位十六进制颜色字符串（RRGGBB 或 #RRGGBB）解析为 RGB 分量。
 * 成功返回 true，字符串格式不合法或包含非法字符时返回 false。
 */
bool parseHexColor(const std::string& hex, uint8_t& r, uint8_t& g, uint8_t& b);

};  // namespace wiliwili