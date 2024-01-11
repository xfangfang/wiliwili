#include "bilibili.h"
#include "bilibili/util/http.hpp"
#include <random>
#include <utility>

namespace bilibili {

std::string genRandomHex(int length) {
    char seed[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '\0'};

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    std::string text;
    for (int n = 0; n < length; ++n) {
        int val = dis(gen);
        text += seed[val];
    }
    return text;
}

std::string BilibiliClient::genRandomBuvid3() {
    return genRandomHex(8) + "-" + genRandomHex(4) + "-" + genRandomHex(4) + "-" + genRandomHex(4) + "-" +
           genRandomHex(17) + "infoc";
}

// set bilibili cookie and cookies callback
// This callback is called when the BilibiliClient updates the cookie
void BilibiliClient::init(Cookies& data, std::function<void(Cookies, std::string)> callback, int timeout,
                          const std::string& httpProxy, const std::string& httpsProxy, bool tlsVerify) {
    BilibiliClient::writeCookiesCallback = std::move(callback);
    for (const auto& cookie : data) {
        HTTP::COOKIES.emplace_back({cookie.first, cookie.second});
    }
    HTTP::TIMEOUT = timeout;

    if (!httpProxy.empty() && !httpsProxy.empty()) HTTP::PROXIES = {{"http", httpProxy}, {"https", httpsProxy}};

    HTTP::VERIFY = cpr::VerifySsl{tlsVerify};
}

void BilibiliClient::setProxy(const std::string& httpProxy, const std::string& httpsProxy) {
    HTTP::PROXIES = {};
    if (!httpProxy.empty() && !httpsProxy.empty()) HTTP::PROXIES = {{"http", httpProxy}, {"https", httpsProxy}};
}

void BilibiliClient::setTlsVerify(bool value) { HTTP::VERIFY = cpr::VerifySsl{value}; }

}  // namespace bilibili