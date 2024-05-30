#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

#include "presenter/inbox_chat.hpp"

class ButtonClose;
class RecyclingGrid;
class AutoTabFrame;

class InboxView : public brls::Box, public InboxChatRequest {
public:
    InboxView();
    ~InboxView();

    bool isTranslucent() override;

    View* getDefaultFocus() override;

    void onChatList(const bilibili::InboxChatListResult& result, bool refresh) override;

    void onError(const std::string& error) override;

private:
    BRLS_BIND(ButtonClose, closebtn, "inbox/close");
    BRLS_BIND(brls::Box, cancel, "player/cancel");
    BRLS_BIND(AutoTabFrame, tabFrame, "inbox/tab/frame");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "inbox/chatList");
};