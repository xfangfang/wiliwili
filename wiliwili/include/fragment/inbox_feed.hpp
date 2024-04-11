#pragma once

#include "view/auto_tab_frame.hpp"
#include "presenter/inbox_feed.hpp"

class RecyclingGrid;

class InboxFeed : public AttachedView, public InboxFeedRequest {
public:
    InboxFeed();

    ~InboxFeed() override;

    void onCreate() override;

    static View* create();

    void setMode(MsgFeedMode mode);

    void onFeedReplyList(const bilibili::FeedReplyResultWrapper& result) override;

    void onFeedAtList(const bilibili::FeedAtResultWrapper& result) override;

    void onFeedLikeList(const bilibili::FeedLikeResultWrapper& result) override;

    void onError(const std::string& error) override;

private:
    BRLS_BIND(RecyclingGrid, recyclingGrid, "inbox/feedList");

    MsgFeedMode feedMode;
};