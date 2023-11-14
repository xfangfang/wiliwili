//
// Created by fang on 2023/11/14.
//

#pragma once

#include <nlohmann/json.hpp>

namespace bilibili {

class LiveDanmakuHostinfo {
public:
    std::string host;
    int ws_port;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveDanmakuHostinfo, host, ws_port);

class LiveDanmakuinfo {
public:
    std::vector<LiveDanmakuHostinfo> host_list;
    std::string token = "";
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LiveDanmakuinfo, host_list, token);

};  // namespace bilibili