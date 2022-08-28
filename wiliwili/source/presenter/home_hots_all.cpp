//
// Created by fang on 2022/7/10.
//


#include "bilibili.h"
#include "bilibili/result/home_hots_all_result.h"
#include "presenter/home_hots_all.hpp"


void HomeHotsAllRequest::requestData(bool refresh) {
    static int current_page = 1;
    if(refresh){
        current_page = 1;
    }
    this->requestHotsAllVideoList(current_page, 40);
    current_page++;
}

void HomeHotsAllRequest::requestHotsAllVideoList(int index, int num) {
    bilibili::BilibiliClient::get_hots_all(index, num,
                                           [this, index](const bilibili::HotsAllVideoListResult &result, bool no_more){
                                               if(no_more){

                                               } else {

                                                   this->onHotsAllVideoList(result, index);
                                               }
                                           }, [this](const std::string &error) {
        this->onError(error);
            });
}
