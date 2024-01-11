//
// Created by fang on 2022/8/24.
//

#include <cpr/cpr.h>

#include "bilibili.h"
#include "presenter/pgc_index.hpp"
#include <pystring.h>
#include "borealis/core/logger.hpp"

void PGCIndexRequest::onPGCIndex(const bilibili::PGCIndexResultWrapper& result) {}

void PGCIndexRequest::onPGCFilter(const bilibili::PGCIndexFilters& result) {}

void PGCIndexRequest::onError(const std::string& error) {}

void PGCIndexRequest::requestData(UserRequestData data, bool refresh) {
    auto parameters = cpr::Parameters();
    for (auto& d : data) {
        auto value = d.second;
        parameters.Add(cpr::Parameter(d.first, d.second));
    }
    if (refresh) {
        this->requestIndex = 1;
    }
    this->requestPGCIndex(parameters.GetContent(cpr::CurlHolder()), this->requestIndex);
}

void PGCIndexRequest::requestPGCIndex(const std::string& param, int page) {
    ASYNC_RETAIN
    BILI::get_pgc_index(
        param, page,
        [ASYNC_TOKEN](const bilibili::PGCIndexResultWrapper& result) {
            ASYNC_RELEASE
            if (result.num != this->requestIndex) {
                brls::Logger::error("获取视频列表 错误的index: {} 请求的index: {}", result.num, this->requestIndex);
                return;
            }
            this->requestIndex++;
            this->onPGCIndex(result);
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            this->onError(error);
        });
}

void PGCIndexRequest::requestPGCFilter() {
    ASYNC_RETAIN
    BILI::get_pgc_all_filter(
        [ASYNC_TOKEN](const bilibili::PGCIndexFilters& result) {
            ASYNC_RELEASE
            INDEX_FILTERS = result;
            this->onPGCFilter(result);
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            this->onError(error);
        });
}