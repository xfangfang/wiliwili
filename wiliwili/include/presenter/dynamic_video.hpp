//
// Created by fang on 2022/8/18.
//

#pragma once

#include "bilibili/result/dynamic_video.h"
#include "presenter.h"

enum class DynamicRequestMode {
    Video,
    Article,
};
class DynamicVideoRequest : public Presenter {
public:
    virtual void onDynamicVideoList(const bilibili::DynamicVideoListResult& result, unsigned int index);

    virtual void onDynamicArticleList(const bilibili::DynamicArticleListResult &result, unsigned int index);

    virtual void onVideoError(const std::string& error);

    virtual void onArticleError(const std::string& error);

    void setCurrentUser(int64_t mid);

    void requestData(bool refresh = false, DynamicRequestMode mode = DynamicRequestMode::Video);

    void requestDynamicVideoList(unsigned int page = 1, const std::string& offset = "");

    void requestDynamicArticleList(unsigned int page = 1, const std::string& offset = "");

    void requestUserDynamicVideoList(int64_t mid, int pn = 0, int ps = 30);

protected:
    int64_t currentUser       = 0;
    unsigned int currentVideoPage = 1, currentArticlePage = 1;
    std::string currentVideoOffset, currentArticleOffset;

    void requestVideoData(unsigned int page, const std::string& offset, int64_t mid);
};