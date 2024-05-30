//
// Created by fang on 2022/8/18.
//

#pragma once

#include "bilibili/result/inbox_result.h"
#include "presenter.h"

enum class MsgFeedMode
{
    REPLY,
    AT,
    LIKE
};

class InboxFeedRequest : public Presenter {
public:
    virtual void onFeedReplyList(const bilibili::FeedReplyResultWrapper& result, bool refresh);

    virtual void onFeedAtList(const bilibili::FeedAtResultWrapper& result, bool refresh);

    virtual void onFeedLikeList(const bilibili::FeedLikeResultWrapper& result, bool refresh);

    virtual void onError(const std::string& error);

    void requestData(MsgFeedMode mode, bool refresh = false);

protected:
    bilibili::MsgFeedCursor cursor{};
};