//
// Created by fang on 2024/3/23.
//

#include <borealis/core/thread.hpp>
#include <borealis/core/application.hpp>
#include <utility>

#include "bilibili.h"
#include "presenter/comment_related.hpp"
#include "utils/dialog_helper.hpp"
#include "utils/config_helper.hpp"

/// CommentAction

void CommentAction::onError(const std::string& error) { brls::Application::notify(error); }

void CommentAction::commentLike(const std::string& oid, uint64_t rpid, bool action, int type) {
    ASYNC_RETAIN
    BILI::be_agree_comment(
        ProgramConfig::instance().getCSRF(), oid, rpid, action, type,
        [ASYNC_TOKEN, oid, rpid, action]() {
            ASYNC_RELEASE
            brls::Logger::debug("Comment like success: {} {} {}", oid, rpid, action);
        },
        [ASYNC_TOKEN, oid, rpid, action](BILI_ERR) {
            brls::Logger::error("Comment like error: {} {} {}", oid, rpid, action);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->onError(error);
            });
        });
}

void CommentAction::commentDislike(const std::string& oid, uint64_t rpid, bool action, int type) {
    ASYNC_RETAIN
    BILI::be_disagree_comment(
        ProgramConfig::instance().getCSRF(), oid, rpid, action, type,
        [ASYNC_TOKEN, oid, rpid, action]() {
            ASYNC_RELEASE
            brls::Logger::debug("Comment dislike success: {} {} {}", oid, rpid, action);
        },
        [ASYNC_TOKEN, oid, rpid, action](BILI_ERR) {
            brls::Logger::error("Comment dislike error: {} {} {}", oid, rpid, action);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->onError(error);
            });
        });
}

void CommentAction::commentReply(const std::string& text, const std::string& oid, uint64_t rpid, uint64_t root, int type,
                                 const std::function<void(const bilibili::VideoCommentAddResult&)>& cb) {
    ASYNC_RETAIN
    BILI::add_comment(
        ProgramConfig::instance().getCSRF(), text, oid, rpid, root, type,
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

void CommentAction::commentDelete(const std::string& oid, uint64_t rpid, int type) {
    ASYNC_RETAIN
    BILI::delete_comment(
        ProgramConfig::instance().getCSRF(), oid, rpid, type,
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

/// CommentRequest

void CommentRequest::requestVideoComment(const std::string& oid, int next, int mode, int type) {
    if (mode != -1) {
        this->setVideoCommentMode(mode);
    }
    if (next >= 0) {
        this->commentRequestIndex = next;
        // 指定页加载时，重置end状态
        this->end = false;
    }
    brls::Logger::debug("请求评论: {}; 类型: {}; 模式: {}; 页号: {}", oid, type, commentMode, commentRequestIndex);
    if (this->end) {
        brls::Logger::debug("评论已加载完毕，取消加载: {}", oid);
        return;
    }
    ASYNC_RETAIN
    BILI::get_comment(
        oid, commentRequestIndex, getVideoCommentMode(), type,
        [ASYNC_TOKEN, oid](const bilibili::VideoCommentResultWrapper& result) {
            brls::sync([ASYNC_TOKEN, oid, result]() {
                ASYNC_RELEASE
                if (this->commentRequestIndex != (int)result.requestIndex) {
                    brls::Logger::error("request comment {}/{} got: {}", oid, commentRequestIndex, result.requestIndex);
                    return;
                }
                this->end = result.cursor.is_end;
                if (!result.cursor.is_end) {
                    this->commentRequestIndex = result.cursor.next;
                }
                this->onCommentInfo(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("request comment error: {}", error);
            this->onRequestCommentError(error);
        });
}

int CommentRequest::getVideoCommentMode() const { return commentMode; }

void CommentRequest::setVideoCommentMode(int mode) {
    this->commentMode         = mode;
    this->commentRequestIndex = 0;
    this->end                 = false;
}

void DynamicAction::onError(const std::string& error) {}

void DynamicAction::dynamicLike(const std::string& id, bool action) {
    ASYNC_RETAIN
    BILI::be_agree_dynamic(
        ProgramConfig::instance().getCSRF(), id, action,
        [ASYNC_TOKEN, id, action]() {
            ASYNC_RELEASE
            brls::Logger::debug("Dynamic action success: {} {}", id, action);
        },
        [ASYNC_TOKEN, id, action](BILI_ERR) {
            brls::Logger::error("Comment action error: {} {}", id, action);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->onError(error);
            });
        });
}