#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"
#include "md5.hpp"

namespace bilibili {


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

    void BilibiliClient::get_top100(int rid, std::function<void(VideoList)> callback){
        this->request_common = cpr::GetCallback([callback](cpr::Response r) {
                    nlohmann::json res = nlohmann::json::parse(r.text);
                    VideoList videoList = res.get<VideoList>();
                    callback(videoList);
            },
            cpr::Url{"http://api.bilibili.com/x/web-interface/dynamic/region"},
            cpr::Parameters{
                {"ps","10"},
                {"rid","160"}
            }
        );
    }

    void BilibiliClient::get_playurl(int cid, std::function<void(VideoPage)> callback){
        std::string _APP_KEY = "iVGUTjsxvpLeuDCf";
        std::string _BILIBILI_KEY = "aHRmhWMLkdeMuILqORnYZocwMBpMEOdt";
        std::string payload = "appkey="+_APP_KEY+"&cid="+std::to_string(cid)+"&otype=json&qn=80&quality=80&type=";
        std::string sign = websocketpp::md5::md5_hash_hex(payload+_BILIBILI_KEY);
        std::string url = "http://interface.bilibili.com/v2/playurl?"+payload+"&sign="+sign;

        this->request_common = cpr::GetCallback([cid, callback](cpr::Response r) {
                nlohmann::json res = nlohmann::json::parse(r.text);
                VideoPage videoPage = res.get<VideoPage>();
                videoPage.cid = cid;
                callback(videoPage);
            },
            cpr::Url{url},
            cpr::Header{
                {"Referer","http://www.bilibili.com"},
                {"Agent","NintendoSwitch"}
            }
        );
    }

    void BilibiliClient::download(std::string url, std::function<void(unsigned char *, size_t)> callback){
        this->request_common = cpr::GetCallback([callback](cpr::Response r) {
                callback((unsigned char *)r.text.c_str(), (size_t)r.downloaded_bytes);
            },
            cpr::Url{url},
            cpr::Header{
                {"Referer","http://www.bilibili.com"},
                {"Agent","NintendoSwitch"}
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