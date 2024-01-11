//
// Created by fang on 2023/1/18.
//

#include <borealis/core/i18n.hpp>
#include <borealis/core/thread.hpp>

#include "bilibili.h"
#include "presenter/mine_bangumi.hpp"
#include "utils/config_helper.hpp"

using namespace brls::literals;

MineBangumiRequest::MineBangumiRequest() {
    bangumiCollection.ps = 20;
    bangumiCollection.pn = 1;
    requestType          = 1;
    requestEnd           = false;
}

void MineBangumiRequest::setRequestType(size_t type) { requestType = type; }

void MineBangumiRequest::requestData(bool refresh) {
    CHECK_REQUEST
    if (refresh) {
        bangumiCollection.ps = 20;
        bangumiCollection.pn = 1;
        requestEnd           = false;
    }
    if (requestEnd) return;
    this->requestBangumiVideoList();
}

void MineBangumiRequest::requestBangumiVideoList() {
    std::string mid = ProgramConfig::instance().getUserID();
    if (mid.empty() || mid == "0") {
        this->onError("wiliwili/home/common/no_login"_i18n);
        return;
    }
    CHECK_AND_SET_REQUEST
    ASYNC_RETAIN
    BILI::get_my_bangumi(
        mid, requestType, bangumiCollection.pn, bangumiCollection.ps,
        [ASYNC_TOKEN](const bilibili::BangumiCollectionWrapper& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                if (result.pn != bangumiCollection.pn) {
                    brls::Logger::error(
                        "error request bangumi, type: {}, pn: {}, expected pn: "
                        "{}",
                        requestType, result.pn, bangumiCollection.pn);
                    return;
                }
                if (result.list.empty()) {
                    requestEnd = true;
                }
                bangumiCollection.pn++;
                bangumiCollection.total = result.total;
                this->onBangumiList(result);
                UNSET_REQUEST
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->onError(error);
                UNSET_REQUEST
            });
        });
}