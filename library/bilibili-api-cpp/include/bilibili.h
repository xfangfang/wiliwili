#pragma once

#include <string>
#include <vector>
#include <future>
#include <nlohmann/json.hpp>
#include "ThreadPool.hpp"

#include "bilibili_type.h"

namespace bilibili {
    
    // using Request = std::future<void>;
    // using json = nlohmann::json;
    using Cookies = std::map<std::string, std::string>;

    class BilibiliClient {
        static std::function<void(Cookies)> writeCookiesCallback;
        public:
            static Cookies cookies;
            static ThreadPool pool;
            static ThreadPool imagePool;
            static void get_top10(int rid, std::function<void(VideoList)> callback);
            static void get_recommend(int rid, int num, std::function<void(VideoList)> callback);
            static void get_playurl(int cid, int quality, std::function<void(VideoPage)> callback);

            // get qrcode for login
            static void get_login_url(std::function<void(std::string, std::string)> callback);

            // check if qrcode has been scaned
            static void get_login_info(std::string oauthKey, std::function<void(enum LoginInfo)> callback);

            // get person info (if login)
            static void get_my_info(std::function<void(UserDetail)> callback);

            // get user's upload videos
            static void get_user_videos(int mid, int pn, int ps, std::function<void(space_user_videos::VideoList)> callback);

            //get user's collections
            static void get_user_collections(int mid, int pn, int ps, std::function<void(space_user_collections::CollectionList)> callback);

            //get videos by collection id
            static void get_collection_videos(int id, int pn, int ps, std::function<void(space_user_collections::CollectionDetail)> callback);

            // get 
            static void get_description(int aid, std::function<void(std::string)> callback);

            static void download(std::string url, std::function<void(unsigned char *, size_t)> callback);
            static void get(std::string url, std::function<void(std::string)> callback);
            static void init(Cookies &cookies, std::function<void(Cookies)> writeCookiesCallback);
            static void clean();
    };
}