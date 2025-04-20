//
// Created for wiliwili based on bilibili-API-collect documentation
//

#pragma once

#include <array>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <map>
#include "md5.hpp"
#include <nlohmann/json.hpp>
#include "cpr/cpr.h"
#include "pystring.h"

namespace bilibili {

/**
 * WBI签名模块
 *
 * 自2023年3月起，B站部分接口开始采用WBI签名鉴权，作为一种Web端风控手段
 * 该模块根据 bilibili-API-collect 文档实现WBI签名算法
 *
 * 签名过程：
 * 1. 获取最新的 img_key 和 sub_key
 * 2. 通过混淆算法计算出 mixin_key
 * 3. 将请求参数按照字典序排序并拼接，再附加 mixin_key 后计算MD5值，得到w_rid
 * 4. 将w_rid和wts(时间戳)添加到原始请求参数中
 */
namespace wbi {

// 重排映射表，用于计算 mixin_key
const std::array<int, 64> MIXIN_KEY_ENC_TAB = {46, 47, 18, 2,  53, 8,  23, 32, 15, 50, 10, 31, 58, 3,  45, 35,
                                               27, 43, 5,  49, 33, 9,  42, 19, 29, 28, 14, 39, 12, 38, 41, 13,
                                               37, 48, 7,  16, 24, 55, 40, 61, 26, 17, 0,  1,  60, 51, 30, 4,
                                               22, 25, 54, 21, 56, 59, 6,  63, 57, 62, 11, 36, 20, 34, 44, 52};

// 存储的 WBI 实时口令
static std::string img_key;
static std::string sub_key;
static std::time_t last_update_time = 0;

// 缓存的 mixin_key
static std::string mixin_key;

/**
 * 从URL中提取key
 * 例如：从 https://i0.hdslb.com/bfs/wbi/7cd084941338484aae1ad9425b84077c.png
 * 提取出 7cd084941338484aae1ad9425b84077c
 */
inline std::string extractKeyFromUrl(const std::string& url) {
    size_t lastSlash = url.find_last_of('/');
    size_t lastDot   = url.find_last_of('.');

    if (lastSlash != std::string::npos && lastDot != std::string::npos && lastSlash < lastDot) {
        return url.substr(lastSlash + 1, lastDot - lastSlash - 1);
    }

    return "";
}

/**
 * 通过混淆算法计算 mixin_key
 * 将 img_key 和 sub_key 拼接后，根据映射表重新排列得到新的字符串
 * 并截取前32位作为 mixin_key
 */
inline std::string getMixinKey(const std::string& img_key, const std::string& sub_key) {
    std::string raw_key = img_key + sub_key;
    std::string key;
    key.reserve(32);

    for (size_t i = 0; i < 32; ++i) {
        key.push_back(raw_key[MIXIN_KEY_ENC_TAB[i]]);
    }

    return key;
}

/**
 * 从API获取最新的 img_key 和 sub_key
 * 每次调用此函数会检查缓存，如果距离上次更新时间不足1小时，则使用缓存值
 * 否则会从B站API获取最新的值
 */
inline bool updateWbiKeys() {
    std::time_t now = std::time(nullptr);

    // 如果距离上次更新时间少于1小时，则不更新
    if (now - last_update_time < 3600 && !img_key.empty() && !sub_key.empty()) {
        return true;
    }

    auto r = cpr::Get(cpr::Url{"https://api.bilibili.com/x/web-interface/nav"},
                      cpr::Header{{"User-Agent",
                                   "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) "
                                   "Chrome/91.0.4472.124 Safari/537.36"},
                                  {"Referer", "https://www.bilibili.com/"}});

    if (r.status_code != 200) {
        return false;
    }

    try {
        nlohmann::json res = nlohmann::json::parse(r.text);
        if (res.contains("data") && res["data"].contains("wbi_img")) {
            std::string img_url = res["data"]["wbi_img"]["img_url"];
            std::string sub_url = res["data"]["wbi_img"]["sub_url"];

            img_key = extractKeyFromUrl(img_url);
            sub_key = extractKeyFromUrl(sub_url);

            // 计算并缓存 mixin_key
            mixin_key = getMixinKey(img_key, sub_key);

            last_update_time = now;
            return true;
        }
    } catch (...) {
        return false;
    }

    return false;
}

/**
 * 对参数进行 WBI 签名
 * 流程：
 * 1. 获取最新的 wbi keys
 * 2. 添加 wts 时间戳参数
 * 3. 对参数排序并过滤特殊字符
 * 4. 计算签名并添加 w_rid 参数
 */
inline bool encWbi(cpr::Parameters& params) {
    // 确保已获取最新的 WBI keys
    if (!updateWbiKeys()) {
        return false;
    }

    // 添加 wts 时间戳
    std::time_t wts = std::time(nullptr);
    params.Add({"wts", std::to_string(wts)});

    // 获取所有参数并解析
    std::string paramContent = params.GetContent(cpr::CurlHolder());
    std::vector<std::string> paramPairs;
    pystring::split(paramContent, paramPairs, "&");

    // 解析参数并过滤特殊字符
    std::map<std::string, std::string> sorted_params;
    for (const auto& pair : paramPairs) {
        std::vector<std::string> kv;
        pystring::split(pair, kv, "=");
        if (kv.size() == 2) {
            std::string key   = kv[0];
            std::string value = kv[1];

            // 过滤 value 中的 "!'()*" 字符
            value.erase(
                std::remove_if(value.begin(), value.end(),
                               [](char c) { return c == '!' || c == '\'' || c == '(' || c == ')' || c == '*'; }),
                value.end());

            sorted_params[key] = value;
        }
    }

    // 构建待签名字符串
    std::string query;
    for (const auto& pair : sorted_params) {
        if (!query.empty()) {
            query += "&";
        }
        query += pair.first + "=" + pair.second;
    }

    // 计算 w_rid
    std::string w_rid = websocketpp::md5::md5_hash_hex(query + mixin_key);

    // 添加 w_rid 参数
    params.Add({"w_rid", w_rid});

    return true;
}

}  // namespace wbi
}  // namespace bilibili