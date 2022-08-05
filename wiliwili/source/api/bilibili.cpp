#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"
#include "bilibili/util/md5.hpp"
#include "curl/curl.h"
#include "bilibili/util/http.hpp"

namespace bilibili {

    // set bilibili cookie and cookies callback
    // This callback is called when the BilibiliClient updates the cookie
    void BilibiliClient::init(Cookies &data, std::function<void(Cookies)> callback){
        BilibiliClient::writeCookiesCallback = callback;
        HTTP::cookies = data;
    }

}