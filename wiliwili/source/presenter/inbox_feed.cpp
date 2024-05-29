#include <borealis/core/thread.hpp>

#include "bilibili.h"
#include "presenter/inbox_feed.hpp"

void InboxFeedRequest::onFeedReplyList(const bilibili::FeedReplyResultWrapper& result) {}

void InboxFeedRequest::onFeedAtList(const bilibili::FeedAtResultWrapper& result) {}

void InboxFeedRequest::onFeedLikeList(const bilibili::FeedLikeResultWrapper& result) {}

void InboxFeedRequest::onError(const std::string& error) {}

void InboxFeedRequest::requestData(MsgFeedMode mode, bool refresh) {
    switch (mode) {
        case MsgFeedMode::REPLY: {
            ASYNC_RETAIN
            BILI::msg_feed_reply(
                this->cursor,
                [ASYNC_TOKEN](const bilibili::FeedReplyResultWrapper& result) {
                    brls::sync([ASYNC_TOKEN, result]() {
                        ASYNC_RELEASE
                        this->onFeedReplyList(result);
                    });
                },
                [ASYNC_TOKEN](BILI_ERR) {
                    brls::sync([ASYNC_TOKEN, error]() {
                        ASYNC_RELEASE
                        this->onError(error);
                    });
                });
            break;
        }
        case MsgFeedMode::AT: {
            ASYNC_RETAIN
            BILI::msg_feed_at(
                this->cursor,
                [ASYNC_TOKEN](const bilibili::FeedAtResultWrapper& result) {
                    brls::sync([ASYNC_TOKEN, result]() {
                        ASYNC_RELEASE
                        this->onFeedAtList(result);
                    });
                },
                [ASYNC_TOKEN](BILI_ERR) {
                    brls::sync([ASYNC_TOKEN, error]() {
                        ASYNC_RELEASE
                        this->onError(error);
                    });
                });
            break;
        }
        case MsgFeedMode::LIKE: {
            ASYNC_RETAIN
            BILI::msg_feed_like(
                this->cursor,
                [ASYNC_TOKEN](const bilibili::FeedLikeResultWrapper& result) {
                    brls::sync([ASYNC_TOKEN, result]() {
                        ASYNC_RELEASE
                        this->onFeedLikeList(result);
                    });
                },
                [ASYNC_TOKEN](BILI_ERR) {
                    brls::sync([ASYNC_TOKEN, error]() {
                        ASYNC_RELEASE
                        this->onError(error);
                    });
                });
            break;
        }
        default:;
    }
}