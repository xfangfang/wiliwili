//
// Created by fang on 2022/5/1.
//

#pragma once

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>


namespace bilibili {

    using Cookies = std::map<std::string, std::string>;
    using ErrorCallback = std::function<void(const std::string&)>;

    class HTTP {
    public:
        static Cookies cookies;

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
                if(error) error("Network error");
                return;
            }
            try{
                nlohmann::json res = nlohmann::json::parse(r.text);
                int code = res.at("code");
                if(code == 0){
                    if(callback) callback(res.at("data").get<ReturnType>());
                    return;
                } else {
                    if(error) error("Param error");
                }
            }
            catch(const std::exception& e){
                if(error) error("API error");
                printf("ERROR: %s\n",e.what());
            }
        }


        template<typename ReturnType>
        static void getResultAsync(const std::string& url,
                              const cpr::Parameters& parameters={},
                              const std::function<void(ReturnType)>& callback=nullptr,
                              const ErrorCallback& error=nullptr){
            cpr::GetCallback([callback, error](cpr::Response r) {
                                 if( r.status_code  != 200){
                                     if(error) error("Network error");
                                     return;
                                 }
                                 try{
                                     nlohmann::json res = nlohmann::json::parse(r.text);
                                     int code = res.at("code");
                                     if(code == 0){
//                                         std::this_thread::sleep_for(std::chrono::seconds(3));
                                         if(callback) callback(res.at("data").get<ReturnType>());
                                         return;
                                     } else {
                                         if(error) error("Param error");
                                     }
                                 }
                                 catch(const std::exception& e){
                                     if(error) error("API error");
                                     printf("ERROR: %s\n",e.what());
                                 }
                             },
                             cpr::Url{url},
                             cpr::Header{
                                     {"User-Agent"   , "NintendoSwitch"},
                                     {"Referer"      , "https://www.bilibili.com"},
                             },
                             parameters,
                             cpr::Cookies(HTTP::cookies, false),
                             cpr::Timeout{10000});
        }
    };

}