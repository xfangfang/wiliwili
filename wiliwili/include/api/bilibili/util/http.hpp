//
// Created by fang on 2022/5/1.
//

#pragma once

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include "bilibili/util/md5.hpp"
#include "utils/number_helper.hpp"
#include <pystring.h>

namespace bilibili {

using Cookies = std::map<std::string, std::string>;

const std::string BILIBILI_APP_KEY    = "aa1e74ee4874176e";
const std::string BILIBILI_APP_SECRET = "54e6a9a31b911cd5fc0daa66ebf94bc4";
const std::string BILIBILI_BUILD      = "1001011000";

using ErrorCallback = std::function<void(const std::string&, int code)>;
#define ERROR_MSG(msg, ...) \
    if (error) error(msg, __VA_ARGS__)
#ifdef CALLBACK
#undef CALLBACK
#endif
#define CALLBACK(data) \
    if (callback) callback(data)
#define CPR_HTTP_BASE                                                                               \
    cpr::HttpVersion{cpr::HttpVersionCode::VERSION_2_0_TLS}, cpr::Timeout{bilibili::HTTP::TIMEOUT}, \
        bilibili::HTTP::HEADERS, bilibili::HTTP::COOKIES, bilibili::HTTP::PROXIES, bilibili::HTTP::VERIFY

class HTTP {
public:
    static inline cpr::Cookies COOKIES = {false};
    static inline cpr::Header HEADERS  = {
        {"User-Agent", "wiliwili"},
        {"Referer", "https://www.bilibili.com/client"},
        {"Origin", "https://www.bilibili.com"},
    };
    static inline int TIMEOUT = 10000;
    static inline cpr::Proxies PROXIES;
    static inline cpr::VerifySsl VERIFY;

    static cpr::Response get(const std::string& url, const cpr::Parameters& parameters = {}, int timeout = 10000);

    static void __cpr_post(const std::string& url, const cpr::Parameters& parameters = {},
                           const cpr::Payload& payload                               = {},
                           const std::function<void(const cpr::Response&)>& callback = nullptr,
                           const ErrorCallback& error                                = nullptr) {
        cpr::PostCallback(
            [callback, error](const cpr::Response& r) {
                if (r.error) {
                    ERROR_MSG(r.error.message, -1);
                    return;
                } else if (r.status_code != 200) {
                    ERROR_MSG("Network error. [Status code: " + std::to_string(r.status_code) + " ]", r.status_code);
                    return;
                }
                callback(r);
            },
            cpr::Url{url}, parameters, payload, CPR_HTTP_BASE);
    }

    static void __cpr_get(const std::string& url, const cpr::Parameters& parameters = {},
                          const std::function<void(const cpr::Response&)>& callback = nullptr,
                          const ErrorCallback& error                                = nullptr) {
        cpr::GetCallback(
            [callback, error](const cpr::Response& r) {
                if (r.error) {
                    ERROR_MSG(r.error.message, -1);
                    return;
                } else if (r.status_code != 200) {
                    ERROR_MSG("Network error. [Status code: " + std::to_string(r.status_code) + " ]", r.status_code);
                    return;
                }
                callback(r);
            },
            cpr::Url{url}, parameters, CPR_HTTP_BASE);
    }

    template <typename ReturnType>
    static int parseJson(const cpr::Response& r, const std::function<void(ReturnType)>& callback = nullptr,
                          const ErrorCallback& error = nullptr) {
        try {
            nlohmann::json res = nlohmann::json::parse(r.text);
            int code           = res.at("code").get<int>();
            if (code == 0) {
                if (res.contains("data") && (res.at("data").is_object() || res.at("data").is_array())) {
                    CALLBACK(res.at("data").get<ReturnType>());
                    return 0;
                } else if (res.contains("result") && res.at("result").is_object()) {
                    CALLBACK(res.at("result").get<ReturnType>());
                    return 0;
                } else {
                    printf("data: %s\n", r.text.c_str());
                    ERROR_MSG("Cannot find data", -1);
                }
            } else if (res.at("message").is_string()) {
                ERROR_MSG(res.at("message").get<std::string>(), code);
            } else {
                ERROR_MSG("Param error", -1);
            }
        } catch (const std::exception& e) {
            ERROR_MSG("Api error. \n" + std::string{e.what()}, 200);
            printf("data: %s\n", r.text.c_str());
            printf("ERROR: %s\n", e.what());
        }
        return 1;
    }

    static void signParameters(cpr::Parameters& parameters) {
        parameters.Add({{"appkey", BILIBILI_APP_KEY},
                        {"build", BILIBILI_BUILD},
                        {"ts", std::to_string(wiliwili::getUnixTime() * 1000)}});
        std::vector<std::string> kv;
        pystring::split(parameters.GetContent(cpr::CurlHolder()), kv, "&");
        std::sort(kv.begin(), kv.end());
        parameters.Add({{"sign", websocketpp::md5::md5_hash_hex(pystring::join("&", kv) + BILIBILI_APP_SECRET)}});
    }

    template <typename ReturnType>
    static void getResultAsync(const std::string& url, cpr::Parameters parameters = {},
                               const std::function<void(ReturnType)>& callback = nullptr,
                               const ErrorCallback& error = nullptr, bool needSign = false) {
        if (needSign) {
            signParameters(parameters);
        }
        __cpr_get(
            url, parameters, [callback, error](const cpr::Response& r) { parseJson<ReturnType>(r, callback, error); },
            error);
    }

    template <typename ReturnType>
    static void postResultAsync(const std::string& url, cpr::Parameters parameters = {}, cpr::Payload payload = {},
                                const std::function<void(ReturnType)>& callback = nullptr,
                                const ErrorCallback& error = nullptr, bool needSign = false) {
        if (needSign) {
            parameters.Add({{"appkey", BILIBILI_APP_KEY},
                            {"build", BILIBILI_BUILD},
                            {"ts", std::to_string(wiliwili::getUnixTime() * 1000)}});
            std::vector<std::string> kv;
            pystring::split(parameters.GetContent(cpr::CurlHolder()), kv, "&");
            std::sort(kv.begin(), kv.end());
            parameters.Add({{"sign", websocketpp::md5::md5_hash_hex(pystring::join("&", kv) + BILIBILI_APP_SECRET)}});
        }
        __cpr_post(
            url, parameters, payload,
            [callback, error](const cpr::Response& r) {
                try {
                    nlohmann::json res = nlohmann::json::parse(r.text);
                    int code           = res.at("code").get<int>();
                    if (code == 0) {
                        if (res.contains("data") && res.at("data").is_object()) {
                            CALLBACK(res.at("data").get<ReturnType>());
                        } else if (res.contains("result") && res.at("result").is_object()) {
                            CALLBACK(res.at("result").get<ReturnType>());
                        } else {
                            printf("data: %s\n", r.text.c_str());
                            ERROR_MSG("Cannot find data", -1);
                        }
                        return;
                    }
                    ERROR_MSG(res.at("message").get<std::string>(), code);
                } catch (const std::exception& e) {
                    ERROR_MSG(std::string(e.what()), r.status_code);
                    printf("data: %s\n", r.text.c_str());
                    printf("ERROR: %s\n", e.what());
                }
            },
            error);
    }

    static void postResultAsync(const std::string& url, cpr::Parameters parameters = {}, cpr::Payload payload = {},
                                const std::function<void()>& callback = nullptr, const ErrorCallback& error = nullptr) {
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
                    ERROR_MSG(res.at("message").get<std::string>(), code);
                } catch (const std::exception& e) {
                    ERROR_MSG(std::string(e.what()), r.status_code);
                    printf("data: %s\n", r.text.c_str());
                    printf("ERROR: %s\n", e.what());
                }
            },
            error);
    }
};

}  // namespace bilibili