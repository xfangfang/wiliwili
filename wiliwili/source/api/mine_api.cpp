//
// Created by fang on 2022/7/26.
//
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"
#include "bilibili/util/md5.hpp"
#include "curl/curl.h"
#include "bilibili/util/http.hpp"
#include "bilibili/result/home_result.h"
#include "bilibili/result/setting.h"
#include "bilibili/result/mine_collection_result.h"
#include "bilibili/result/mine_bangumi_result.h"
#include "bilibili/result/mine_result.h"
#include "bilibili/result/mine_history_result.h"
#include "bilibili/result/mine_later_result.h"

namespace bilibili {

/// get qrcode for login
void BilibiliClient::get_login_url(const std::function<void(std::string, std::string)>& callback,
                                   const ErrorCallback& error) {
    HTTP::getResultAsync<QrLoginTokenResult>(
        Api::QrLoginUrl, {}, [callback](auto wrapper) { callback(wrapper.url, wrapper.oauthKey); }, error);
}

void BilibiliClient::get_login_url_v2(const std::function<void(std::string, std::string)>& callback,
                                      const ErrorCallback& error) {
    HTTP::getResultAsync<QrLoginTokenResultV2>(
        Api::QrLoginUrlV2, {{"source", "main_electron_pc"}},
        [callback](auto wrapper) { callback(wrapper.url, wrapper.qrcode_key); }, error);
}

/// check if qrcode has been scanned
void BilibiliClient::get_login_info(const std::string& oauthKey, const std::function<void(enum LoginInfo)>& callback,
                                    const ErrorCallback& error) {
    HTTP::__cpr_post(
        Api::QrLoginInfo, {}, {{"oauthKey", oauthKey}},
        [callback, error](const cpr::Response& r) {
            try {
                nlohmann::json res = nlohmann::json::parse(r.text);
                auto data          = res.get<QrLoginInfoResult>();

                if (data.status) {
                    std::map<std::string, std::string> cookies;
                    for (auto it = r.cookies.begin(); it != r.cookies.end(); it++) {
                        cookies[it->GetName()] = it->GetValue();
                        HTTP::COOKIES.emplace_back({it->GetName(), it->GetValue()});
                    }
                    if (BilibiliClient::writeCookiesCallback) {
                        BilibiliClient::writeCookiesCallback(cookies, "");
                    }
                }

                if (callback) callback(data.data);
                return;
            } catch (const std::exception& e) {
                ERROR_MSG("API error", -1);
                printf("data: %s\n", r.text.c_str());
                printf("ERROR: %s\n", e.what());
            }
        },
        error);
}

void BilibiliClient::get_login_info_v2(const std::string& qrcodeKey, const std::string& deviceName,
                                       const std::string& deviceID, const std::function<void(enum LoginInfo)>& callback,
                                       const ErrorCallback& error) {
    auto buvid3   = BilibiliClient::genRandomBuvid3();
    HTTP::COOKIES = {{{"appkey", BILIBILI_APP_KEY},
                      {"mobi_app", "pc_electron"},
                      {"device", "mac"},
                      {"innersign", "0"},
                      {"buvid3", buvid3},
                      {"device_id", deviceID},
                      {"device_name", deviceName}},
                     false};

    HTTP::__cpr_get(
        Api::QrLoginInfoV2, {{"qrcode_key", qrcodeKey}, {"source", "main_electron_pc"}},
        [callback, error, buvid3](const cpr::Response& r) {
            try {
                HTTP::COOKIES      = {false};
                nlohmann::json res = nlohmann::json::parse(r.text);
                auto data          = res.at("data").get<QrLoginInfoResultV2>();
                if (data.status) {
                    std::map<std::string, std::string> cookies;
                    cookies["buvid3"] = buvid3;
                    for (const auto& cookie : r.cookies) {
                        cookies[cookie.GetName()] = cookie.GetValue();
                        HTTP::COOKIES.emplace_back({cookie.GetName(), cookie.GetValue()});
                    }
                    if (BilibiliClient::writeCookiesCallback) {
                        BilibiliClient::writeCookiesCallback(cookies, data.refresh_token);
                    }
                }

                if (callback) callback(data.data);
                return;
            } catch (const std::exception& e) {
                ERROR_MSG("API error", -1);
                printf("data: %s\n", r.text.c_str());
                printf("ERROR: %s\n", e.what());
            }
        },
        error);
}

/// get person info (if login)
void BilibiliClient::get_my_info(const std::function<void(UserResult)>& callback, const ErrorCallback& error) {
    HTTP::getResultAsync<UserResult>(Api::MyInfo, {}, [callback](auto user) { callback(user); }, error);
}

/// 获取用户 关注/粉丝/黑名单数量
void BilibiliClient::get_user_relation(const std::string& mid, const std::function<void(UserRelationStat)>& callback,
                                       const ErrorCallback& error) {
    HTTP::getResultAsync<UserRelationStat>(Api::UserRelationStat, {{"vmid", mid}}, callback, error);
}

/// 获取用户动态的数量
void BilibiliClient::get_user_dynamic_count(const std::string& mid,
                                            const std::function<void(UserDynamicCount)>& callback,
                                            const ErrorCallback& error) {
    HTTP::getResultAsync<UserDynamicCount>(Api::UserDynamicStat, {{"uids", mid}}, callback, error);
}

/// get person history videos
void BilibiliClient::get_my_history(const HistoryVideoListCursor& cursor,
                                    const std::function<void(HistoryVideoResultWrapper)>& callback,
                                    const ErrorCallback& error) {
    HTTP::getResultAsync<HistoryVideoResultWrapper>(Api::HistoryVideo,
                                                    {{"max", std::to_string(cursor.max)},
                                                     {"view_at", std::to_string(cursor.view_at)},
                                                     {"business", cursor.business},
                                                     {"ps", std::to_string(cursor.ps)}},
                                                    callback, error);
}

// get watch later list
void BilibiliClient::getWatchLater(const std::function<void(WatchLaterListWrapper)>& callback,
                                   const bilibili::ErrorCallback& error) {
    HTTP::getResultAsync<WatchLaterListWrapper>(Api::WatchLater, {}, callback, error);
}

//
//static void getWatchLater(
//    const std::function<void(WatchLaterList)>& callback = nullptr,
//    const ErrorCallback& error = nullptr);

void BilibiliClient::get_my_collection_list(const int64_t mid, int index, int num, int type,
                                            const std::function<void(CollectionListResultWrapper)>& callback,
                                            const ErrorCallback& error) {
    BilibiliClient::get_my_collection_list(std::to_string(mid), index, num, type, callback, error);
}

void BilibiliClient::get_my_collection_list(const std::string& mid, int index, int num, int type,
                                            const std::function<void(CollectionListResultWrapper)>& callback,
                                            const ErrorCallback& error) {
    HTTP::getResultAsync<CollectionListResultWrapper>(
        type == 1 ? Api::CollectionList : Api::UserUGCSeason,
        {
            {"platform", type == 1 ? "pc" : "web"},
            {"up_mid", mid},
            {"ps", std::to_string(num)},
            {"pn", std::to_string(index)},
        },
        [callback, index](auto data) {
            data.index = index;
            callback(data);
        },
        error);
}

void BilibiliClient::get_collection_list_all(int rid, int type, std::string mid,
                                             const std::function<void(SimpleCollectionListResultWrapper)>& callback,
                                             const ErrorCallback& error) {
    HTTP::getResultAsync<SimpleCollectionListResultWrapper>(
        Api::CollectionListAll,
        {
            {"rid", std::to_string(rid)},
            {"type", std::to_string(type)},
            {"up_mid", mid},
        },
        [callback](SimpleCollectionListResultWrapper result) {
            for (size_t i = 0; i < result.list.size(); ++i) result.list[i].index = i;
            std::sort(result.list.begin(), result.list.end());
            callback(result);
        },
        error);
}

void BilibiliClient::get_collection_video_list(int64_t id, int index, int num, int type,
                                               const std::function<void(CollectionVideoListResultWrapper)>& callback,
                                               const ErrorCallback& error) {
    HTTP::getResultAsync<CollectionVideoListResultWrapper>(
        type == 1 ? Api::CollectionVideoList : Api::UserUGCSeasonVideoList,
        {
            {type == 1 ? "media_id" : "season_id", std::to_string(id)},
            {"ps", std::to_string(num)},
            {"pn", std::to_string(index)},
            {"platform", "web"},
        },
        [callback, index](auto data) {
            data.index = index;
            callback(data);
        },
        error);
}

void BilibiliClient::get_my_bangumi(const std::string& mid, size_t type, size_t pn, size_t ps,
                                    const std::function<void(BangumiCollectionWrapper)>& callback,
                                    const ErrorCallback& error) {
    HTTP::getResultAsync<BangumiCollectionWrapper>(Api::UserBangumiCollection,
                                                   {
                                                       {"vmid", mid},
                                                       {"ps", std::to_string(ps)},
                                                       {"pn", std::to_string(pn)},
                                                       {"type", std::to_string(type)},
                                                       {"platform", "web"},
                                                       {"follow_status", "0"},
                                                   },
                                                   callback, error);
}

/// get user's upload videos
void BilibiliClient::get_user_videos(const int64_t mid, int pn, int ps,
                                     const std::function<void(UserUploadedVideoResultWrapper)>& callback,
                                     const ErrorCallback& error) {
    HTTP::getResultAsync<UserUploadedVideoResultWrapper>(Api::UserUploadedVideo,
                                                         {
                                                             {"mid", std::to_string(mid)},
                                                             {"ps", std::to_string(ps)},
                                                             {"pn", std::to_string(pn)},
                                                         },
                                                         callback, error);
}

void BilibiliClient::get_user_videos2(const int64_t mid, int pn, int ps,
                                      const std::function<void(UserDynamicVideoResultWrapper)>& callback,
                                      const ErrorCallback& error) {
    HTTP::getResultAsync<UserDynamicVideoResultWrapper>(Api::UserDynamicVideo,
                                                        {
                                                            {"mid", std::to_string(mid)},
                                                            {"ps", std::to_string(ps)},
                                                            {"pn", std::to_string(pn)},
                                                        },
                                                        callback, error);
}

void BilibiliClient::get_unix_time(const std::function<void(UnixTimeResult)>& callback, const ErrorCallback& error) {
    HTTP::getResultAsync<UnixTimeResult>(Api::UnixTime, {}, callback, error);
}
}  // namespace bilibili