#include "bilibili.h"
#include "presenter/inbox_msg.hpp"

void InboxMsgRequest::onMsgList(const bilibili::InboxMessageResultWrapper& result, bool refresh) {}

void InboxMsgRequest::onError(const std::string& error) {}

void InboxMsgRequest::requestData(bool refresh, int session_type) {
    BILI::fetch_inbox_msgs(
        std::to_string(this->talkerId), 20, session_type, std::to_string(this->msgSeq),
        [this, refresh](const bilibili::InboxMessageResultWrapper& result) {
            this->onMsgList(result, refresh);
            this->msgSeq = result.max_seqno;
        },
        [this](BILI_ERR) { this->onError(error); });
}

void InboxMsgRequest::setTalkerId(uint64_t mid) { this->talkerId = mid; }