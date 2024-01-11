//
// Created by fang on 2023/7/18.
//

#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include "bilibili/util/md5.hpp"

namespace bilibili {

std::string genUUID(const std::string& str) {
    auto hash = websocketpp::md5::md5_hash_hex(str);

    std::ostringstream uuid;
    uuid << std::setw(8) << std::setfill('0') << hash.substr(0, 8) << "-";
    uuid << std::setw(4) << std::setfill('0') << hash.substr(8, 4) << "-";
    uuid << std::setw(4) << std::setfill('0') << hash.substr(12, 4) << "-";
    uuid << std::setw(4) << std::setfill('0') << hash.substr(16, 4) << "-";
    uuid << std::setw(12) << std::setfill('0') << hash.substr(20, 12);

    return uuid.str();
}

}  // namespace bilibili
