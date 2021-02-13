#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"
#include "md5.hpp"
#include "curl/curl.h"

namespace bilibili {

    ThreadPool BilibiliClient::pool(1);
    ThreadPool BilibiliClient::imagePool(4);

    size_t writeToString(void *ptr, size_t size, size_t count, void *stream){
        ((std::string*)stream)->append((char*)ptr, 0, size* count);
        return size* count;
    }

    std::string _curl_get(std::string url){
        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        std::string data;
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
            // data now holds response
        
            // write curl response to string variable..
            res = curl_easy_perform(curl);
            /* Check for errors */
            if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            /* always cleanup */
            curl_easy_cleanup(curl);
        }
        return data;
    }

    cpr::Response _cpr_get(std::string url){
        return cpr::Get(
            cpr::Url{url},
            cpr::Header{
                {"Agent"  , "NintendoSwitch"},
                {"Referer", "http://www.bilibili.com"}
            },
            cpr::Timeout{30000}
        );
    }

    void BilibiliClient::init(){
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    void BilibiliClient::clean(){
        curl_global_cleanup();
    }

    void BilibiliClient::get_top10(int rid, std::function<void(VideoList)> callback){
        std::string url = "http://api.bilibili.com/x/web-interface/ranking/region?day=7&rid="+std::to_string(rid);
        BilibiliClient::pool.enqueue([callback, url]{
            cpr::Response r = _cpr_get(url);
            nlohmann::json res = nlohmann::json::parse(r.text);
            VideoList videoList = res.get<VideoList>();
            callback(videoList);
        });
    }

    void BilibiliClient::get_recommend(int rid, int num, std::function<void(VideoList)> callback){
        std::string url = "http://api.bilibili.com/x/web-interface/dynamic/region?ps="+std::to_string(num)+"&rid="+std::to_string(rid);
        BilibiliClient::pool.enqueue([callback, url]{
            cpr::Response r = _cpr_get(url);
            nlohmann::json res = nlohmann::json::parse(r.text);
            VideoList videoList = res.get<VideoList>();
            callback(videoList);
        });
    }

    void BilibiliClient::get_playurl(int cid, int quality, std::function<void(VideoPage)> callback){
        std::string _APP_KEY = "iVGUTjsxvpLeuDCf";
        std::string _BILIBILI_KEY = "aHRmhWMLkdeMuILqORnYZocwMBpMEOdt";
        std::string q = std::to_string(quality);
        std::string payload = "appkey="+_APP_KEY+"&cid="+std::to_string(cid)+
            "&otype=json&qn="+q+"&quality="+q+"&type=";
        std::string sign = websocketpp::md5::md5_hash_hex(payload+_BILIBILI_KEY);
        std::string url = "http://interface.bilibili.com/v2/playurl?"+payload+"&sign="+sign;
        BilibiliClient::pool.enqueue([callback, url, cid]{
            cpr::Response r = _cpr_get(url);
            nlohmann::json res = nlohmann::json::parse(r.text);
            VideoPage videoPage = res.get<VideoPage>();
            videoPage.cid = cid;
            callback(videoPage);
        });
    }

    void BilibiliClient::get_description(int aid, std::function<void(std::string)> callback){
        std::string url = "http://api.bilibili.com/x/web-interface/archive/desc?aid="+std::to_string(aid);
        BilibiliClient::pool.enqueue([callback, url]{
            cpr::Response r = _cpr_get(url);
            nlohmann::json res = nlohmann::json::parse(r.text);
            callback(res.at("data"));
        });
    }

    void BilibiliClient::download(std::string url, std::function<void(unsigned char *, size_t)> callback){
        BilibiliClient::imagePool.enqueue([callback, url]{
            cpr::Response r = _cpr_get(url);
            if(r.error){
                callback(nullptr, 0);
            }else{
                callback((unsigned char *)r.text.c_str(), (size_t)r.downloaded_bytes);
            }
        });
    }


    void BilibiliClient::get(std::string url, std::function<void(std::string)> callback){
        BilibiliClient::pool.enqueue([callback, url]{
            cpr::Response r = _cpr_get(url);
            if (r.status_code >= 400) {
                callback(std::to_string(r.status_code));
            } else {
                callback(r.text);
            }
        });
    }
}