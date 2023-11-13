//
// Created by maye174 on 2023/4/6.
//

#pragma once

#include <cstddef>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>

#include <borealis.hpp>
#include <borealis/core/singleton.hpp>
#include "mongoose.h"
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

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

typedef void (*on_message_func_t)(const std::string &);
class LiveDanmaku : public brls::Singleton<LiveDanmaku> {
public:
    int room_id;
    int uid;
    void connect(int room_id, int64_t uid);
    void disconnect();
    void send_join_request(int room_id, int64_t uid);

    void send_heartbeat();
    void send_text_message(const std::string &message);

    void setonMessage(on_message_func_t func);
    on_message_func_t onMessage = nullptr;

    void set_wait_time(size_t time);
    size_t wait_time = 100;

    LiveDanmaku();
    ~LiveDanmaku();

    bool is_connected();
    std::atomic_bool connected{false};
    bool is_evOK();
    std::atomic_bool ms_ev_ok{false};

    LiveDanmakuinfo info;

    std::thread mongoose_thread;
    std::thread task_thread;
    std::mutex mongoose_mutex;
    mg_mgr *mgr;
    mg_connection *nc;
};