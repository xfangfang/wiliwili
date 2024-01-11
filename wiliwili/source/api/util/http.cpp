//
// Created by fang on 2022/5/1.
//

#include "bilibili/util/http.hpp"

namespace bilibili {

cpr::Response HTTP::get(const std::string& url, const cpr::Parameters& parameters, int timeout) {
    return cpr::Get(cpr::Url{url}, parameters, HTTP::HEADERS, HTTP::COOKIES, HTTP::PROXIES, HTTP::VERIFY,
                    cpr::Timeout{timeout});
}

};  // namespace bilibili