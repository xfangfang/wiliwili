#include "bilibili.h"
#include "presenter/inbox_feed.hpp"

void InboxFeedRequest::onFeedReplyList(const bilibili::FeedReplyResultWrapper& result) {}

void InboxFeedRequest::onFeedAtList(const bilibili::FeedAtResultWrapper& result) {}

void InboxFeedRequest::onFeedLikeList(const bilibili::FeedLikeResultWrapper& result) {}

void InboxFeedRequest::onError(const std::string& error) {}

void InboxFeedRequest::requestData(MsgFeedMode mode, bool refresh) {
    switch (mode) {
        case MsgFeedMode::REPLY:
            BILI::msg_feed_reply(
                this->cursor, [this](const bilibili::FeedReplyResultWrapper &result) { this->onFeedReplyList(result); },
                [this](BILI_ERR) { this->onError(error); });
            break;
        case MsgFeedMode::AT:
            BILI::msg_feed_at(
                this->cursor, [this](const bilibili::FeedAtResultWrapper &result) { this->onFeedAtList(result); },
                [this](BILI_ERR) { this->onError(error); });
            break;
        case MsgFeedMode::LIKE:
            BILI::msg_feed_like(
                this->cursor, [this](const bilibili::FeedLikeResultWrapper &result) { this->onFeedLikeList(result); },
                [this](BILI_ERR) { this->onError(error); });
            break;
        default:;
    }
}