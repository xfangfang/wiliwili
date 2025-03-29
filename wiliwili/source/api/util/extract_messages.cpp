#include "live/extract_messages.hpp"

#include <nlohmann/json.hpp>

namespace message {

// Danmaku类的实现
Danmaku::Danmaku() {
    // 默认值已在头文件中设置
}

Danmaku::Danmaku(const Danmaku& other) {
    user_name = other.user_name;
    user_name_color = other.user_name_color;
    dan = other.dan;
    fan_medal_name = other.fan_medal_name;
    fan_medal_liveuser_name = other.fan_medal_liveuser_name;
    
    user_uid = other.user_uid;
    dan_color = other.dan_color;
    fan_medal_roomid = other.fan_medal_roomid;
    fan_medal_font_color = other.fan_medal_font_color;
    fan_medal_border_color = other.fan_medal_border_color;
    fan_medal_start_color = other.fan_medal_start_color;
    fan_medal_end_color = other.fan_medal_end_color;
    fan_medal_liveuser_uid = other.fan_medal_liveuser_uid;
    
    is_emoticon = other.is_emoticon;
    dan_type = other.dan_type;
    dan_size = other.dan_size;
    user_level = other.user_level;
    user_vip_level = other.user_vip_level;
    fan_medal_level = other.fan_medal_level;
    fan_medal_vip_level = other.fan_medal_vip_level;
    is_guard = other.is_guard;
    glory_v = other.glory_v;
}

// SuperChat类的实现
SuperChat::SuperChat() {
    // 默认值已在头文件中设置
}

SuperChat::SuperChat(const SuperChat& other) {
    user_name = other.user_name;
    user_face = other.user_face;
    message = other.message;
    message_font_color = other.message_font_color;
    background_color = other.background_color;
    background_color_start = other.background_color_start;
    background_color_end = other.background_color_end;
    fan_medal_name = other.fan_medal_name;
    fan_medal_liveuser_name = other.fan_medal_liveuser_name;
    
    user_uid = other.user_uid;
    price = other.price;
    time = other.time;
    fan_medal_level = other.fan_medal_level;
}

// InteractWord类的实现
InteractWord::InteractWord() {
    // 默认值已在头文件中设置
}

} // namespace message

// 解析消息函数
std::vector<std::shared_ptr<message::LiveMessage>> extract_messages(const std::vector<std::string>& messages) {
    std::vector<std::shared_ptr<message::LiveMessage>> live_messages;
    live_messages.reserve(messages.size() / 5);

    for (const auto& message : messages) {
        nlohmann::json json_message;

        try {
            json_message = nlohmann::json::parse(message);
        } catch (const std::exception& e) {
            continue;
        } catch (...) {
            continue;
        }

        auto it = json_message.find("cmd");
        if (it == json_message.end()) continue;

        const std::string& cmd = it->get_ref<const std::string&>();

        // 处理超级留言
        if (cmd == "SUPER_CHAT_MESSAGE" || cmd == "SUPER_CHAT_MESSAGE_JPN") {
            if (!json_message.contains("data")) continue;
            auto& data = json_message["data"];

            auto sc = std::make_shared<message::SuperChat>();

            // 获取消息内容
            if (data.contains("message") && data["message"].is_string()) {
                sc->message = data["message"].get<std::string>();
            }

            // 获取消息字体颜色
            if (data.contains("message_font_color") && data["message_font_color"].is_string()) {
                sc->message_font_color = data["message_font_color"].get<std::string>();
            }

            // 获取背景颜色
            if (data.contains("background_color") && data["background_color"].is_string()) {
                sc->background_color = data["background_color"].get<std::string>();
            }
            
            // 获取背景渐变色（开始）
            if (data.contains("background_color_start") && data["background_color_start"].is_string()) {
                sc->background_color_start = data["background_color_start"].get<std::string>();
            }
            
            // 获取背景渐变色（结束）
            if (data.contains("background_color_end") && data["background_color_end"].is_string()) {
                sc->background_color_end = data["background_color_end"].get<std::string>();
            }
            
            // 获取价格背景颜色
            if (data.contains("background_price_color") && data["background_price_color"].is_string()) {
                sc->background_price_color = data["background_price_color"].get<std::string>();
            }

            // 获取金额
            if (data.contains("price") && data["price"].is_number()) {
                sc->price = data["price"].get<int>();
            }

            // 获取持续时间
            if (data.contains("time") && data["time"].is_number()) {
                sc->time = data["time"].get<int>();
            }
            
            // 获取开始时间
            if (data.contains("start_time") && data["start_time"].is_number()) {
                sc->start_time = data["start_time"].get<int64_t>();
            }
            
            // 获取结束时间
            if (data.contains("end_time") && data["end_time"].is_number()) {
                sc->end_time = data["end_time"].get<int64_t>();
            }

            // 获取用户信息
            if (data.contains("user_info")) {
                auto& user_info = data["user_info"];
                
                // 用户名
                if (user_info.contains("uname") && user_info["uname"].is_string()) {
                    sc->user_name = user_info["uname"].get<std::string>();
                }
                
                // 用户头像
                if (user_info.contains("face") && user_info["face"].is_string()) {
                    sc->user_face = user_info["face"].get<std::string>();
                }
            }

            // 获取UID
            if (data.contains("uid") && data["uid"].is_number()) {
                sc->user_uid = data["uid"].get<int>();
            }

            // 获取粉丝牌信息
            if (data.contains("medal_info")) {
                auto& medal_info = data["medal_info"];
                
                // 粉丝牌名称
                if (medal_info.contains("medal_name") && medal_info["medal_name"].is_string()) {
                    sc->fan_medal_name = medal_info["medal_name"].get<std::string>();
                }
                
                // 粉丝牌等级
                if (medal_info.contains("medal_level") && medal_info["medal_level"].is_number()) {
                    sc->fan_medal_level = medal_info["medal_level"].get<int>();
                }
                
                // 粉丝牌主播名称
                if (medal_info.contains("anchor_uname") && medal_info["anchor_uname"].is_string()) {
                    sc->fan_medal_liveuser_name = medal_info["anchor_uname"].get<std::string>();
                }
            }

            live_messages.push_back(std::make_shared<message::LiveSuperChat>(sc));
            continue;
        }

        // 处理在线人数更新
        if (cmd == "WATCHED_CHANGE") {
            if (!json_message.contains("data") || !json_message["data"].contains("num")) continue;
            
            auto& data = json_message["data"];
            if (!data["num"].is_number()) continue;
            
            int num = data["num"].get<int>();
            live_messages.push_back(std::make_shared<message::LiveWatchedChange>(num));
            continue;
        }

        // 处理弹幕消息
        if (cmd == "DANMU_MSG") {
            auto& info = json_message["info"];
            if (!info.is_array() || info.size() < 17) continue;

            auto dan = std::make_shared<message::Danmaku>();

            // 处理弹幕属性
            if (info[0].is_array() && info[0].size() > 12) {
                auto& attribute = info[0];
                if (attribute[1].is_number()) dan->dan_type = attribute[1].get<int>();
                if (attribute[2].is_number()) dan->dan_size = attribute[2].get<int>();
                if (attribute[3].is_number()) dan->dan_color = attribute[3].get<int>();
                if (attribute[12].is_number()) dan->is_emoticon = attribute[12].get<int>();
            }

            // 处理弹幕内容
            if (info[1].is_string()) {
                dan->dan = info[1].get<std::string>();
            }

            // 处理用户信息
            if (info[2].is_array() && info[2].size() == 8) {
                auto& user = info[2];
                if (user[0].is_number()) dan->user_uid = user[0].get<int>();
                if (user[1].is_string()) dan->user_name = user[1].get<std::string>();
                if (user[2].is_number()) dan->is_guard = user[2].get<int>();
                if (user[7].is_string()) dan->user_name_color = user[7].get<std::string>();
            }

            // 处理粉丝牌信息
            if (info[3].is_array() && info[3].size() == 13) {
                auto& fan = info[3];
                if (fan[0].is_number()) dan->fan_medal_level = fan[0].get<int>();
                if (fan[1].is_string()) dan->fan_medal_name = fan[1].get<std::string>();
                if (fan[2].is_string()) dan->fan_medal_liveuser_name = fan[2].get<std::string>();
                if (fan[3].is_number()) dan->fan_medal_roomid = fan[3].get<int>();
                if (fan[6].is_number()) dan->fan_medal_font_color = fan[6].get<int>();
                if (fan[7].is_number()) dan->fan_medal_border_color = fan[7].get<int>();
                if (fan[8].is_number()) dan->fan_medal_end_color = fan[8].get<int>();
                if (fan[9].is_number()) dan->fan_medal_start_color = fan[9].get<int>();
                if (fan[10].is_number()) dan->fan_medal_vip_level = fan[10].get<int>();
                if (fan[12].is_number()) dan->fan_medal_liveuser_uid = fan[12].get<int>();
            }

            // 处理用户等级
            if (info[4].is_array() && info[4].size() > 0) {
                if (info[4][0].is_number()) dan->user_level = info[4][0].get<int>();
            }

            // 处理VIP等级
            if (info[7].is_number()) {
                dan->user_vip_level = info[7].get<int>();
            }

            live_messages.push_back(std::make_shared<message::LiveDanmaku>(dan));
            continue;
        }
    }
    
    return live_messages;
}