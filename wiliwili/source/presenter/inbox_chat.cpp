#include "bilibili.h"
#include "presenter/inbox_chat.hpp"
#include "utils/number_helper.hpp"
#include "utils/config_helper.hpp"

void InboxChatRequest::onChatList(const bilibili::InboxChatListResult& result, bool refresh) {}

void InboxChatRequest::onError(const std::string& error) {}

void InboxChatRequest::requestData(bool refresh) {
    CHECK_AND_SET_REQUEST

    BILI::new_inbox_sessions(
        refresh ? 0 : this->last_time,
        [this, refresh](const bilibili::InboxChatResultWrapper& result) {
            std::vector<std::string> uids;
            for (auto& s : result.session_list) {
                if (s.account_info.name.empty()) {
                    uids.push_back(std::to_string(s.talker_id));
                }
            }
            uids.push_back(ProgramConfig::instance().getUserID());
            this->last_time = wiliwili::unix_time() * 1000000;

            BILI::get_user_cards(
                uids,
                [this, result, refresh](const bilibili::UserCardListResult& users) {
                    InboxUserMap user_map;
                    for (auto& u : users) user_map[u.mid] = u;

                    auto list = result.session_list;
                    for (auto& s : list) {
                        auto it = user_map.find(s.talker_id);
                        if (it != user_map.end()) {
                            s.account_info.name    = it->second.name;
                            s.account_info.pic_url = it->second.face;
                        }
                    }
                    this->onChatList(list, refresh);
                    UNSET_REQUEST
                },
                [this](BILI_ERR) {
                    this->onError(error);
                    UNSET_REQUEST
                });
        },
        [this](BILI_ERR) {
            this->onError(error);
            UNSET_REQUEST
        });
}