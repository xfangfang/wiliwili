#pragma once

#include <string>
#include <vector>
#include <future>
#include <nlohmann/json.hpp>

#include "bilibili_type.h"

namespace bilibili {
    
    using Request = std::future<void>;
    using json = nlohmann::json;

    class BilibiliClient {
        public:
            BilibiliClient();
            void test(std::function<void(std::string)> callback);
            void get_top10(int rid, std::function<void(VideoList)> callback);

        private:
            Request request_common;
            void _common_get(std::string url, std::function<void(std::string)> callback);
    };
}
