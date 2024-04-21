//
// Created by fang on 2022/8/18.
//

#include <borealis/core/i18n.hpp>

#include "bilibili.h"
#include "bilibili/result/mine_result.h"
#include "presenter/dynamic_video.hpp"
#include "utils/config_helper.hpp"

using namespace brls::literals;

void DynamicVideoRequest::onDynamicVideoList(const bilibili::DynamicVideoListResult &result, unsigned int index) {}

void DynamicVideoRequest::onDynamicArticleList(const bilibili::DynamicArticleListResult &result, unsigned int index) {}

void DynamicVideoRequest::onVideoError(const std::string &error) {}

void DynamicVideoRequest::onArticleError(const std::string &error) {}

void DynamicVideoRequest::setCurrentUser(int64_t mid) { this->currentUser = mid; }

void DynamicVideoRequest::requestData(bool refresh, DynamicRequestMode mode) {
    if (refresh) {
        currentVideoPage     = 1;
        currentVideoOffset   = "";
        currentArticlePage   = 1;
        currentArticleOffset = "";
    }
    if (mode == DynamicRequestMode::Article) {
        // 图文
        this->requestDynamicArticleList(currentArticlePage, currentArticleOffset);
    } else {
        // 视频
        this->requestVideoData(currentVideoPage, currentVideoOffset, currentUser);
    }
}

void DynamicVideoRequest::requestVideoData(unsigned int page, const std::string &offset, int64_t mid) {
    if (mid == 0) {
        this->requestDynamicVideoList(page, offset);
    } else {
        this->requestUserDynamicVideoList(mid, page);
    }
}

void DynamicVideoRequest::requestDynamicVideoList(unsigned int page, const std::string &offset) {
    auto mid = ProgramConfig::instance().getUserID();
    if (mid.empty() || mid == "0") {
        this->onVideoError("wiliwili/home/common/no_login"_i18n);
        return;
    }
    brls::Logger::debug("request dynamic video list: user: {}; page: {}; offset: {}", currentUser, page, offset);
    BILI::dynamic_video(
        page, offset,
        [this](const bilibili::DynamicVideoListResultWrapper &result) {
            if (currentVideoPage != result.page) {
                brls::Logger::error("request dynamic video error: current page: {}, got: {}", currentVideoPage,
                                    result.page);
                return;
            }
            currentVideoOffset = result.offset;
            currentVideoPage   = result.page + 1;
            this->onDynamicVideoList(result.items, result.page);
        },
        [this](BILI_ERR) { this->onVideoError(error); });
}

void DynamicVideoRequest::requestDynamicArticleList(unsigned int page, const std::string &offset) {
    auto mid = ProgramConfig::instance().getUserID();
    if (mid.empty() || mid == "0") {
        this->onArticleError("wiliwili/home/common/no_login"_i18n);
        return;
    }
    brls::Logger::debug("request dynamic article list: user: {}; page: {}; offset: {}", currentUser, page, offset);
    BILI::dynamic_article(
        page, offset, currentUser,
        [this](const bilibili::DynamicArticleListResultWrapper &result) {
            if (currentArticlePage != result.page) {
                brls::Logger::error("request dynamic video error: current page: {}, got: {}", currentArticlePage,
                                    result.page);
                return;
            }
            currentArticleOffset = result.offset;
            currentArticlePage   = result.page + 1;
            this->onDynamicArticleList(result.items, result.page);
        },
        [this](BILI_ERR) { this->onArticleError(error); });
}

void DynamicVideoRequest::requestUserDynamicVideoList(int64_t mid, int pn, int ps) {
    brls::Logger::debug("request dynamic video list: user: {}; page: {}", currentUser, pn);
    BILI::get_user_videos2(
        mid, pn, ps,
        [this](const bilibili::UserDynamicVideoResultWrapper &result) {
            if (currentVideoPage != result.page.pn) {
                brls::Logger::error("request dynamic video error: current page: {}, got: {}", currentVideoPage,
                                    result.page.pn);
                return;
            }
            currentVideoPage = result.page.pn + 1;
            this->onDynamicVideoList(result.archives, result.page.pn);
        },
        [this](BILI_ERR) {
            this->onVideoError(error);
        });
}