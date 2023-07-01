
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

#define MG_ENABLE_HTTP 1
#define MG_ENABLE_HTTP_WEBSOCKET 1
#include "mongoose.h"  // Include Mongoose header file

class LiveDanmaku : public brls::Singleton<LiveDanmaku> {
public:
    int room_id;
    int uid;
    void connect(int room_id, int uid);
    void disconnect();
    bool is_connected();

    void send_join_request(int room_id, int uid);
    void send_heartbeat();
    void send_text_message(const std::string &message);

    void setonMessage(std::function<void(std::string)> func);
    std::function<void(std::string)> onMessage;

    LiveDanmaku();
    ~LiveDanmaku();
private:
    std::atomic_bool connected{false};
    std::atomic_bool ms_ev_ok{false};

    std::thread mongoose_thread;
    std::mutex mongoose_mutex;
    mg_mgr *mgr;
    mg_connection *nc;
    
    //人气值
    int popularity = 0;
};
