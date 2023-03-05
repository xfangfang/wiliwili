#include "bilibili.h"
#include <cpr/cpr.h>

#include <utility>

namespace bilibili {

// set bilibili cookie and cookies callback
// This callback is called when the BilibiliClient updates the cookie
void BilibiliClient::init(Cookies& data, std::function<void(Cookies)> callback,
                          int timeout, const std::string& httpProxy,
                          const std::string& httpsProxy) {
    BilibiliClient::writeCookiesCallback = std::move(callback);
    for (const auto& cookie : data) {
        HTTP::COOKIES.emplace_back({cookie.first, cookie.second});
    }
    HTTP::TIMEOUT = timeout;

    if (!httpProxy.empty() && !httpsProxy.empty())
        HTTP::PROXIES = {{"http", httpProxy}, {"https", httpsProxy}};
}

}  // namespace bilibili