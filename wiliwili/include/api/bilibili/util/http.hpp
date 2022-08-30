//
// Created by fang on 2022/5/1.
//

#pragma once

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

//todo: 临时写在这里
#include "utils/md5.hpp"
#include "utils/number_helper.hpp"
#include "pystring.h"

namespace bilibili {

    using Cookies = std::map<std::string, std::string>;

    const std::string BILIBILI_APP_KEY = "aa1e74ee4874176e";
    const std::string BILIBILI_APP_SECRET = "54e6a9a31b911cd5fc0daa66ebf94bc4";
    const std::string BILIBILI_BUILD = "1001005000";

    using ErrorCallback = std::function<void(const std::string&)>;
#define ERROR(msg, ...) if(error) error(msg)

    class HTTP {
    public:
        static cpr::Cookies COOKIES;
        static int TIMEOUT;

        static cpr::Response get(const std::string &url,
                                      const cpr::Parameters &parameters = {},
                                      int timeout = 10000);

        template<typename ReturnType>
        static void getResult(const std::string &url,
                              const std::initializer_list<cpr::Parameter> &parameters={},
                              const std::function<void(ReturnType)>& callback=nullptr,
                              const ErrorCallback& error=nullptr) {


            cpr::Parameters param(parameters);
            cpr::Response r = HTTP::get(url, param);
            if( r.status_code  != 200){
                ERROR("Network error. [Status code: " + std::to_string(r.status_code) + " ]", -404);
                return;
            }
            try{
                nlohmann::json res = nlohmann::json::parse(r.text);
                int code = res.at("code");
                if(code == 0){
                    if(callback) callback(res.at("data").get<ReturnType>());
                    return;
                } else {
                    ERROR("Param error", code);
                }
            }
            catch(const std::exception& e){
                if(error) ERROR("API error");
                printf("ERROR: %s\n",e.what());
            }
        }

        static void __cpr_post(
                const std::string& url,
                cpr::Parameters parameters={},
                cpr::Payload payload = {},
                const std::function<void(const cpr::Response&)>& callback=nullptr,
                const ErrorCallback& error=nullptr){
            cpr::PostCallback([callback, error](cpr::Response r) {
                if( r.status_code  != 200){
                    ERROR("Network error. [Status code: " + std::to_string(r.status_code) + " ]", -404);
                    return;
                }
                callback(r);
              },
              cpr::Url{url},
              cpr::Header{
                {"User-Agent"   , "NintendoSwitch"},
                {"Referer"      , "https://www.bilibili.com/"},
                {"Origin"      , "https://www.bilibili.com"},
              },
              parameters,
              payload,
              HTTP::COOKIES,
              cpr::Timeout{HTTP::TIMEOUT});
        }


        template<typename ReturnType>
        static void getResultAsync(const std::string& url, cpr::Parameters parameters={},
                              const std::function<void(ReturnType)>& callback=nullptr,
                              const ErrorCallback& error=nullptr,
                              bool needSign=false){
            if(needSign){
                parameters.Add({
                    {"appkey", BILIBILI_APP_KEY},
                    {"build", BILIBILI_BUILD},
                    {"ts", std::to_string(wiliwili::getUnixTime() * 1000)}
                });
                std::vector<std::string> kv;
                pystring::split(parameters.GetContent(cpr::CurlHolder()), kv, "&");
                std::sort(kv.begin(), kv.end());
                parameters.Add({
                    {"sign", websocketpp::md5::md5_hash_hex(pystring::join("&", kv) + BILIBILI_APP_SECRET)}
                });
            }
            cpr::GetCallback([callback, error](cpr::Response r) {
                                 try{
                                     nlohmann::json res = nlohmann::json::parse(r.text);
                                     int code = res.at("code");
                                     if(code == 0){
                                        if(res.contains("data")){
                                            if(callback) callback(res.at("data").get<ReturnType>());
                                        } else if(res.contains("result")){
                                            if(callback) callback(res.at("result").get<ReturnType>());
                                        } else{
                                            printf("data: %s\n", r.text.c_str());
                                            ERROR("Cannot find data");
                                        }
                                         return;
                                     } else {
                                         if(error){
                                             if(res.at("message").is_string()){
                                                 ERROR("error msg: " + res.at("message").get<std::string>() + \
                                                         "; error code: " + std::to_string(code));
                                             }else {
                                                 ERROR("Param error");
                                             }
                                         }
                                     }
                                 }
                                 catch(const std::exception& e){
                                     ERROR("Network error. [Status code: " + std::to_string(r.status_code) + " ]", -404);
                                     printf("data: %s\n", r.text.c_str());
                                     printf("ERROR: %s\n",e.what());
                                 }
                             },
                             cpr::Url{url},
                             cpr::Header{
                                     {"User-Agent"   , "NintendoSwitch"},
                                     {"Referer"      , "https://www.bilibili.com/"},
                                     {"Origin"      , "https://www.bilibili.com"},
                             },
                             parameters,
                             HTTP::COOKIES,
                             cpr::Timeout{HTTP::TIMEOUT});
        }

        template<typename ReturnType>
        static void postResultAsync(const std::string& url,
                                    cpr::Parameters parameters={},
                                    cpr::Payload payload = {},
                                   const std::function<void(ReturnType)>& callback=nullptr,
                                   const ErrorCallback& error=nullptr,
                                   bool needSign=false){
            if(needSign){
                parameters.Add({
                                       {"appkey", BILIBILI_APP_KEY},
                                       {"build", BILIBILI_BUILD},
                                       {"ts", std::to_string(wiliwili::getUnixTime() * 1000)}
                               });
                std::vector<std::string> kv;
                pystring::split(parameters.GetContent(cpr::CurlHolder()), kv, "&");
                std::sort(kv.begin(), kv.end());
                parameters.Add({
                                       {"sign", websocketpp::md5::md5_hash_hex(pystring::join("&", kv) + BILIBILI_APP_SECRET)}
                               });
            }
            __cpr_post(url, parameters, payload, [callback, error](const cpr::Response& r){
                try{
                    nlohmann::json res = nlohmann::json::parse(r.text);
                    int code = res.at("code");
                    if(code == 0){
                        if(callback) callback(res.at("data").get<ReturnType>());
                        return;
                    } else {
                        // todo: 这里貌似code不为0时且设置了error 并没有报错
                        ERROR("Param error");
                    }
                }
                catch(const std::exception& e){
                    ERROR("API error");
                    printf("data: %s\n", r.text.c_str());
                    printf("ERROR: %s\n",e.what());
                }
            }, error);
        }
    };

}