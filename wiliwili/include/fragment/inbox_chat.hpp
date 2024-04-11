#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

#include "presenter/inbox_msg.hpp"

class RecyclingGrid;

class InboxChat : public brls::Box, public InboxMsgRequest {
public:
    InboxChat(uint64_t talker_id, int session_type);

    ~InboxChat() override;

    void onMsgList(const bilibili::InboxMessageResultWrapper& result, bool refresh) override;

    void onError(const std::string& error) override;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "inbox/msgList");
};