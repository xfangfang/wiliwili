#include "bilibili.h"
#include "presenter/inbox_msg.hpp"
#include "utils/config_helper.hpp"
#include <borealis/core/thread.hpp>

void InboxMsgRequest::onMsgList(const bilibili::InboxMessageResultWrapper& result, bool refresh) {}

void InboxMsgRequest::onSendMsg(const bilibili::InboxSendResult& result) {}

void InboxMsgRequest::onError(const std::string& error) {}

void InboxMsgRequest::requestData(bool refresh, int session_type, size_t size) {
    CHECK_AND_SET_REQUEST
    ASYNC_RETAIN
    BILI::fetch_inbox_msgs(
        std::to_string(this->talkerId), size, session_type, refresh ? "" : std::to_string(this->msgSeq),
        [ASYNC_TOKEN, refresh, session_type](const bilibili::InboxMessageResultWrapper& result) {
            brls::sync([ASYNC_TOKEN, refresh, session_type, result](){
                ASYNC_RELEASE
                this->onMsgList(result, refresh);
                if (this->msgSeq != result.max_seqno) {
                    this->msgSeq = result.max_seqno;
                    this->updateAck(session_type);
                }
                UNSET_REQUEST
            });

        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::sync([ASYNC_TOKEN, error](){
                ASYNC_RELEASE
                this->onError(error);
                UNSET_REQUEST
            });
        });
}

void InboxMsgRequest::setTalkerId(uint64_t mid) { this->talkerId = mid; }

void InboxMsgRequest::setMsgSeq(uint64_t seq) { this->msgSeq = seq; }

void InboxMsgRequest::sendMsg(const std::string& text) {
    CHECK_AND_SET_REQUEST
    ASYNC_RETAIN
    std::string csrf       = ProgramConfig::instance().getCSRF();
    nlohmann::json content = {{"content", text}};
    BILI::send_inbox_msg(
        ProgramConfig::instance().getUserID(), std::to_string(this->talkerId), content.dump(), csrf,
        [ASYNC_TOKEN](const bilibili::InboxSendResult& result) {
            brls::sync([ASYNC_TOKEN, result](){
                ASYNC_RELEASE
                this->onSendMsg(result);
                UNSET_REQUEST
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::sync([ASYNC_TOKEN, error](){
                ASYNC_RELEASE
                this->onError(error);
                UNSET_REQUEST
            });
        });
}

void InboxMsgRequest::updateAck(int session_type) {
    BILI::update_inbox_ack(
        std::to_string(this->talkerId), session_type, std::to_string(this->msgSeq), ProgramConfig::instance().getCSRF());
}