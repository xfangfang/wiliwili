//
// Created by fang on 2022/5/30.
//

#pragma once
#include <iostream>
#include <ctime>
#include <chrono>

#ifndef SIZE_T_MAX
/* some limits.h headers have this defined, some don't */
#ifdef __LP64__
#define SIZE_T_MAX 0xffffffffffffffffUL
#else
#define SIZE_T_MAX 0xffffffffUL
#endif
#endif

namespace wiliwili {

inline std::string pre0(size_t num, size_t length) {
    std::string str = std::to_string(num);
    if (length <= str.length()) {
        return str;
    }
    return std::string(length - str.length(), '0') + str;
}

static inline time_t unix_time() { return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); }

std::string getRandomText(int length = 16);

std::string getRandomNumber(int length = 8);

std::string getRandomHex(int length, bool lowerCase = true);

//100 => 01:40
//4180 => 01:09:40
std::string sec2Time(size_t t);

// 100 => 0:01:40
// 4180 => 1:09:40
std::string sec2TimeDLNA(size_t t);

//4180 => 69:40
std::string sec2MinSec(size_t t);

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

/**
 * 生成年月日: 2023-01-01
 * @param sec unix 时间戳
 * @return 年-月-日
 */
std::string sec2dateV2(time_t sec);

std::string sec2FullDate(time_t sec);

std::string sec2TimeDate(time_t sec);

size_t getUnixTime();

};  // namespace wiliwili