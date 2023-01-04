//
// Created by fang on 2022/5/1.
//

#pragma once

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include "bilibili/util/md5.hpp"
#include "utils/number_helper.hpp"
#include "pystring.h"

namespace bilibili {

using Cookies = std::map<std::string, std::string>;

const std::string BILIBILI_APP_KEY    = "aa1e74ee4874176e";
const std::string BILIBILI_APP_SECRET = "54e6a9a31b911cd5fc0daa66ebf94bc4";
const std::string BILIBILI_BUILD      = "1001005000";

using ErrorCallback = std::function<void(const std::string&)>;
#define ERROR_MSG(msg, ...) \
    if (error) error(msg)
#define CALLBACK(data) \
    if (callback) callback(data)

class HTTP {
public:
    static cpr::Cookies COOKIES;
    static cpr::Header HEADERS;
    static int TIMEOUT;

    static cpr::Response get(const std::string& url,
                             const cpr::Parameters& parameters = {},
                             int timeout                       = 10000);

    static void __cpr_post(
        const std::string& url, cpr::Parameters parameters = {},
        cpr::Payload payload                                      = {},
        const std::function<void(const cpr::Response&)>& callback = nullptr,
        const ErrorCallback& error                                = nullptr) {
        cpr::PostCallback(
            [callback, error](cpr::Response r) {
                if (r.status_code != 200) {
                    ERROR_MSG("Network error. [Status code: " +
                                  std::to_string(r.status_code) + " ]",
                              -404);
                    return;
                }
                callback(r);
            },
            cpr::Url{url},
            cpr::Header{
                {"User-Agent", "NintendoSwitch"},
                {"Referer", "https://www.bilibili.com/"},
                {"Origin", "https://www.bilibili.com"},
            },
            parameters, payload, HTTP::COOKIES,
#ifndef VERIFY_SSL
            cpr::VerifySsl{false},
#endif
            cpr::Timeout{HTTP::TIMEOUT});
    }

    template <typename ReturnType>
    static void getResultAsync(
        const std::string& url, cpr::Parameters parameters = {},
        const std::function<void(ReturnType)>& callback = nullptr,
        const ErrorCallback& error = nullptr, bool needSign = false) {
        if (needSign) {
            parameters.Add(
                {{"appkey", BILIBILI_APP_KEY},
                 {"build", BILIBILI_BUILD},
                 {"ts", std::to_string(wiliwili::getUnixTime() * 1000)}});
            std::vector<std::string> kv;
            pystring::split(parameters.GetContent(cpr::CurlHolder()), kv, "&");
            std::sort(kv.begin(), kv.end());
            parameters.Add(
                {{"sign", websocketpp::md5::md5_hash_hex(
                              pystring::join("&", kv) + BILIBILI_APP_SECRET)}});
        }
        cpr::GetCallback(
            [callback, error](cpr::Response r) {
                try {
                    nlohmann::json res = nlohmann::json::parse(r.text);
                    int code           = res.at("code").get<int>();
                    if (code == 0) {
                        if (res.contains("data")) {
                            CALLBACK(res.at("data").get<ReturnType>());
                        } else if (res.contains("result")) {
                            CALLBACK(res.at("result").get<ReturnType>());
                        } else {
                            printf("data: %s\n", r.text.c_str());
                            ERROR_MSG("Cannot find data");
                        }
                        return;
                    }

                    if (res.at("message").is_string()) {
                        ERROR_MSG("error msg: " +
                                  res.at("message").get<std::string>() +
                                  "; error code: " + std::to_string(code));
                    } else {
                        ERROR_MSG("Param error");
                    }
                } catch (const std::exception& e) {
                    ERROR_MSG("Network error. [Status code: " +
                                  std::to_string(r.status_code) + " ]",
                              -404);
                    printf("data: %s\n", r.text.c_str());
                    printf("ERROR: %s\n", e.what());
                }
            },
            cpr::Url{url},
            cpr::Header{
                {"User-Agent", "NintendoSwitch"},
                {"Referer", "https://www.bilibili.com/"},
                {"Origin", "https://www.bilibili.com"},
            },
            parameters, HTTP::COOKIES,
#ifndef VERIFY_SSL
            cpr::VerifySsl{false},
#endif
            cpr::Timeout{HTTP::TIMEOUT});
    }

    template <typename ReturnType>
    static void postResultAsync(
        const std::string& url, cpr::Parameters parameters = {},
        cpr::Payload payload                            = {},
        const std::function<void(ReturnType)>& callback = nullptr,
        const ErrorCallback& error = nullptr, bool needSign = false) {
        if (needSign) {
            parameters.Add(
                {{"appkey", BILIBILI_APP_KEY},
                 {"build", BILIBILI_BUILD},
                 {"ts", std::to_string(wiliwili::getUnixTime() * 1000)}});
            std::vector<std::string> kv;
            pystring::split(parameters.GetContent(cpr::CurlHolder()), kv, "&");
            std::sort(kv.begin(), kv.end());
            parameters.Add(
                {{"sign", websocketpp::md5::md5_hash_hex(
                              pystring::join("&", kv) + BILIBILI_APP_SECRET)}});
        }
        __cpr_post(
            url, parameters, payload,
            [callback, error](const cpr::Response& r) {
                try {
                    nlohmann::json res = nlohmann::json::parse(r.text);
                    int code           = res.at("code").get<int>();
                    if (code == 0) {
                        if (res.contains("data")) {
                            CALLBACK(res.at("data").get<ReturnType>());
                        } else if (res.contains("result")) {
                            CALLBACK(res.at("result").get<ReturnType>());
                        } else {
                            ERROR_MSG("");
                        }
                        return;
                    }
                    ERROR_MSG("code: " + std::to_string(code) +
                              "; msg: " + res.at("message").get<std::string>());
                } catch (const std::exception& e) {
                    ERROR_MSG(
                        "API error: code: " + std::to_string(r.status_code) +
                        "; msg: " + std::string(e.what()));
                    printf("data: %s\n", r.text.c_str());
                    printf("ERROR: %s\n", e.what());
                }
            },
            error);
    }

    static void postResultAsync(const std::string& url,
                                cpr::Parameters parameters            = {},
                                cpr::Payload payload                  = {},
                                const std::function<void()>& callback = nullptr,
                                const ErrorCallback& error = nullptr) {
        __cpr_post(
            url, parameters, payload,
            [callback, error](const cpr::Response& r) {
                try {
                    nlohmann::json res = nlohmann::json::parse(r.text);
                    int code           = res.at("code").get<int>();
                    if (code == 0) {
                        if (callback) callback();
                        return;
                    }
                    ERROR_MSG("code: " + std::to_string(code) +
                              "; msg: " + res.at("message").get<std::string>());
                } catch (const std::exception& e) {
                    ERROR_MSG(
                        "API error: code: " + std::to_string(r.status_code) +
                        "; msg: " + std::string(e.what()));
                    printf("data: %s\n", r.text.c_str());
                    printf("ERROR: %s\n", e.what());
                }
            },
            error);
    }
};

}  // namespace bilibili