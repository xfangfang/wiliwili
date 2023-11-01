//
//Created by Maye174 on 2023/8/7.
//

#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

using lmp = std::unordered_map<std::string, std::vector<uint8_t>>;

lmp dl_emoticon(int room_id);