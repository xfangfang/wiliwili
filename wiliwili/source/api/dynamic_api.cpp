//
// Created by fang on 2022/8/18.
//

#include "bilibili.h"
#include "bilibili/result/dynamic_video.h"
#include "bilibili/util/http.hpp"

namespace bilibili {

void BilibiliClient::dynamic_video(const unsigned int page, const std::string& offset,
                                   const std::function<void(DynamicVideoListResultWrapper)>& callback,
                                   const ErrorCallback& error) {
    HTTP::getResultAsync<DynamicVideoListResultWrapper>(
        Api::DynamicVideo,
        {
            {"page", std::to_string(page)},
            {"offset", offset},
        },
        [callback, page](DynamicVideoListResultWrapper wrapper) {
            wrapper.page = page;
            callback(wrapper);
        },
        error);
}

void BilibiliClient::dynamic_up_list(const std::function<void(DynamicUpListResultWrapper)>& callback,
                                     const ErrorCallback& error) {
    HTTP::getResultAsync<DynamicUpListResultWrapper>(
        Api::DynamicUpList, {{"teenagers_mode", "0"}},
        [callback](const DynamicUpListResultWrapper& wrapper) { callback(wrapper); }, error);
}

}  // namespace bilibili