//
// Created by maye174 on 2023/4/6.
//

#pragma once

#include <cstddef>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <queue>
#include <condition_variable>

#include <borealis/core/singleton.hpp>
#include <mongoose.h>
#include <nlohmann/json.hpp>

#include "bilibili/result/live_danmaku_result.h"

using json = nlohmann::json;

class LiveDanmaku {
public:
    using MessageCallback = std::function<void(const std::string&)>;

    LiveDanmaku();
    ~LiveDanmaku();

    // 禁止拷贝和移动
    LiveDanmaku(const LiveDanmaku&) = delete;
    LiveDanmaku& operator=(const LiveDanmaku&) = delete;
    LiveDanmaku(LiveDanmaku&&) = delete;
    LiveDanmaku& operator=(LiveDanmaku&&) = delete;

    void connect(int room_id, uint64_t uid, const bilibili::LiveDanmakuinfo &info);
    void disconnect();
    void send_join_request(int room_id, uint64_t uid);
    void send_heartbeat();
    void send_text_message(const std::string &message);

    void setonMessage(MessageCallback callback);
    void set_wait_time(size_t time);

    bool is_connected() const;
    bool is_evOK() const;

private:
    // 线程安全的消息队列
    class MessageQueue {
    public:
        void push(std::string&& msg);
        bool pop(std::string& msg, bool wait, const std::atomic<bool>& running);
        void notify_all();
        void clear();
    private:
        std::queue<std::string> queue;
        std::mutex mutex;
        std::condition_variable cv;
    };

public:
    // 这些属性保留为公有以便于回调函数访问
    int room_id = 0;
    uint64_t uid = 0;
    bilibili::LiveDanmakuinfo info;
    mg_mgr* mgr = nullptr;
    mg_connection* nc = nullptr;
    std::atomic<bool> connected{false};
    std::atomic<bool> ms_ev_ok{false};
    size_t wait_time = 100;

private:
    std::thread mongoose_thread;
    std::thread task_thread;
    MessageCallback onMessage = nullptr;
    MessageQueue msg_queue;
    
    friend void add_message(LiveDanmaku* live, std::string&& msg);
};