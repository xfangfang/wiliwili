#include "bilibili.h"
#include "presenter/inbox_feed.hpp"

void InboxFeedRequest::onFeedReplyList(const bilibili::FeedReplyResultWrapper& result) {}

void InboxFeedRequest::onFeedAtList(const bilibili::FeedAtResultWrapper& result) {}

void InboxFeedRequest::onFeedLikeList(const bilibili::FeedLikeResultWrapper& result) {}

void InboxFeedRequest::onError(const std::string& error) {}

void InboxFeedRequest::requestData(MsgFeedMode mode, bool refresh) {
    switch (mode) {
        case MsgFeedMode::REPLY:
            CHECK_AND_SET_REQUEST
            BILI::msg_feed_reply(
                this->cursor,
                [this](const bilibili::FeedReplyResultWrapper& result) {
                    this->onFeedReplyList(result);
                    UNSET_REQUEST
                },
                [this](BILI_ERR) {
                    this->onError(error);
                    UNSET_REQUEST
                });
            break;
        case MsgFeedMode::AT:
            CHECK_AND_SET_REQUEST
            BILI::msg_feed_at(
                this->cursor,
                [this](const bilibili::FeedAtResultWrapper& result) {
                    this->onFeedAtList(result);
                    UNSET_REQUEST
                },
                [this](BILI_ERR) {
                    this->onError(error);
                    UNSET_REQUEST
                });
            break;
        case MsgFeedMode::LIKE:
            CHECK_AND_SET_REQUEST
            BILI::msg_feed_like(
                this->cursor,
                [this](const bilibili::FeedLikeResultWrapper& result) {
                    this->onFeedLikeList(result);
                    UNSET_REQUEST
                },
                [this](BILI_ERR) {
                    this->onError(error);
                    UNSET_REQUEST
                });
            break;
        default:;
    }
}