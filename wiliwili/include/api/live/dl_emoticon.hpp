//
//Created by Maye174 on 2023/8/7.
//

#pragma once

#include <unordered_map>
#include <string>

// 修改表情存储结构，从存储二进制数据改为存储URL
using lmp = std::unordered_map<std::string, std::string>;

lmp dl_emoticon(int room_id);