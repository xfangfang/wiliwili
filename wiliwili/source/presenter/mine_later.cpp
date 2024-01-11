//
// Created by 贾海峰 on 2023/7/7.
//

#include "bilibili.h"
#include "presenter/mine_later.hpp"
#include "borealis/core/i18n.hpp"
#include "utils/config_helper.hpp"

using namespace brls::literals;

void MineLaterRequest::onWatchLaterList(const bilibili::WatchLaterListWrapper& result) {}

void MineLaterRequest::onError(const std::string& error) {}

void MineLaterRequest::requestData(bool refresh) { this->requestWatchLaterList(); }

void MineLaterRequest::requestWatchLaterList() {
    auto mid = ProgramConfig::instance().getUserID();
    if (mid.empty() || mid == "0") {
        this->onError("wiliwili/home/common/no_login"_i18n);
        return;
    }
    BILI::getWatchLater([this](const auto& result) { this->onWatchLaterList(result); },
                        [this](BILI_ERR) { this->onError(error); });
}