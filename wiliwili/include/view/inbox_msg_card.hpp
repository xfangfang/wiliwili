#pragma once

#include "view/recycling_grid.hpp"
#include "bilibili/result/inbox_result.h"

class TextBox;

typedef std::shared_ptr<bilibili::InboxEmote> InboxEmotePtr;
typedef std::unordered_map<std::string, InboxEmotePtr> IEMap;

class InboxMsgCard : public RecyclingGridItem {
public:
    InboxMsgCard();

    void setCard(const bilibili::InboxMessageResult& r, const IEMap& m, uint64_t talker);

    void setAvatar(const std::string& face);

private:
    BRLS_BIND(TextBox, textBox, "msg/content");
    BRLS_BIND(brls::Box, msgBox, "msg/content_box");
    BRLS_BIND(brls::Image, talker, "avatar/talker");
    BRLS_BIND(brls::Image, mine, "avatar/mine");
};
