#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"
#include "bilibili/util/md5.hpp"
#include "curl/curl.h"
#include "bilibili/util/http.hpp"

namespace bilibili {

    void BilibiliClient::get_recommend(const int index, const int num,
                                       const std::function<void(RecommendVideoListResult)>& callback,
                                       const ErrorCallback& error){
//        BilibiliClient::pool.enqueue([=]{
            HTTP::getResultAsync<RecommendVideoListResultWrapper>(Api::Recommend,
                                               {{"fresh_idx", std::to_string(index)},
                                                {"ps", std::to_string(num)},
                                                {"feed_version", "V1"},
                                                {"fresh_type", "4"},
                                                {"plat", "1"},
                                                },
                                               [callback](const RecommendVideoListResultWrapper& wrapper){
                                                   callback(wrapper.item);
                }, error);
//        });
    }

}