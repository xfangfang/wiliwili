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

void DynamicVideoRequest::onError(const std::string &error) {}

void DynamicVideoRequest::setCurrentUser(int64_t mid) { this->currentUser = mid; }

void DynamicVideoRequest::requestData(bool refresh) {
    CHECK_REQUEST
    if (refresh) {
        currentPage   = 1;
        currentOffset = "";
    }
    if (this->currentUser == 0) {
        this->requestDynamicVideoList(currentPage, currentOffset);
    } else {
        this->requestUserDynamicVideoList(currentUser, currentPage);
    }
}

void DynamicVideoRequest::requestDynamicVideoList(unsigned int page, const std::string &offset) {
    auto mid = ProgramConfig::instance().getUserID();
    if (mid.empty() || mid == "0") {
        this->onError("wiliwili/home/common/no_login"_i18n);
        return;
    }
    CHECK_AND_SET_REQUEST
    BILI::dynamic_video(
        page, offset,
        [this](const bilibili::DynamicVideoListResultWrapper &result) {
            if (currentPage != result.page) {
                brls::Logger::error("request dynamic video error: current page: {}, got: {}", currentPage, result.page);
                return;
            }
            currentOffset = result.offset;
            currentPage   = result.page + 1;
            this->onDynamicVideoList(result.items, result.page);
            UNSET_REQUEST
        },
        [this](BILI_ERR) {
            this->onError(error);
            UNSET_REQUEST
        });
}

void DynamicVideoRequest::requestUserDynamicVideoList(int64_t mid, int pn, int ps) {
    CHECK_AND_SET_REQUEST
    BILI::get_user_videos2(
        mid, pn, ps,
        [this](const bilibili::UserDynamicVideoResultWrapper &result) {
            if (currentPage != result.page.pn) {
                brls::Logger::error("request dynamic video error: current page: {}, got: {}", currentPage,
                                    result.page.pn);
                return;
            }
            currentPage = result.page.pn + 1;
            this->onDynamicVideoList(result.archives, result.page.pn);
            UNSET_REQUEST
        },
        [this](BILI_ERR) {
            this->onError(error);
            UNSET_REQUEST
        });
}