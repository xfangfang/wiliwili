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
#include <string>

namespace bilibili {
void BilibiliClient::get_live_danmaku_info(int roomid, const std::function<void(LiveDanmakuinfo)> &callback,
                                           const ErrorCallback &error) {
    HTTP::getResultAsync<LiveDanmakuinfo>(Api::LiveDanmakuInfo, {{"type", "0"}, {"id", std::to_string(roomid)}},
                                          callback, error);
}
}  // namespace bilibili

static void mongoose_event_handler(struct mg_connection *nc, int ev, void *ev_data);

// 心跳定时器回调函数
static void heartbeat_timer(void *param) {
    auto liveDanmaku = static_cast<LiveDanmaku *>(param);
    if (liveDanmaku->is_connected() && liveDanmaku->is_evOK()) {
        int cmd = 1; // 心跳命令
        mg_wakeup(liveDanmaku->mgr, liveDanmaku->nc->id, &cmd, sizeof(cmd));
    }
}

// 添加消息到队列的辅助函数
void add_message(LiveDanmaku* live, std::string&& msg) {
    if (live) {
        live->msg_queue.push(std::move(msg));
    }
}

// MessageQueue实现
void LiveDanmaku::MessageQueue::push(std::string&& msg) {
    std::lock_guard<std::mutex> lock(mutex);
    queue.emplace(std::move(msg));
    cv.notify_one();
}

bool LiveDanmaku::MessageQueue::pop(std::string& msg, bool wait, const std::atomic<bool>& running) {
    std::unique_lock<std::mutex> lock(mutex);
    
    if (wait) {
        cv.wait(lock, [this, &running] { 
            return !queue.empty() || !running.load(std::memory_order_acquire); 
        });
    }
    
    if (!running.load(std::memory_order_acquire) && queue.empty()) {
        return false;
    }
    
    if (!queue.empty()) {
        msg = std::move(queue.front());
        queue.pop();
        return true;
    }
    
    return false;
}

void LiveDanmaku::MessageQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex);
    std::queue<std::string> empty;
    std::swap(queue, empty);
}

void LiveDanmaku::MessageQueue::notify_all() {
    cv.notify_all();
}

// LiveDanmaku实现
LiveDanmaku::LiveDanmaku() = default;

LiveDanmaku::~LiveDanmaku() {
    disconnect();
}

bool LiveDanmaku::is_connected() const {
    return connected.load(std::memory_order_acquire);
}

bool LiveDanmaku::is_evOK() const {
    return ms_ev_ok.load(std::memory_order_acquire);
}

void LiveDanmaku::connect(int room_id, uint64_t uid, const bilibili::LiveDanmakuinfo &info) {
    if (is_connected()) {
        return;
    }
    
    // 初始化成员变量
    this->room_id = room_id;
    this->uid = uid;
    this->info = info;
    
    // 创建并配置Mongoose
    mgr = new mg_mgr;
    mg_log_set(MG_LL_NONE);
    mg_mgr_init(mgr);
    mg_wakeup_init(mgr);  // 初始化wakeup功能

    // 建立WebSocket连接
    std::string host = "ws://" + this->info.host_list[this->info.host_list.size() - 1].host + ":" +
                      std::to_string(this->info.host_list[this->info.host_list.size() - 1].ws_port) + "/sub";
    nc = mg_ws_connect(mgr, host.c_str(), mongoose_event_handler, this, nullptr);

    if (nc == nullptr) {
        brls::Logger::error("(LiveDanmaku) 无法创建WebSocket连接");
        mg_mgr_free(mgr);
        delete mgr;
        mgr = nullptr;
        return;
    }

    // 设置连接标志
    connected.store(true, std::memory_order_release);

    // 启动Mongoose事件循环线程
    mongoose_thread = std::thread([this]() {
        while (this->is_connected()) {
            if (mgr) {
                mg_mgr_poll(mgr, wait_time);
            } else {
                break;
            }
        }
        
        // 在线程结束前安全释放资源
        if (mgr) {
            mg_mgr_free(mgr);
            delete mgr;
            mgr = nullptr;
            nc = nullptr;
        }
    });

    // 启动消息处理线程
    task_thread = std::thread([this]() {
        std::string msg;
        while (connected.load(std::memory_order_acquire)) {
            if (msg_queue.pop(msg, true, connected) && onMessage) {
                onMessage(msg);
            }
        }
        
        // 直接清空剩余消息
        msg_queue.clear();
    });

    brls::Logger::info("(LiveDanmaku) connect step finish");
}

void LiveDanmaku::disconnect() {
    if (!is_connected()) {
        return;
    }

    // 标记为断开连接
    connected.store(false, std::memory_order_release);

    // 通知消息队列退出等待
    msg_queue.notify_all();
    
    // 等待线程结束
    if (mongoose_thread.joinable()) {
        mongoose_thread.join();
    }

    if (task_thread.joinable()) {
        task_thread.join();
    }
    brls::Logger::info("(LiveDanmaku) close step finish");
}

void LiveDanmaku::set_wait_time(size_t time) { 
    wait_time = time; 
}

void LiveDanmaku::send_join_request(const int room_id, const uint64_t uid) {
    if (!is_connected() || !mgr || !nc) {
        return;
    }

    json join_request = {{"uid", uid},
                         {"roomid", room_id},
                         {"protover", 2},
                         {"buvid", ProgramConfig::instance().getBuvid3()},
                         {"platform", "web"},
                         {"type", 2},
                         {"key", this->info.token}};
    std::string join_request_str = join_request.dump();
    brls::Logger::info("(LiveDanmaku) join_request:{}", join_request_str);
    std::vector<uint8_t> packet = encode_packet(0, 7, join_request_str);
    
    // 在主Mongoose线程中安全发送
    auto *packet_ptr = new std::string(packet.begin(), packet.end());
    mg_wakeup(mgr, nc->id, &packet_ptr, sizeof(void*));
}

void LiveDanmaku::send_heartbeat() {
    if (!is_connected() || !mgr || !nc) {
        return;
    }

    brls::Logger::debug("(LiveDanmaku) send_heartbeat");
    std::vector<uint8_t> packet = encode_packet(0, 2, "");
    
    // 在主Mongoose线程中安全发送
    auto *packet_ptr = new std::string(packet.begin(), packet.end());
    mg_wakeup(mgr, nc->id, &packet_ptr, sizeof(void*));
}

void LiveDanmaku::send_text_message(const std::string &message) {
    // 暂时不用
}

void LiveDanmaku::setonMessage(MessageCallback callback) {
    onMessage = std::move(callback);
}

static void mongoose_event_handler(struct mg_connection *nc, int ev, void *ev_data) {
    auto *liveDanmaku = static_cast<LiveDanmaku *>(nc->fn_data);
    if (!liveDanmaku) return;
    
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
        liveDanmaku->ms_ev_ok.store(true, std::memory_order_release);
        liveDanmaku->send_join_request(liveDanmaku->room_id, liveDanmaku->uid);
        mg_timer_add(liveDanmaku->mgr, 30000, MG_TIMER_REPEAT, heartbeat_timer, nc->fn_data);
    } else if (ev == MG_EV_WS_MSG) {
        MG_DEBUG(("%p %s", nc->fd, (char *)ev_data));
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
        add_message(liveDanmaku, std::string(wm->data.buf, wm->data.len));
    } else if (ev == MG_EV_CLOSE) {
        MG_DEBUG(("%p %s", nc->fd, (char *)ev_data));
        liveDanmaku->ms_ev_ok.store(false, std::memory_order_release);
    } else if (ev == MG_EV_WAKEUP) {
        // 处理唤醒事件
        struct mg_str *data = (struct mg_str *)ev_data;
        if (data != nullptr && data->len >= sizeof(int)) {
            int *cmd = (int *)data->buf;
            if (*cmd == 1) {
                // 心跳命令
                liveDanmaku->send_heartbeat();
            } else {
                // 数据包指针
                std::string **packet_ptr = (std::string **)data->buf;
                if (*packet_ptr != nullptr) {
                    // 发送数据包
                    mg_ws_send(nc, (*packet_ptr)->data(), (*packet_ptr)->size(), WEBSOCKET_OP_BINARY);
                    // 释放内存
                    delete *packet_ptr;
                    *packet_ptr = nullptr;
                }
            }
        }
    }
}