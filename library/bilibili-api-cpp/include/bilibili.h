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

    class BilibiliClient {
        public:
            static ThreadPool pool;
            static ThreadPool imagePool;
            static void get_top10(int rid, std::function<void(VideoList)> callback);
            static void get_recommend(int rid, int num, std::function<void(VideoList)> callback);
            static void get_playurl(int cid, int quality, std::function<void(VideoPage)> callback);
            static void get_description(int aid, std::function<void(std::string)> callback);
            static void download(std::string url, std::function<void(unsigned char *, size_t)> callback);
            static void get(std::string url, std::function<void(std::string)> callback);
            static void init();
            static void clean();
    };
}