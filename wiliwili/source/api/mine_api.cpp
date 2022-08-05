//
// Created by fang on 2022/7/26.
//
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "bilibili.h"
#include "bilibili/util/md5.hpp"
#include "curl/curl.h"
#include "bilibili/util/http.hpp"

namespace bilibili {

    /// get qrcode for login
    void BilibiliClient::get_login_url(const std::function<void(std::string, std::string)>& callback,
                                       const ErrorCallback& error){
        HTTP::getResultAsync<QrLoginTokenResult>(Api::QrLoginUrl,
                                               {},
                                               [callback](auto wrapper){
                                                   callback(wrapper.url, wrapper.oauthKey);
                                               }, error);
    }

    /// check if qrcode has been scanned
    void BilibiliClient::get_login_info(const std::string oauthKey,
                                        const std::function<void(enum LoginInfo)> &callback,
                                        const ErrorCallback& error){
        HTTP::__cpr_post(Api::QrLoginInfo,{}, {{"oauthKey", oauthKey}}, [callback, error](const cpr::Response& r){
            try{
                nlohmann::json res = nlohmann::json::parse(r.text);
                auto data = res.get<QrLoginInfoResult>();

                if(data.status){
                    std::map<std::string, std::string> cookies;
                    for(auto it = r.cookies.begin(); it != r.cookies.end(); it++){
                        cookies[it->first] = it->second;
                    }
                    HTTP::cookies = cookies;
                    if(BilibiliClient::writeCookiesCallback){
                        BilibiliClient::writeCookiesCallback(cookies);
                    }
                }

                if(callback) callback(data.data);
                return;
            }
            catch(const std::exception& e){
                if(error) error("API error");
                printf("data: %s\n", r.text.c_str());
                printf("ERROR: %s\n",e.what());
            }
        }, error);
    }

    /// get person info (if login)
    void BilibiliClient::get_my_info(const std::function<void(UserResult)>& callback, const ErrorCallback& error){
        HTTP::getResultAsync<UserResult>(Api::MyInfo,
                                                 {},
                                                 [callback](auto user){
                                                     callback(user);
                                                 }, error);
    }

    /// get person history videos
    void BilibiliClient::get_my_history(const HistoryVideoListCursor& cursor,
                                        const std::function<void(HistoryVideoResultWrapper)>& callback,
                                        const ErrorCallback& error){
        HTTP::getResultAsync<HistoryVideoResultWrapper>(Api::HistoryVideo,
                                         {{"max", std::to_string(cursor.max)},
                                          {"view_at", std::to_string(cursor.view_at)},
                                          {"business", cursor.business},
                                          {"ps", std::to_string(cursor.ps)}},
                                         [callback](auto data){
                                             callback(data);
                                         }, error);
    }


    /// get person collection list
    void BilibiliClient::get_my_collection_list(const int mid, const int index, const int num,
                                                const std::function<void(CollectionListResultWrapper)>& callback,
                                                const ErrorCallback& error){
        HTTP::getResultAsync<CollectionListResultWrapper>(Api::CollectionList,
                                                        {{"platform", "pc"},
                                                         {"up_mid", std::to_string(mid)},
                                                         {"ps", std::to_string(num)},
                                                         {"pn", std::to_string(index)},
                                                         },
                                                        [callback, index](auto data){
                                                            data.index = index;
                                                            callback(data);
                                                        }, error);
    }


    /// get person collection list
    void BilibiliClient::get_my_collection_list(const std::string& mid, const int index, const int num,
                                                const std::function<void(CollectionListResultWrapper)>& callback,
                                                const ErrorCallback& error){
        HTTP::getResultAsync<CollectionListResultWrapper>(Api::CollectionList,
                                                          {{"platform", "pc"},
                                                           {"up_mid", mid},
                                                           {"ps", std::to_string(num)},
                                                           {"pn", std::to_string(index)},
                                                          },
                                                          [callback, index](auto data){
                                                              data.index = index;
                                                              callback(data);
                                                          }, error);
    }


    /// get collection video list
    void BilibiliClient::get_collection_video_list(int media_id, const int index, const int num,
                                          const std::function<void(CollectionVideoListResultWrapper)>& callback,
                                          const ErrorCallback& error){
        HTTP::getResultAsync<CollectionVideoListResultWrapper>(Api::CollectionVideoList,
                                                          {{"media_id", std::to_string(media_id)},
                                                           {"ps", std::to_string(num)},
                                                           {"pn", std::to_string(index)},
                                                          },
                                                          [callback, index](auto data){
                                                                data.index = index;
                                                              callback(data);
                                                          }, error);
    }

    /// get user's upload videos
    void BilibiliClient::get_user_videos(int mid, int pn, int ps,
                                const std::function<void(UserUploadedVideoResultWrapper)>& callback,
                                const ErrorCallback& error){
        HTTP::getResultAsync<UserUploadedVideoResultWrapper>(Api::UserUploadedVideo,
                                                               {{"mid", std::to_string(mid)},
                                                                {"ps", std::to_string(ps)},
                                                                {"pn", std::to_string(pn)},
                                                               },
                                                               [callback](auto data){
                                                                   callback(data);
                                                               }, error);
    }
}