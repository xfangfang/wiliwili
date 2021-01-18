#pragma once
#include <functional>
#include <string>


namespace bilibili {
    class BilibiliClient {
        public:
        BilibiliClient();
        void test(std::function<void(std::string)> callback);
    };
}
