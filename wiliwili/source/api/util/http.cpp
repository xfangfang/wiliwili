//
// Created by fang on 2022/5/1.
//

#include "bilibili/util/http.hpp"

namespace bilibili {

std::string HTTP::getEncodedCookie(const cpr::Cookies& cookies) {
    std::stringstream stream;
    for (auto it = cookies.begin(); it != cookies.end(); ++it) {
        const cpr::Cookie& item = *it;
        stream << item.GetName() << "=" << item.GetValue();
        if (std::next(it) != cookies.end()) stream << "; ";
    }
    return stream.str();
}

void HTTP::signParameters(cpr::Parameters& parameters) {
    parameters.Add({{"appkey", BILIBILI_APP_KEY},
                    {"build", BILIBILI_BUILD},
                    {"ts", std::to_string(wiliwili::getUnixTime() * 1000)}});
    std::vector<std::string> kv;
    pystring::split(parameters.GetContent(cpr::CurlHolder()), kv, "&");
    std::sort(kv.begin(), kv.end());
    parameters.Add({{"sign", websocketpp::md5::md5_hash_hex(pystring::join("&", kv) + BILIBILI_APP_SECRET)}});
}

};  // namespace bilibili