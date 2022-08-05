//
// Created by fang on 2022/5/30.
//

#pragma once
#include <iostream>
#include <ctime>

namespace wiliwili {

    inline std::string pre0(size_t num, size_t length){
        std::string str = std::to_string(num);
        if(length <= str.length()){
            return str;
        }
        return std::string(length - str.length(), '0') + str;
    }

    //100 => 01:40
    std::string sec2Time(size_t t);


    // eg1: 1102 => 1102
    // eg2: 11022 => 1.1万
    // eg3: 10022 => 1万
    std::string num2w(size_t t);


    // < 1min => 刚刚
    // < 1hour => N分钟前
    // < 1day => N小时前
    // < 2day => 昨天
    // this year => M-D
    // else => YYYY-M-D
    std::string sec2date(time_t sec);


    size_t getUnixTime();

};