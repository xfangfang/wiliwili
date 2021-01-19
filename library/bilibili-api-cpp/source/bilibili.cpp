#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"

namespace bilibili {

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Video, bvid, title, pic);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoList, code, message, data);

    BilibiliClient::BilibiliClient(){

    }
    void BilibiliClient::test(std::function<void(std::string)> callback){
        this->_common_get("http://www.httpbin.org/get", callback);
    }

    void BilibiliClient::get_top10(int rid, std::function<void(VideoList)> callback){
        this->request_common = cpr::GetCallback([callback](cpr::Response r) {
                nlohmann::json res = nlohmann::json::parse(r.text);
                VideoList videoList = res.get<VideoList>();
                callback(videoList);
            },
            cpr::Url{"http://api.bilibili.com/x/web-interface/ranking/region"},
            cpr::Parameters{
                {"rid","160"},
                {"day","7"}
            }
        );
    }

    void BilibiliClient::_common_get(std::string url, std::function<void(std::string)> callback){
        this->request_common = cpr::GetCallback([callback](cpr::Response r) {
                callback(r.text);
            },
            cpr::Url{url},
            cpr::Header{
                {"Agent","NintendoSwitch"}
            },
            cpr::Parameters{
                {"test","1"}
            }
        );
    }
}