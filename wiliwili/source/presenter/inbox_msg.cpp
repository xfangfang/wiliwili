#include "bilibili.h"
#include "presenter/inbox_msg.hpp"
#include "utils/config_helper.hpp"

void InboxMsgRequest::onMsgList(const bilibili::InboxMessageResultWrapper& result, bool refresh) {}

void InboxMsgRequest::onSendMsg(const bilibili::InboxSendResult& result) {}

void InboxMsgRequest::onError(const std::string& error) {}

void InboxMsgRequest::requestData(bool refresh, int session_type) {
    CHECK_AND_SET_REQUEST
    BILI::fetch_inbox_msgs(
        std::to_string(this->talkerId), 20, session_type, std::to_string(this->msgSeq),
        [this, refresh](const bilibili::InboxMessageResultWrapper& result) {
            this->onMsgList(result, refresh);
            this->msgSeq = result.max_seqno;
            UNSET_REQUEST
        },
        [this](BILI_ERR) {
            this->onError(error);
            UNSET_REQUEST
        });
}

void InboxMsgRequest::setTalkerId(uint64_t mid) { this->talkerId = mid; }

void InboxMsgRequest::sendMsg(const std::string& text) {
    CHECK_AND_SET_REQUEST
    std::string csrf       = ProgramConfig::instance().getCSRF();
    nlohmann::json content = {{"content", text}};
    BILI::send_inbox_msg(
        ProgramConfig::instance().getUserID(), std::to_string(this->talkerId), content.dump(), csrf,
        [this](const bilibili::InboxSendResult& result) {
            this->onSendMsg(result);
            UNSET_REQUEST
        },
        [this](BILI_ERR) {
            this->onError(error);
            UNSET_REQUEST
        });
}