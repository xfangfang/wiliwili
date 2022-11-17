//
// Created by fang on 2022/8/19.
//

#include "bilibili.h"
#include "presenter/dynamic_tab.hpp"

void DynamicTabRequest::onUpList(
    const bilibili::DynamicUpListResultWrapper &result) {}

void DynamicTabRequest::onError(const std::string &error) {}

void DynamicTabRequest::requestData() {
    CHECK_REQUEST
    this->requestUpList();
}

void DynamicTabRequest::requestUpList() {
    CHECK_AND_SET_REQUEST
    bilibili::BilibiliClient::dynamic_up_list(
        [this](const bilibili::DynamicUpListResultWrapper &result) {
            this->onUpList(result);
            UNSET_REQUEST
        },
        [this](const std::string &error) {
            this->onError(error);
            UNSET_REQUEST
        });
}