#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"
#include "md5.hpp"
#include "curl/curl.h"

namespace bilibili {

    ThreadPool BilibiliClient::pool(1);
    ThreadPool BilibiliClient::imagePool(4);
    std::function<void(Cookies)> BilibiliClient::writeCookiesCallback = nullptr;
    Cookies BilibiliClient::cookies;

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

    cpr::Response _cpr_get(std::string url, const cpr::Parameters& parameters = {}){
        return cpr::Get(
            cpr::Url{url},
            cpr::Header{
                {"User-Agent"  , "NintendoSwitch"},
                {"Referer", "http://www.bilibili.com"},
            },
            parameters,
            cpr::Cookies(BilibiliClient::cookies, false),
            cpr::Timeout{30000}
        );
    }

    cpr::Response _cpr_post(std::string url, std::initializer_list<cpr::Pair> data){
        return cpr::Post(
            cpr::Url{url},
            cpr::Payload(data),
            cpr::Header{
                {"User-Agent"  , "NintendoSwitch"},
                {"Referer", "http://www.bilibili.com"}
            },
            cpr::Cookies(BilibiliClient::cookies, false),
            cpr::Timeout{30000}
        );
    }

    template<class ReturnType>
    void _bilibili_get(std::function<void(ReturnType)> callback, const std::string& url, const std::initializer_list<cpr::Parameter>& parameters = {}){
        cpr::Parameters param(parameters);
        BilibiliClient::pool.enqueue([callback, url, param]{
            cpr::Response r = _cpr_get(url, param);
            nlohmann::json res = nlohmann::json::parse(r.text);
            // printf("_bilibili : %s", r.text.c_str());
            int code = res.at("code");
            if(code == 0){
                try{
                    callback(res.at("data").get<ReturnType>());
                    return;
                }
                catch(const std::exception& e){
                    printf("ERROR: %s",e.what());
                }
            }
            ReturnType tmp;
            callback(tmp);
        }); 
    }

    void BilibiliClient::init(Cookies &cookies, std::function<void(Cookies)> writeCookiesCallback){
        curl_global_init(CURL_GLOBAL_DEFAULT);
        BilibiliClient::writeCookiesCallback = writeCookiesCallback;
        BilibiliClient::cookies = cookies;
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

    void BilibiliClient::get_login_url(std::function<void(std::string, std::string)> callback){
        std::string url = "http://passport.bilibili.com/qrcode/getLoginUrl";
        BilibiliClient::pool.enqueue([callback, url]{
            cpr::Response r = _cpr_get(url);
            nlohmann::json res = nlohmann::json::parse(r.text);
            nlohmann::json data = res.at("data");
            callback(data.at("url"), data.at("oauthKey"));
        });        
    }

    void BilibiliClient::get_login_info(std::string oauthKey, std::function<void(enum LoginInfo)> callback){
        std::string url = "http://passport.bilibili.com/qrcode/getLoginInfo";
        BilibiliClient::pool.enqueue([callback, url, oauthKey]{
            cpr::Response r = _cpr_post(url,{
                {std::string("oauthKey"), std::string(oauthKey)}
            });
            nlohmann::json res = nlohmann::json::parse(r.text);
            bool success = res.at("status");
            if(success){
                std::map<std::string, std::string> cookies;
                for(std::map<std::string, std::string>::iterator it = r.cookies.begin(); it != r.cookies.end(); it++){
                    cookies[it->first] = it->second;
                }
                BilibiliClient::cookies = cookies;
                if(BilibiliClient::writeCookiesCallback){
                    BilibiliClient::writeCookiesCallback(cookies);
                }
                callback(LoginInfo::SUCCESS);
            } else {
                int data = res.at("data");
                callback(LoginInfo(data));
            }
        }); 
    }

    void BilibiliClient::get_my_info(std::function<void(UserDetail)> callback){
        _bilibili_get(callback, "http://api.bilibili.com/x/space/myinfo");
    }

    void BilibiliClient::get_user_videos(int mid, int pn, int ps, std::function<void(space_user_videos::VideoList)> callback){
        _bilibili_get<space_user_videos::VideoList>([callback](space_user_videos::VideoList list){
            list.has_more = list.page.pn * list.page.ps < list.page.count;
            callback(list);
        }, "http://api.bilibili.com/x/space/arc/search",
            {
                {"mid", std::to_string(mid)},
                {"pn" , std::to_string(pn)},
                {"ps" , std::to_string(ps)}
            }
        );
    }

    void BilibiliClient::get_user_collections(int mid, int pn, int ps, std::function<void(space_user_collections::CollectionList)> callback){
        _bilibili_get(callback, "https://api.bilibili.com/x/v3/fav/folder/created/list",
            {
                {"up_mid", std::to_string(mid)},
                {"pn" , std::to_string(pn)},
                {"ps" , std::to_string(ps)}
            }
        );
    }

    // ps must less then 20
    void BilibiliClient::get_collection_videos(int id, int pn, int ps, std::function<void(space_user_collections::CollectionDetail)> callback){
        _bilibili_get(callback, "https://api.bilibili.com/x/v3/fav/resource/list",
            {
                {"media_id", std::to_string(id)},
                {"pn" , std::to_string(pn)},
                {"ps" , std::to_string(ps)}
            }
        );
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