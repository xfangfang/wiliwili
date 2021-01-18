#include <bilibili.h>
#include <cpr/cpr.h>

std::string text_callback(std::string& expected_text, cpr::Response r) {
    expected_text = r.text;
    return r.text;
}

namespace bilibili {
    BilibiliClient::BilibiliClient(){

    }
    void BilibiliClient::test(std::function<void(std::string)> callback){
        cpr::GetCallback([callback](cpr::Response r) {
            callback(r.text);
        }, cpr::Url{"http://www.httpbin.org/get"});
    }
}