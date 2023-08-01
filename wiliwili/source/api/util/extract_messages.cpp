
#include "live/extract_messages.hpp"

#include <nlohmann/json.hpp>

#include <malloc.h>
#include <utility>

std::vector<live_t> extract_messages(const std::vector<std::string>& messages) {

    std::vector<live_t> live_messages;
    live_messages.reserve(messages.size()); // 预留空间

    for (auto& message : messages) {

        nlohmann::json json_message = nlohmann::json::parse(message); 

        auto it = json_message.find("cmd");
        if (it != json_message.end()) {
            
            if (it->get<std::string>() == "WATCHED_CHANGE") {
                auto& num = json_message["data"]["num"];

                if(!num.is_number()) continue;

                watched_change_t *wc = (watched_change_t *)malloc(sizeof(watched_change_t));

                live_messages.emplace_back(live_t{.type = watched_change, .ptr = wc});

            } else if (it->get<std::string>() == "DANMU_MSG") {
                auto& info = json_message["info"];

                if(!info.is_array()) continue;

                danmaku_t *dan = (danmaku_t *)malloc(sizeof(danmaku_t));

                if (info.size() > 0 and info[0].is_array() and info[0].size() > 3) {
                    auto& attribute = info[0];
                    if(attribute[1].is_number())
                        dan->dan_type = attribute[1].get<int>();

                    if(attribute[2].is_number())
                        dan->dan_size = attribute[2].get<int>();

                    if(attribute[3].is_number())
                        dan->dan_color = attribute[3].get<int>();

                }
                if(info.size() > 1 and info[1].is_string()){
                    dan->dan = info[1].get_ref<const std::string&>();
                }
                if(info.size() > 2 and info[2].is_array() and info[2].size() == 8) {
                    auto& user = info[2];
                    if(user[0].is_number())
                        dan->user_uid = user[0].get<int>();

                    if(user[1].is_string())
                        dan->user_name = user[1].get_ref<std::string&>();

                    if(user[2].is_number())
                        dan->is_guard = user[2].get<int>();

                    if(user[7].is_string()) 
                        dan->user_name_color = user[7].get_ref<std::string&>();
                }
                if(info.size() > 3 and info[3].is_array() and info[3].size() == 13){
                    auto& fan = info[3];
                    if(fan[0].is_number())
                        dan->fan_medal_level = fan[0].get<int>();

                    if(fan[1].is_string()) 
                        dan->fan_medal_name = fan[1].get_ref<std::string&>();

                    if(fan[2].is_string()) 
                        dan->fan_medal_liveuser_name = fan[2].get_ref<std::string&>();

                    if(fan[3].is_number())
                        dan->fan_medal_roomid = fan[3].get<int>();

                    if(fan[6].is_number())
                        dan->fan_medal_font_color = fan[6].get<int>();

                    if(fan[7].is_number())
                        dan->fan_medal_border_color = fan[7].get<int>();

                    if(fan[8].is_number())
                        dan->fan_medal_end_color = fan[8].get<int>();

                    if(fan[9].is_number())
                        dan->fan_medal_start_color = fan[9].get<int>();

                    if(fan[10].is_number())
                        dan->fan_medal_vip_level = fan[10].get<int>();

                    if(fan[12].is_number())
                        dan->fan_medal_liveuser_uid = fan[12].get<int>();

                }
                if(info.size() > 4 and info[4].is_array() and info[4].size() > 0){
                    if(info[4][0].is_number())
                        dan->user_level = info[4][0].get<int>();
                }
                if(info.size() > 7 and info[7].is_number()){
                    dan->user_vip_level = info[7].get<int>();
                }
                live_messages.emplace_back(live_t{.type = danmaku, .ptr = dan});
            }
        }

    }
    return live_messages;
}