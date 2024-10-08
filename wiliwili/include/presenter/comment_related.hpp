//
// Created by fang on 2022/1/7.
//

#pragma once

#include "presenter/presenter.h"
#include "bilibili/result/video_detail_result.h"

class CommentAction : public Presenter {
public:
    virtual void onError(const std::string& error);

    void commentLike(const std::string& oid, uint64_t rpid, bool action, int type);

    void commentDislike(const std::string& oid, uint64_t rpid, bool action, int type);

    void commentReply(const std::string& text, const std::string& oid, uint64_t rpid, uint64_t root, int type,
                      const std::function<void(const bilibili::VideoCommentAddResult&)>& cb = nullptr);

    void commentDelete(const std::string& oid, uint64_t rpid, int type);
};

class CommentRequest : public Presenter {
public:
    virtual void onCommentInfo(const bilibili::VideoCommentResultWrapper& result) {}
    virtual void onRequestCommentError(const std::string& error) {}

    /**
     * 获取评论
     * @param next -1 自动加载下一页；0 加载首页
     * @param mode -1 按上次使用的顺序；2 按时间排序；3 按热度排序
     * @param type 评论类型，1 视频评论；11 图片动态评论；17 文字动态评论
     */
    void requestVideoComment(const std::string& oid, int next = -1, int mode = -1, int type = 1);

    int getVideoCommentMode() const;

    void setVideoCommentMode(int mode);

protected:
    int commentRequestIndex = 0;
    int commentMode         = 3;
    bool end                = false;
};

class DynamicAction : public Presenter {
public:
    virtual void onError(const std::string& error);

    void dynamicLike(const std::string& id, bool action);
};