//
// Created by fang on 2022/8/24.
//

#include "bilibili.h"
#include "presenter/pgc_index.hpp"
#include "pystring.h"
#include "borealis/core/logger.hpp"

void PGCIndexRequest::onPGCIndex(const bilibili::PGCIndexResultWrapper &result){}

void PGCIndexRequest::onPGCFilter(const bilibili::PGCIndexFilters &result){}

void PGCIndexRequest::onError(const std::string& error){}

void PGCIndexRequest::requestData(UserRequestData data, bool refresh){

    std::string req = "";
    size_t i = 0;
    for(auto& d: data){
        if(i!=0){
            req += "&";
        }
        auto value = d.second;
        value = pystring::replace(value, "-01-01 00:00:00", "");
        value = pystring::replace(value, "[", "%5B");
        value = pystring::replace(value, ",", "%2C");
        value = pystring::replace(value, ")", "%29");
        req += d.first + "=" + value;
        i++;
    }
    printf("requestPGCIndex: %s\n", req.c_str());

    if(refresh){
        this->requestIndex = 1;
    }
    this->requestPGCIndex(req, this->requestIndex);
}

void PGCIndexRequest::requestPGCIndex(const string& param, int page){
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_pgc_index(param, page,
            [ASYNC_TOKEN](const bilibili::PGCIndexResultWrapper& result) {
                ASYNC_RELEASE
                if(result.num != this->requestIndex){
                    brls::Logger::error("获取视频列表 错误的index: {} 请求的index: {}", result.num, this->requestIndex);
                    return;
                }
                this->requestIndex++;
                this->onPGCIndex(result);
            }, [ASYNC_TOKEN](const std::string &error) {
                ASYNC_RELEASE
                this->onError(error);
            });
}

void PGCIndexRequest::requestPGCFilter() {
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_pgc_all_filter([ASYNC_TOKEN](const bilibili::PGCIndexFilters& result){
        ASYNC_RELEASE
        INDEX_FILTERS = result;
       this->onPGCFilter(result);
    }, [ASYNC_TOKEN](const std::string &error) {
        ASYNC_RELEASE
        this->onError(error);
    });
}