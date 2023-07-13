//
// Created by maye174 on 2023/4/6.
//

#include <nlohmann/json.hpp>

#include <sstream>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "live/danmaku_live.hpp"
#include "live/ws_utils.hpp"
#include "live/extract_messages.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

using json = nlohmann::json;

const std::string url = "ws://broadcastlv.chat.bilibili.com:2244/sub";

LiveDanmaku::LiveDanmaku() {
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed with error: %d\n", result);
    }
#endif
}

LiveDanmaku::~LiveDanmaku() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

static void mongoose_event_handler(struct mg_connection *nc, int ev, void *ev_data, void *user_data);

void LiveDanmaku::connect(int room_id, int uid) {
    if (connected.load(std::memory_order_acquire)) {
        return;
    }
    connected.store(true, std::memory_order_release);

    // Create and configure Mongoose connection
    struct mg_mgr *mgr = new mg_mgr;
    mg_log_set(MG_LL_NONE);
    mg_mgr_init(mgr);
    struct mg_connection *nc = mg_ws_connect(mgr, url.c_str(), mongoose_event_handler, this, nullptr);

    this->mgr = mgr;
    this->nc = nc;

    if(nc == nullptr) {
        std::cout << "nc is null" << std::endl;
        disconnect();
        mg_mgr_free(this->mgr);
        delete mgr;
        return;
    }


    this->room_id = room_id;
    this->uid = uid;
    //mg_mgr_poll(this->mgr, 10);

    // Start Mongoose event loop and heartbeat thread
    mongoose_thread = std::thread([this]() {
        int last = 0;
        int s = 0;
        while (this->is_connected()) {
            this->mongoose_mutex.lock();
            if(this->nc == nullptr) {
                break;
            }
            this->mongoose_mutex.unlock();
            mg_mgr_poll(this->mgr, 800);
            s += 1;
            if (s - last >= 36) {
                send_heartbeat();
                last = s;
            }
            if (s < 0) {
                s = 0;
                last = 0;
            }
        }
        mg_mgr_free(this->mgr);
        delete this->mgr;
    });
}



void LiveDanmaku::disconnect() {
    if (!connected.load(std::memory_order_acquire)) {
        return;
    }

    // Stop Mongoose event loop thread
    connected.store(false, std::memory_order_release);

    // Stop Mongoose event loop thread
    if(mongoose_thread.joinable()) {
        mongoose_thread.join();
    }
}

bool LiveDanmaku::is_connected() {
    return connected.load(std::memory_order_acquire);
}

void LiveDanmaku::send_join_request(int room_id, int uid) {
    json join_request = {
        {"clientver", "1.6.3"},
        {"platform", "web"},
        {"protover", 0},
        {"roomid", room_id},
        {"uid", uid},
        {"type", 2}
    };
    std::string join_request_str = join_request.dump();
    std::vector<uint8_t> packet = encode_packet(0, 7, join_request_str);
    std::string packet_str(packet.begin(), packet.end());
    mongoose_mutex.lock();
    mg_ws_send(this->nc, packet_str.data(), packet_str.size(), WEBSOCKET_OP_BINARY);
    mongoose_mutex.unlock();
}

void LiveDanmaku::send_heartbeat() {
    std::vector<uint8_t> packet = encode_packet(0, 2, "");
    std::string packet_str(packet.begin(), packet.end());
    mongoose_mutex.lock();
    mg_ws_send(this->nc, packet_str.data(), packet_str.size(), WEBSOCKET_OP_BINARY);
    mongoose_mutex.unlock();
}

void LiveDanmaku::send_text_message(const std::string &message) {
//暂时不用
}

static void mongoose_event_handler(struct mg_connection *nc, int ev, void *ev_data, void *user_data) {
    LiveDanmaku *liveDanmaku = static_cast<LiveDanmaku *>(user_data);
    if (ev == MG_EV_OPEN) {
        nc->is_hexdumping = 1;
    } else if (ev == MG_EV_ERROR) {
        //MG_ERROR(("%p %s", nc->fd, (char *) ev_data));
        liveDanmaku->disconnect();
    } else if (ev == MG_EV_WS_OPEN) {
        liveDanmaku->send_join_request(liveDanmaku->room_id, liveDanmaku->uid);
    } else if (ev == MG_EV_WS_MSG) {
        struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
        std::string message(wm->data.ptr, wm->data.len);
        liveDanmaku->onMessage(message);
    } else if(ev == MG_EV_CLOSE) {
        liveDanmaku->disconnect();
    }
}

void LiveDanmaku::setonMessage(std::function<void(std::string)> func) {
    onMessage = func;
}