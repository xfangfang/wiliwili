//
// Created by fang on 2022/5/1.
//

#pragma once

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include "bilibili/util/md5.hpp"
#include "bilibili/util/json.hpp"
#include "bilibili/util/wbi.hpp"
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
#define HTTP_CALLBACK(data) \
    if (callback) callback(data)

class CurlSharedObject {
public:
    CurlSharedObject() {
        share = curl_share_init();
        curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        // TODO: 下列两个选线不支持多线程，需要实现自定义线程池，每个线程共享一个 curl share 对象
        // curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
        // curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
        curl_share_setopt(share, CURLSHOPT_LOCKFUNC, lock_callback);
        curl_share_setopt(share, CURLSHOPT_UNLOCKFUNC, unlock_callback);
        curl_share_setopt(share, CURLSHOPT_USERDATA, lock_array);
    }
    ~CurlSharedObject() {
        curl_share_cleanup(share);
    }

    static void lock_callback(CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr) {
        auto *lock_array = (std::recursive_mutex *)userptr;
        lock_array[data].lock();
    }

    static void unlock_callback(CURL *handle, curl_lock_data data, void *userptr) {
        auto *lock_array = (std::recursive_mutex *)userptr;
        lock_array[data].unlock();
    }

    CURLSH* getShare() {
        return share;
    }

private:
    CURLSH* share;
    std::recursive_mutex lock_array[CURL_LOCK_DATA_LAST];
};

class HTTP {
public:
    static inline cpr::Cookies COOKIES = {false};
    static inline cpr::Header HEADERS  = {
        {"User-Agent", "wiliwili"},
        {"Referer", "https://www.bilibili.com/client"},
        {"Origin", "https://www.bilibili.com"},
    };
    static inline int TIMEOUT = 10000;
    static inline int CONNECTION_TIMEOUT = 0;
    static inline int DNS_CACHE_TIMEOUT = 60;
    static inline cpr::Proxies PROXIES;
    static inline cpr::VerifySsl VERIFY;
    static inline std::string PROTOCOL = "https:";
    static inline CurlSharedObject CURL_SHARE;

    static std::string getEncodedCookie(const cpr::Cookies& cookies);

    static std::shared_ptr<cpr::Session> createSession() {
        auto session = std::make_shared<cpr::Session>();
        CURL* curl = session->GetCurlHolder()->handle;
        curl_easy_setopt(curl, CURLOPT_SHARE, HTTP::CURL_SHARE.getShare());
        curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, HTTP::DNS_CACHE_TIMEOUT);
        session->SetTimeout(cpr::Timeout{bilibili::HTTP::TIMEOUT});
        session->SetConnectTimeout(cpr::ConnectTimeout{bilibili::HTTP::CONNECTION_TIMEOUT});
        session->SetHeader(bilibili::HTTP::HEADERS);
        session->SetProxies(bilibili::HTTP::PROXIES);
        session->SetVerifySsl(bilibili::HTTP::VERIFY);
        return session;
    }

    static void _cpr_post(const std::string& url, const cpr::Parameters& parameters = {},
                           const cpr::Payload& payload                               = {},
                           const std::function<void(const cpr::Response&)>& callback = nullptr,
                           const ErrorCallback& error                                = nullptr) {
        auto session = createSession();;
        session->SetUrl(cpr::Url{parseLink(url)});
        session->SetParameters(parameters);
        session->SetPayload(payload);

        session->PostCallback(
            [callback, error](const cpr::Response& r) {
                if (r.error) {
                    ERROR_MSG(r.error.message, -1);
                    return;
                } else if (r.status_code != 200) {
                    ERROR_MSG("Network error. [Status code: " + std::to_string(r.status_code) + " ]", r.status_code);
                    return;
                }
                callback(r);
            });
    }

    static void _cpr_get(const std::string& url, const cpr::Parameters& parameters = {},
                          const std::function<void(const cpr::Response&)>& callback = nullptr,
                          const ErrorCallback& error                                = nullptr) {
        auto session = createSession();;
        session->SetUrl(cpr::Url{parseLink(url)});
        session->SetParameters(parameters);

        session->GetCallback(
            [callback, error](const cpr::Response& r) {
                if (r.error) {
                    ERROR_MSG(r.error.message, -1);
                    return;
                }
                if (r.status_code != 200) {
                    ERROR_MSG("Network error. [Status code: " + std::to_string(r.status_code) + " ]", r.status_code);
                    return;
                }
                callback(r);
            });
    }

    template <typename ReturnType>
    static int parseJson(const cpr::Response& r, const std::function<void(ReturnType)>& callback = nullptr,
                          const ErrorCallback& error = nullptr) {
        try {
            nlohmann::json res = nlohmann::json::parse(r.text);
            int code           = res.at("code").get<int>();
            if (code == 0) {
                if (res.contains("data") && (res.at("data").is_object() || res.at("data").is_array())) {
                    HTTP_CALLBACK(res.at("data").get<ReturnType>());
                    return 0;
                } else if (res.contains("result") && res.at("result").is_object()) {
                    HTTP_CALLBACK(res.at("result").get<ReturnType>());
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

    static void signParameters(cpr::Parameters& parameters);

    template <typename ReturnType>
    static void getResultAsync(const std::string& url,
                               cpr::Parameters parameters                      = {},
                               const std::function<void(ReturnType)>& callback = nullptr,
                               const ErrorCallback& error                      = nullptr,
                               bool needSign                                   = false) {
        if (needSign) {
            signParameters(parameters);
        }
        _cpr_get(
            url,
            parameters,
            [callback, error](const cpr::Response& r) {
                parseJson<ReturnType>(r, callback, error);
            },
            error);
    }

    template <typename ReturnType>
    static void getResultWithWbiAsync(const std::string& url,
                                      cpr::Parameters parameters                      = {},
                                      const std::function<void(ReturnType)>& callback = nullptr,
                                      const ErrorCallback& error                      = nullptr,
                                      bool needSign                                   = false) {
        wbi::updateWbiKeys([url, parameters, callback, error, needSign]() mutable {
            if (needSign) {
                signParameters(parameters);
            }
            wbi::encWbi(parameters);
            _cpr_get(
                url,
                parameters,
                [callback, error](const cpr::Response& r) {
                    parseJson<ReturnType>(r, callback, error);
                },
                error);
        }, error);
    }

    template <typename ReturnType>
    static void postResultAsync(const std::string& url,
                                cpr::Parameters parameters                      = {},
                                const cpr::Payload& payload                     = {},
                                const std::function<void(ReturnType)>& callback = nullptr,
                                const ErrorCallback& error                      = nullptr,
                                bool needSign                                   = false) {
        if (needSign) {
            signParameters(parameters);
        }
        _cpr_post(
            url, parameters, payload,
            [callback, error](const cpr::Response& r) {
                try {
                    nlohmann::json res = nlohmann::json::parse(r.text);
                    const int code     = res.at("code").get<int>();
                    if (code == 0) {
                        if (res.contains("data") && res.at("data").is_object()) {
                            HTTP_CALLBACK(res.at("data").get<ReturnType>());
                        } else if (res.contains("result") && res.at("result").is_object()) {
                            HTTP_CALLBACK(res.at("result").get<ReturnType>());
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

    static void postResultAsync(const std::string& url,
                                const cpr::Parameters& parameters     = {},
                                const cpr::Payload& payload           = {},
                                const std::function<void()>& callback = nullptr,
                                const ErrorCallback& error            = nullptr) {
        _cpr_post(
            url, parameters, payload,
            [callback, error](const cpr::Response& r) {
                try {
                    nlohmann::json res = nlohmann::json::parse(r.text);
                    const int code     = res.at("code").get<int>();
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