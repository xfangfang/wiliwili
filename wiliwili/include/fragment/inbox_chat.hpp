#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

#include "presenter/inbox_msg.hpp"

namespace brls {
class Label;
};
class RecyclingGrid;

class InboxChat : public brls::Box, public InboxMsgRequest {
public:
    InboxChat(const bilibili::InboxChatResult& r);

    ~InboxChat() override;

    void onMsgList(const bilibili::InboxMessageResultWrapper& result, bool refresh) override;

    void onSendMsg(const bilibili::InboxSendResult& result) override;

    void onError(const std::string& error) override;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "inbox/msgList");
    BRLS_BIND(brls::Label, labelTalker, "inbox/talker");
    BRLS_BIND(brls::Box, inputReply, "inbox/reply/hint");
};