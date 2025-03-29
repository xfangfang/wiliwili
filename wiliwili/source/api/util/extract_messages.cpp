
#include "live/extract_messages.hpp"

#include <nlohmann/json.hpp>
#include <cstdlib>
#include <cstring>

danmaku_t *danmaku_t_init() {
    danmaku_t *ret               = (danmaku_t *)malloc(sizeof(danmaku_t));
    ret->user_name               = nullptr;
    ret->user_name_color         = nullptr;
    ret->dan                     = nullptr;
    ret->fan_medal_name          = nullptr;
    ret->fan_medal_liveuser_name = nullptr;
    ret->user_uid                = 0;
    ret->dan_color               = 16777215;
    ret->fan_medal_roomid        = 0;
    ret->fan_medal_font_color    = 0;
    ret->fan_medal_border_color  = 0;
    ret->fan_medal_start_color   = 0;
    ret->fan_medal_end_color     = 0;
    ret->fan_medal_liveuser_uid  = 0;
    ret->is_emoticon             = 0;
    ret->dan_type                = 1;
    ret->dan_size                = 25;
    ret->user_level              = 1;
    ret->user_vip_level          = 0;
    ret->fan_medal_level         = 0;
    ret->fan_medal_vip_level     = 0;
    ret->is_guard                = 0;
    ret->glory_v                 = 0;
    return ret;
}

static char *strdup_s(const char *s) {
    if (!s) return nullptr;
    char *ret = (char *)malloc(strlen(s) + 1);
    if (!ret) return nullptr;
    strcpy(ret, s);
    return ret;
}

danmaku_t *danmaku_t_copy(const danmaku_t *p) {
    if (!p) return nullptr;
    danmaku_t *ret = (danmaku_t *)malloc(sizeof(danmaku_t));
    if (!ret) return nullptr;
    memcpy(ret, p, sizeof(danmaku_t));
    ret->user_name               = strdup_s(p->user_name);
    ret->user_name_color         = strdup_s(p->user_name_color);
    ret->dan                     = strdup_s(p->dan);
    ret->fan_medal_name          = strdup_s(p->fan_medal_name);
    ret->fan_medal_liveuser_name = strdup_s(p->fan_medal_liveuser_name);
    return ret;
}

void danmaku_t_free(const danmaku_t *p) {
    free(p->user_name);
    free(p->user_name_color);
    free(p->dan);
    free(p->fan_medal_name);
    free(p->fan_medal_liveuser_name);
}

static char *to_cstr(const std::string &s) {
    char *ret = (char *)malloc(s.size() + 1);
    memcpy(ret, s.c_str(), s.size() + 1);
    return ret;
}

std::vector<live_t> extract_messages(const std::vector<std::string> &messages) {
    std::vector<live_t> live_messages;
    live_messages.reserve(messages.size() / 5);

    for (const auto &message : messages) {
        nlohmann::json json_message;

        try {
            json_message = nlohmann::json::parse(message);
        } catch (const std::exception &e) {
            continue;
        } catch (...) {
            continue;
        }

        auto it = json_message.find("cmd");

        if (it == json_message.end()) continue;

        if (it->get_ref<std::string &>() == "SUPER_CHAT_MESSAGE" || it->get_ref<std::string &>() == "SUPER_CHAT_MESSAGE_JPN") {
            if (!json_message.contains("data")) continue;
            auto &data = json_message["data"];

            super_chat_t *sc = super_chat_t_init();
            if (!sc) {
                continue;
            }

            // 获取消息内容
            if (data.contains("message") && data["message"].is_string()) {
                sc->message = to_cstr(data["message"].get_ref<const std::string &>());
            }

            // 获取消息字体颜色
            if (data.contains("message_font_color") && data["message_font_color"].is_string()) {
                sc->message_font_color = to_cstr(data["message_font_color"].get_ref<const std::string &>());
            }

            // 获取背景颜色
            if (data.contains("background_color") && data["background_color"].is_string()) {
                sc->background_color = to_cstr(data["background_color"].get_ref<const std::string &>());
            }
            
            // 获取背景渐变色（开始）
            if (data.contains("background_color_start") && data["background_color_start"].is_string()) {
                sc->background_color_start = to_cstr(data["background_color_start"].get_ref<const std::string &>());
            }
            
            // 获取背景渐变色（结束）
            if (data.contains("background_color_end") && data["background_color_end"].is_string()) {
                sc->background_color_end = to_cstr(data["background_color_end"].get_ref<const std::string &>());
            }

            // 获取金额
            if (data.contains("price") && data["price"].is_number()) {
                sc->price = data["price"].get<int>();
            }

            // 获取持续时间
            if (data.contains("time") && data["time"].is_number()) {
                sc->time = data["time"].get<int>();
            }

            // 获取用户信息
            if (data.contains("user_info")) {
                auto &user_info = data["user_info"];
                
                // 用户名
                if (user_info.contains("uname") && user_info["uname"].is_string()) {
                    sc->user_name = to_cstr(user_info["uname"].get_ref<const std::string &>());
                }
                
                // 用户头像
                if (user_info.contains("face") && user_info["face"].is_string()) {
                    sc->user_face = to_cstr(user_info["face"].get_ref<const std::string &>());
                }
                
                // 大航海等级
                if (user_info.contains("guard_level") && user_info["guard_level"].is_number()) {
                    sc->guard_level = user_info["guard_level"].get<int>();
                }
            }

            // 获取UID
            if (data.contains("uid") && data["uid"].is_number()) {
                sc->user_uid = data["uid"].get<int>();
            }

            // 获取粉丝牌信息
            if (data.contains("medal_info")) {
                auto &medal_info = data["medal_info"];
                
                // 粉丝牌名称
                if (medal_info.contains("medal_name") && medal_info["medal_name"].is_string()) {
                    sc->fan_medal_name = to_cstr(medal_info["medal_name"].get_ref<const std::string &>());
                }
                
                // 粉丝牌等级
                if (medal_info.contains("medal_level") && medal_info["medal_level"].is_number()) {
                    sc->fan_medal_level = medal_info["medal_level"].get<int>();
                }
                
                // 粉丝牌主播名称
                if (medal_info.contains("anchor_uname") && medal_info["anchor_uname"].is_string()) {
                    sc->fan_medal_liveuser_name = to_cstr(medal_info["anchor_uname"].get_ref<const std::string &>());
                }
            }

            live_messages.emplace_back(live_t{super_chat, sc});
            continue;
        }

        if (it->get_ref<std::string &>() == "WATCHED_CHANGE") {
            if (json_message["data"]["num"].is_number()) continue;

            auto num = json_message["data"]["num"].get<int>();

            watched_change_t *wc = (watched_change_t *)malloc(sizeof(watched_change_t));

            if (!wc) {
                continue;
            }

            wc->num = num;
            live_messages.emplace_back(live_t{watched_change, wc});

        } else if (it->get_ref<std::string &>() == "DANMU_MSG") {
            auto &info = json_message["info"];

            if (!info.is_array() || info.size() < 17) continue;

            danmaku_t *dan = danmaku_t_init();
            if (!dan) {
                continue;
            }

            if (info[0].is_array() && info[0].size() > 12) {
                auto &attribute = info[0];
                if (attribute[1].is_number()) dan->dan_type = attribute[1].get<int>();

                if (attribute[2].is_number()) dan->dan_size = attribute[2].get<int>();

                if (attribute[3].is_number()) dan->dan_color = attribute[3].get<int>();

                if (attribute[12].is_number()) dan->is_emoticon = attribute[12].get<int>();
            }

            if (info[1].is_string()) {
                dan->dan = to_cstr(info[1].get_ref<const std::string &>());
            }

            if (info[2].is_array() && info[2].size() == 8) {
                auto &user = info[2];
                if (user[0].is_number()) dan->user_uid = user[0].get<int>();

                if (user[1].is_string()) dan->user_name = to_cstr(user[1].get_ref<const std::string &>());

                if (user[2].is_number()) dan->is_guard = user[2].get<int>();

                if (user[7].is_string()) dan->user_name_color = to_cstr(user[7].get_ref<const std::string &>());
            }

            if (info[3].is_array() && info[3].size() == 13) {
                auto &fan = info[3];
                if (fan[0].is_number()) dan->fan_medal_level = fan[0].get<int>();

                if (fan[1].is_string()) dan->fan_medal_name = to_cstr(fan[1].get_ref<const std::string &>());

                if (fan[2].is_string()) dan->fan_medal_liveuser_name = to_cstr(fan[2].get_ref<const std::string &>());

                if (fan[3].is_number()) dan->fan_medal_roomid = fan[3].get<int>();

                if (fan[6].is_number()) dan->fan_medal_font_color = fan[6].get<int>();

                if (fan[7].is_number()) dan->fan_medal_border_color = fan[7].get<int>();

                if (fan[8].is_number()) dan->fan_medal_end_color = fan[8].get<int>();

                if (fan[9].is_number()) dan->fan_medal_start_color = fan[9].get<int>();

                if (fan[10].is_number()) dan->fan_medal_vip_level = fan[10].get<int>();

                if (fan[12].is_number()) dan->fan_medal_liveuser_uid = fan[12].get<int>();
            }

            if (info[4].is_array() && info[4].size() > 0) {
                if (info[4][0].is_number()) dan->user_level = info[4][0].get<int>();
            }

            if (info[7].is_number()) {
                dan->user_vip_level = info[7].get<int>();
            }

            live_messages.emplace_back(live_t{danmaku, dan});
        }
    }
    return live_messages;
}

super_chat_t *super_chat_t_init() {
    super_chat_t *ret          = (super_chat_t *)malloc(sizeof(super_chat_t));
    ret->user_name             = nullptr;
    ret->user_face             = nullptr;
    ret->message               = nullptr;
    ret->message_font_color    = nullptr;
    ret->background_color      = nullptr;
    ret->background_color_start = nullptr;
    ret->background_color_end  = nullptr;
    ret->fan_medal_name        = nullptr;
    ret->fan_medal_liveuser_name = nullptr;
    ret->user_uid              = 0;
    ret->price                 = 0;
    ret->time                  = 0;
    ret->fan_medal_level       = 0;
    ret->guard_level           = 0;
    return ret;
}

super_chat_t *super_chat_t_copy(const super_chat_t *p) {
    if (!p) return nullptr;
    super_chat_t *ret = (super_chat_t *)malloc(sizeof(super_chat_t));
    if (!ret) return nullptr;
    memcpy(ret, p, sizeof(super_chat_t));
    ret->user_name             = strdup_s(p->user_name);
    ret->user_face             = strdup_s(p->user_face);
    ret->message               = strdup_s(p->message);
    ret->message_font_color    = strdup_s(p->message_font_color);
    ret->background_color      = strdup_s(p->background_color);
    ret->background_color_start = strdup_s(p->background_color_start);
    ret->background_color_end  = strdup_s(p->background_color_end);
    ret->fan_medal_name        = strdup_s(p->fan_medal_name);
    ret->fan_medal_liveuser_name = strdup_s(p->fan_medal_liveuser_name);
    return ret;
}

void super_chat_t_free(const super_chat_t *p) {
    free(p->user_name);
    free(p->user_face);
    free(p->message);
    free(p->message_font_color);
    free(p->background_color);
    free(p->background_color_start);
    free(p->background_color_end);
    free(p->fan_medal_name);
    free(p->fan_medal_liveuser_name);
}