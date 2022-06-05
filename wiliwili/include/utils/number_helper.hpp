//
// Created by fang on 2022/5/30.
//

#pragma once
#include <iostream>
#include <ctime>

std::string pre0(size_t num, size_t length){
    std::string str = std::to_string(num);
    return std::string(length - str.length(), '0') + str;
}

std::string sec2Time(size_t t){
    size_t hour = t / 3600;
    size_t minute = t / 60 % 60;
    size_t sec = t % 60;
    if ( hour == 0){
        return pre0(minute, 2) + ":" + pre0(sec, 2);
    }
    return pre0(hour, 2) + ":" + pre0(minute, 2) + ":" + pre0(sec, 2);
}


// eg1: 1102 => 1102
// eg2: 11022 => 1.1万
// eg3: 10022 => 1万
std::string num2w(size_t t){
    if(t < 10000){
        return std::to_string(t);
    }
    t = t / 1000;
    if(t % 10 == 0){
        return std::to_string(t / 10) + "万";
    }
    return std::to_string(t / 10) + "." + std::to_string(t % 10) + "万";
}


// < 1min => 刚刚
// < 1hour => N分钟前
// < 1day => N小时前
// < 2day => 昨天
// this year => M-D
// else => YYYY-M-D
std::string sec2date(time_t sec){
    //todo: 从B站接口校准当前时间
    time_t curTime = time(NULL);
    struct tm curTm;
    localtime_r(&curTime, &curTm);
    struct tm tm;
    localtime_r(&sec, &tm);

    Logger::error("cur: {}, video: {}", curTime, sec);
    if(curTm.tm_year != tm.tm_year || sec > curTime){
        return std::to_string(tm.tm_year)+"-"+std::to_string(tm.tm_mon + 1)+"-"+std::to_string(tm.tm_mday);
    }
    size_t inter = curTime - sec;
    if(inter > 172800){ // larger then 48hour
        return std::to_string(tm.tm_mon + 1)+"-"+std::to_string(tm.tm_mday);
    }
    else if(inter > 86400){ // larger then 24hour
        return "昨天";
    }
    else if(inter > 3600){ // larger then 24hour
        return std::to_string(inter/3600)+"小时前";
    }
    else if(inter > 60){
        return std::to_string(inter/60)+"分钟前";
    }

    return "刚刚";
}