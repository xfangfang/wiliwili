//
// Created by fang on 2022/5/1.
//

#include "bilibili/util/http.hpp"

namespace bilibili {
    Cookies HTTP::cookies;

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
                cpr::Cookies(HTTP::cookies, false),
                cpr::Timeout{timeout}
        );
    }

};