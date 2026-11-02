#include "bilibili.h"
#include "bilibili/util/http.hpp"
#include <atomic>
#include <chrono>
#include <random>
#include <utility>

namespace bilibili {

std::string genRandomHex(int length) {
    char seed[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '\0'};

#ifdef PS5
    // std::random_device 在 PS5 上不可用（见 number_helper.cpp 中 getRandom 的
    // 说明），用时钟 + 计数器做种子。
    static std::atomic<unsigned> seedCounter{0};
    static unsigned seedValue = static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count()) ^ seedCounter.fetch_add(1);
    static std::mt19937 gen(seedValue);
#else
    static std::random_device rd;
    static std::mt19937 gen(rd());
#endif
    static std::uniform_int_distribution<> dis(0, 15);
    std::string text;
    for (int n = 0; n < length; ++n) {
        int val = dis(gen);
        text += seed[val];
    }
    return text;
}

std::string BilibiliClient::genRandomUuid() {
    return genRandomHex(8) + "-" + genRandomHex(4) + "-" + genRandomHex(4) + "-" + genRandomHex(4) + "-" +
           genRandomHex(17) + "infoc";
}

std::string BilibiliClient::genRandomBuvid3() {
    return genRandomHex(32);
}

// set bilibili cookie and cookies callback
// This callback is called when the BilibiliClient updates the cookie
void BilibiliClient::init(Cookies& data, std::function<void(Cookies, std::string)> callback) {
    BilibiliClient::writeCookiesCallback = std::move(callback);
    for (const auto& cookie : data) {
        HTTP::COOKIES.emplace_back({cookie.first, cookie.second});
    }
    // Manually set cookie, because cpr's cookie algorithm does not conform to the rfc6265
    HTTP::HEADERS["cookie"] = HTTP::getEncodedCookie(HTTP::COOKIES);
}

void BilibiliClient::setProxy(const std::string& httpProxy, const std::string& httpsProxy) {
    HTTP::PROXIES = {};
    if (!httpProxy.empty() && !httpsProxy.empty()) HTTP::PROXIES = {{"http", httpProxy}, {"https", httpsProxy}};
}

void BilibiliClient::setTlsVerify(bool value) {
    HTTP::VERIFY = cpr::VerifySsl{value};
}

void BilibiliClient::setHttpTimeout(int ms) {
    HTTP::TIMEOUT = ms;
}

void BilibiliClient::setConnectionTimeout(int ms) {
    HTTP::CONNECTION_TIMEOUT = ms;
}

void BilibiliClient::setDnsCacheTimeout(int ms) {
    HTTP::DNS_CACHE_TIMEOUT = ms / 1000;
}

}  // namespace bilibili