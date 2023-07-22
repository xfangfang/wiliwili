//
// Created by maye174 on 2023/4/6.
//

#pragma once

#include <borealis.hpp>
#include <borealis/core/singleton.hpp>

#include <string>
#include <atomic>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>

#include "mongoose.h"  // Include Mongoose header file

class LiveDanmaku : public brls::Singleton<LiveDanmaku> {
public:
    int room_id;
    int uid;
    void connect(int room_id, int uid);
    void disconnect();
    void send_join_request(int room_id, int uid);

    void send_heartbeat();
    void send_text_message(const std::string &message);

    void setonMessage(std::function<void(std::string)> func);
    std::function<void(std::string)> onMessage;

    void set_wait_time(int time);
    int wait_time = 600;

    LiveDanmaku();
    ~LiveDanmaku();

    bool is_connected();
    std::atomic_bool connected{false};
    bool is_evOK();
    std::atomic_bool ms_ev_ok{false};

    std::thread mongoose_thread;
    std::thread heartbeat_thread;
    std::mutex mongoose_mutex;
    mg_mgr *mgr;
    mg_connection *nc;
    mg_timer *heartbeat_timer_id;
};