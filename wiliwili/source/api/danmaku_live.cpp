//
// Created by maye174 on 2023/4/6.
//

#include "live/danmaku_live.hpp"
#include "bilibili.h"
#include "bilibili/api.h"
#include "bilibili/result/live_danmaku_result.h"
#include "bilibili/util/http.hpp"
#include "live/ws_utils.hpp"
#include "utils/config_helper.hpp"

#include <cstddef>
#include <ctime>
#include <queue>
#include <condition_variable>
#include <string>

namespace bilibili {
void BilibiliClient::get_live_danmaku_info(int roomid, const std::function<void(LiveDanmakuinfo)> &callback,
                                           const ErrorCallback &error) {
    HTTP::getResultAsync<LiveDanmakuinfo>(Api::LiveDanmakuInfo, {{"type", "0"}, {"id", std::to_string(roomid)}},
                                          callback, error);
}
}  // namespace bilibili

static void mongoose_event_handler(struct mg_connection *nc, int ev, void *ev_data);

static void heartbeat_timer(void *param) {
    auto liveDanmaku = static_cast<LiveDanmaku *>(param);
    if (liveDanmaku->is_connected() and liveDanmaku->is_evOK()) {
        liveDanmaku->send_heartbeat();
    }
}

static std::queue<std::string> msg_q;
static std::condition_variable cv;
static std::mutex msg_q_mutex;

static void add_msg(std::string &&a) {
    std::lock_guard<std::mutex> lock(msg_q_mutex);
    msg_q.emplace(std::move(a));
    cv.notify_one();
}

LiveDanmaku::LiveDanmaku() {}

LiveDanmaku::~LiveDanmaku() {
    disconnect();
    std::lock_guard<std::mutex> lock(msg_q_mutex);
    while (!msg_q.empty()) msg_q.pop();
}

void LiveDanmaku::connect(int room_id, uint64_t uid, const bilibili::LiveDanmakuinfo &info) {
    if (connected.load(std::memory_order_acquire)) {
        return;
    }
    connected.store(true, std::memory_order_release);

    // Create and configure Mongoose connection
    this->mgr = new mg_mgr;

    // get_live_s(room_id);
    mg_log_set(MG_LL_NONE);

    this->info = info;
    mg_mgr_init(this->mgr);
    mg_wakeup_init(this->mgr);

    std::string host = "ws://" + this->info.host_list[this->info.host_list.size() - 1].host + ":" +
                       std::to_string(this->info.host_list[this->info.host_list.size() - 1].ws_port) + "/sub";
    this->nc = mg_ws_connect(this->mgr, host.c_str(), mongoose_event_handler, this, nullptr);

    if (this->nc == nullptr) {
        brls::Logger::error("(LiveDanmaku) nc is null");
        this->disconnect();
        mg_mgr_free(this->mgr);
        delete this->mgr;
        return;
    }

    this->room_id = room_id;
    this->uid     = uid;

    // Start Mongoose event loop and heartbeat thread
    this->mongoose_thread = std::thread([this]() {
        while (this->is_connected()) {
            this->mongoose_mutex.lock();
            if (this->nc == nullptr) {
                break;
            }
            this->mongoose_mutex.unlock();
            mg_mgr_poll(this->mgr, this->wait_time);
        }
        mg_mgr_free(this->mgr);
        delete this->mgr;
    });

    this->task_thread = std::thread([this]() {
        while (true) {
            std::unique_lock<std::mutex> lock(msg_q_mutex);
            cv.wait(lock, [this] { return !msg_q.empty() or !this->is_connected(); });
            if (!this->is_connected()) break;
            auto msg = std::move(msg_q.front());
            msg_q.pop();
            lock.unlock();

            this->onMessage(msg);
        }
    });

    brls::Logger::info("(LiveDanmaku) connect step finish");
}

void LiveDanmaku::disconnect() {
    if (!connected.load(std::memory_order_acquire)) {
        return;
    }

    // Stop Mongoose event loop thread
    connected.store(false, std::memory_order_release);

    // Wakeup the mainloop
    if (mgr && nc) mg_wakeup(this->mgr, this->nc->id, nullptr, 0);

    // Stop Mongoose event loop thread
    if (mongoose_thread.joinable()) {
        mongoose_thread.join();
    }

    cv.notify_one();

    if (task_thread.joinable()) {
        task_thread.join();
    }
    brls::Logger::info("(LiveDanmaku) close step finish");
}

void LiveDanmaku::set_wait_time(size_t time) { wait_time = time; }

bool LiveDanmaku::is_connected() { return connected.load(std::memory_order_acquire); }

bool LiveDanmaku::is_evOK() { return ms_ev_ok.load(std::memory_order_acquire); }

void LiveDanmaku::send_join_request(const int room_id, const uint64_t uid) {
    json join_request            = {{"uid", uid},
                                    {"roomid", room_id},
                                    {"protover", 2},
                                    {"buvid", ProgramConfig::instance().getBuvid3()},
                                    {"platform", "web"},
                                    {"type", 2},
                                    {"key", this->info.token}};
    std::string join_request_str = join_request.dump();
    brls::Logger::info("(LiveDanmaku) join_request:{}", join_request_str);
    std::vector<uint8_t> packet = encode_packet(0, 7, join_request_str);
    std::string packet_str(packet.begin(), packet.end());
    mongoose_mutex.lock();
    if (this->nc == nullptr) return;
    mg_ws_send(this->nc, packet_str.data(), packet_str.size(), WEBSOCKET_OP_BINARY);
    mongoose_mutex.unlock();
}

void LiveDanmaku::send_heartbeat() {
    brls::Logger::debug("(LiveDanmaku) send_heartbeat");
    std::vector<uint8_t> packet = encode_packet(0, 2, "");
    std::string packet_str(packet.begin(), packet.end());
    mongoose_mutex.lock();
    if (this->nc == nullptr) return;
    mg_ws_send(this->nc, packet_str.data(), packet_str.size(), WEBSOCKET_OP_BINARY);
    mongoose_mutex.unlock();
}

void LiveDanmaku::send_text_message(const std::string &message) {
    //暂时不用
}

static void mongoose_event_handler(struct mg_connection *nc, int ev, void *ev_data) {
    auto *liveDanmaku = static_cast<LiveDanmaku *>(nc->fn_data);
    liveDanmaku->ms_ev_ok.store(true, std::memory_order_release);
    if (ev == MG_EV_OPEN) {
        MG_DEBUG(("%p %s", nc->fd, (char *)ev_data));
#ifdef MONGOOSE_HEX_DUMPS
        nc->is_hexdumping = 1;
#else
        nc->is_hexdumping = 0;
#endif
    } else if (ev == MG_EV_ERROR) {
        MG_ERROR(("%p %s", nc->fd, (char *)ev_data));
        liveDanmaku->ms_ev_ok.store(false, std::memory_order_release);
    } else if (ev == MG_EV_WS_OPEN) {
        MG_DEBUG(("%p %s", nc->fd, (char *)ev_data));
        liveDanmaku->send_join_request(liveDanmaku->room_id, liveDanmaku->uid);
        mg_timer_add(liveDanmaku->mgr, 30000, MG_TIMER_REPEAT, heartbeat_timer, nc->fn_data);
    } else if (ev == MG_EV_WS_MSG) {
        MG_DEBUG(("%p %s", nc->fd, (char *)ev_data));
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
        add_msg(std::string(wm->data.buf, wm->data.len));
    } else if (ev == MG_EV_CLOSE) {
        MG_DEBUG(("%p %s", nc->fd, (char *)ev_data));
        liveDanmaku->ms_ev_ok.store(false, std::memory_order_release);
    }
}

void LiveDanmaku::setonMessage(on_message_func_t func) { onMessage = func; }