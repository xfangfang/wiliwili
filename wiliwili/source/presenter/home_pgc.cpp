//
// Created by fang on 2022/7/14.
//

#include "bilibili.h"
#include "presenter/home_pgc.hpp"


void HomeBangumiRequest::onBangumiList(const bilibili::PGCModuleListResult &result, int has_next) {}

void HomeBangumiRequest::onError() {}

void HomeBangumiRequest::requestData() {
    this->requestBangumiList(0, 0);
}

void HomeBangumiRequest::requestBangumiList(int is_refresh, int cursor) {
    bilibili::BilibiliClient::get_bangumi(is_refresh, cursor,
            [this](const bilibili::PGCModuleListResult &result, int has_next, std::string next_cursor) {
                this->onBangumiList(result, has_next);
            }, [](const std::string &error) {

            });
}


void HomeCinemaRequest::onCinemaList(const bilibili::PGCModuleListResult &result, int has_next) {}

void HomeCinemaRequest::onError() {}

void HomeCinemaRequest::requestData() {
    this->requestCinemaList(0, 0);
}

void HomeCinemaRequest::requestCinemaList(int is_refresh, int cursor) {
    bilibili::BilibiliClient::get_cinema(is_refresh, cursor,
                                          [this](const bilibili::PGCModuleListResult &result, int has_next, std::string next_cursor) {
                                              this->onCinemaList(result, has_next);
                                          }, [](const std::string &error) {

            });
}