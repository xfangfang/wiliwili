#include <borealis/core/thread.hpp>

#include "bilibili.h"
#include "presenter/inbox_feed.hpp"

void InboxFeedRequest::onFeedReplyList(const bilibili::FeedReplyResultWrapper& result, bool refresh) {}

void InboxFeedRequest::onFeedAtList(const bilibili::FeedAtResultWrapper& result, bool refresh) {}

void InboxFeedRequest::onFeedLikeList(const bilibili::FeedLikeResultWrapper& result, bool refresh) {}

void InboxFeedRequest::onError(const std::string& error) {}

void InboxFeedRequest::requestData(MsgFeedMode mode, bool refresh) {
    if (this->cursor.is_end && !refresh) return;
    if (refresh) this->cursor = bilibili::MsgFeedCursor{};
    switch (mode) {
        case MsgFeedMode::REPLY: {
            ASYNC_RETAIN
            BILI::msg_feed_reply(
                this->cursor,
                [ASYNC_TOKEN, refresh](const bilibili::FeedReplyResultWrapper& result) {
                    brls::sync([ASYNC_TOKEN, result, refresh]() {
                        ASYNC_RELEASE
                        this->cursor = result.cursor;
                        this->onFeedReplyList(result, refresh);
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
                [ASYNC_TOKEN, refresh](const bilibili::FeedAtResultWrapper& result) {
                    brls::sync([ASYNC_TOKEN, result, refresh]() {
                        ASYNC_RELEASE
                        this->cursor = result.cursor;
                        this->onFeedAtList(result, refresh);
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
                [ASYNC_TOKEN, refresh](const bilibili::FeedLikeResultWrapper& result) {
                    brls::sync([ASYNC_TOKEN, result, refresh]() {
                        ASYNC_RELEASE
                        this->cursor = result.total.cursor;
                        this->onFeedLikeList(result, refresh);
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