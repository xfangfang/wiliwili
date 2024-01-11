//
// Created by fang on 2022/8/19.
//

#include "bilibili.h"
#include "borealis/core/i18n.hpp"
#include "presenter/dynamic_tab.hpp"
#include "utils/config_helper.hpp"

using namespace brls::literals;

void DynamicTabRequest::onUpList(const bilibili::DynamicUpListResultWrapper &result) {}

void DynamicTabRequest::onError(const std::string &error) {}

void DynamicTabRequest::requestData() {
    CHECK_REQUEST
    this->requestUpList();
}

void DynamicTabRequest::requestUpList() {
    auto mid = ProgramConfig::instance().getUserID();
    if (mid.empty() || mid == "0") {
        this->onError("wiliwili/home/common/no_login"_i18n);
        return;
    }
    CHECK_AND_SET_REQUEST
    BILI::dynamic_up_list(
        [this](const bilibili::DynamicUpListResultWrapper &result) {
            this->onUpList(result);
            UNSET_REQUEST
        },
        [this](BILI_ERR) {
            if (code == -6) {
                this->onError("账号未登录");
            } else {
                this->onError(error);
            }
            UNSET_REQUEST
        });
}