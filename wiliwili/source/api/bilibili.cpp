#include "bilibili.h"
#include <cpr/cpr.h>

namespace bilibili {

    // set bilibili cookie and cookies callback
    // This callback is called when the BilibiliClient updates the cookie
    void BilibiliClient::init(Cookies &data, std::function<void(Cookies)> callback, int timeout){
        BilibiliClient::writeCookiesCallback = callback;
        for(auto cookie: data){
            HTTP::COOKIES.emplace_back({cookie.first, cookie.second});
        }
        HTTP::TIMEOUT = timeout;
    }

}