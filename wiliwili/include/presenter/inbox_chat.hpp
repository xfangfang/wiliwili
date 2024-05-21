//
// Created by fang on 2022/8/18.
//

#pragma once

#include "bilibili/result/inbox_result.h"
#include "presenter.h"

class InboxChatRequest : public Presenter {
public:
    virtual void onChatList(const bilibili::InboxChatListResult& result, bool refresh);

    virtual void onError(const std::string& error);

    void requestData(bool refresh = false);

protected:
    time_t last_time = 0;

    std::unordered_map<unsigned int, bilibili::UserCardResult> user_map;
};