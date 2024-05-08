//
// Created by fang on 2022/8/18.
//

#pragma once

#include "bilibili/result/inbox_result.h"
#include "presenter.h"

class InboxMsgRequest : public Presenter {
public:
    virtual void onMsgList(const bilibili::InboxMessageResultWrapper& result, bool refresh);

    virtual void onSendMsg(const bilibili::InboxSendResult& result);

    virtual void onError(const std::string& error);

    void setTalkerId(uint64_t mid);

    void setMsgSeq(uint64_t seq);

    void requestData(bool refresh = false, int session_type = 1, size_t size = 20);

    void updateAck(int session_type = 1);

    void sendMsg(const std::string& text);

protected:
    uint64_t talkerId = 0;
    uint64_t msgSeq   = 0;
};