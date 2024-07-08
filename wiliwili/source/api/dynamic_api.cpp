//
// Created by fang on 2022/8/18.
//

#include "bilibili.h"
#include "bilibili/api.h"
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

void BilibiliClient::dynamic_article(const unsigned int page, const std::string& offset, uint64_t mid,
                                     const std::function<void(DynamicArticleListResultWrapper)>& callback,
                                     const ErrorCallback& error) {
    HTTP::getResultAsync<DynamicArticleListResultWrapper>(
        Api::DynamicArticle,
        {
            {"page", std::to_string(page)},
            {"offset", offset},
            {"host_mid", std::to_string(mid)},
        },
        [callback, page](DynamicArticleListResultWrapper wrapper) {
            wrapper.page = page;
            callback(wrapper);
        },
        error);
}

void BilibiliClient::dynamic_up_list(const std::function<void(DynamicUpListResultWrapper)>& callback,
                                     const ErrorCallback& error) {
    HTTP::getResultAsync<DynamicUpListResultWrapper>(
        Api::DynamicUpListV2, {}, [callback](const DynamicUpListResultWrapper& wrapper) { callback(wrapper); }, error);
}

void BilibiliClient::be_agree_dynamic(const std::string& access_key, const std::string& id, bool is_like,
                                      const std::function<void()>& callback, const ErrorCallback& error) {
    cpr::Payload payload = {
        {"dynamic_id", id},
        {"up", is_like ? "1" : "2"},
        {"csrf", access_key},
    };
    HTTP::postResultAsync(Api::DynamicLike, {}, payload, callback, error);
}

void BilibiliClient::get_dynamic_detail(const std::string& id,
                                        const std::function<void(DynamicArticleResultWrapper)>& callback,
                                        const ErrorCallback& error) {
    HTTP::getResultAsync<DynamicArticleResultWrapper>(
        Api::DynamicDetail, {{"id", id}}, [callback](const DynamicArticleResultWrapper& wrapper) { callback(wrapper); },
        error);
}

}  // namespace bilibili