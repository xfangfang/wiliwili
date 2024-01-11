//
// Created by fang on 2022/1/7.
//

#pragma once

#include <borealis/core/thread.hpp>

#include "bilibili.h"
#include "bilibili/result/home_live_result.h"
#include "presenter/presenter.h"
#include "utils/config_helper.hpp"
#include "presenter/video_detail.hpp"
#include "bilibili/result/video_detail_result.h"

class CommentRequest : public Presenter {
public:
    virtual void onError(const std::string& error) { DialogHelper::showDialog(error); }

    void commentLike(size_t oid, int64_t rpid, int action) {
        ASYNC_RETAIN
        BILI::be_agree_comment(
            ProgramConfig::instance().getCSRF(), oid, rpid, action,
            [ASYNC_TOKEN, oid, rpid, action]() {
                ASYNC_RELEASE
                brls::Logger::debug("Comment action success: {} {} {}", oid, rpid, action);
            },
            [ASYNC_TOKEN, oid, rpid, action](BILI_ERR) {
                brls::Logger::error("Comment action error: {} {} {}", oid, rpid, action);
                brls::sync([ASYNC_TOKEN, error]() {
                    ASYNC_RELEASE
                    this->onError(error);
                });
            });
    }

    void commentReply(const std::string& text, size_t oid, int64_t rpid, int64_t root,
                      std::function<void(const bilibili::VideoCommentAddResult&)> cb = nullptr) {
        ASYNC_RETAIN
        BILI::add_comment(
            ProgramConfig::instance().getCSRF(), text, oid, rpid, root,
            [ASYNC_TOKEN, cb](const bilibili::VideoCommentAddResult& result) {
                brls::sync([ASYNC_TOKEN, result, cb]() {
                    ASYNC_RELEASE
                    cb(result);
                });
            },
            [ASYNC_TOKEN](BILI_ERR) {
                brls::Logger::error("Comment action error: {}", error);
                brls::sync([ASYNC_TOKEN, error]() {
                    ASYNC_RELEASE
                    this->onError(error);
                });
            });
    }

    void commentDelete(size_t oid, int64_t rpid) {
        ASYNC_RETAIN
        BILI::delete_comment(
            ProgramConfig::instance().getCSRF(), oid, rpid,
            [ASYNC_TOKEN, oid, rpid]() {
                ASYNC_RELEASE
                brls::Logger::debug("Comment delete success: {} {}", oid, rpid);
            },
            [ASYNC_TOKEN, oid, rpid](BILI_ERR) {
                brls::Logger::error("Comment action error: {} {}", oid, rpid);
                brls::sync([ASYNC_TOKEN, error]() {
                    ASYNC_RELEASE
                    this->onError(error);
                });
            });
    }
};