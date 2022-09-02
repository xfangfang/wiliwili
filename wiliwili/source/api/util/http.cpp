//
// Created by fang on 2022/5/1.
//

#include "bilibili/util/http.hpp"

namespace bilibili {
    cpr::Cookies HTTP::COOKIES = cpr::Cookies(false);
    int HTTP::TIMEOUT = 10000;
    cpr::Header HTTP::HEADERS = {
                                    {"User-Agent"   , "NintendoSwitch"},
                                    {"Referer"      , "https://www.bilibili.com/"},
                                    {"Origin"      , "https://www.bilibili.com"},
                                };

     cpr::Response HTTP::get(const std::string& url,
                                  const cpr::Parameters& parameters,
                                  int timeout){
        return cpr::Get(
                cpr::Url{url},
                cpr::Header{
                        {"User-Agent"   , "NintendoSwitch"},
                        {"Referer"      , "https://www.bilibili.com"},
                },
                parameters,
                HTTP::COOKIES,
                cpr::Timeout{timeout}
        );
    }

};