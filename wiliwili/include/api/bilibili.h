#pragma once

#include <string>
#include <vector>
#include <future>
#include <nlohmann/json.hpp>
#include "ThreadPool.hpp"
#include "bilibili/api.h"
#include "bilibili/util/md5.hpp"

#include "bilibili_type.h"
#include "bilibili/util/http.hpp"
#include "bilibili/result/video_detail_result.h"
#include "bilibili/result/home_result.h"
#include "bilibili/result/home_hots_all_result.h"
#include "bilibili/result/home_hots_weekly_result.h"
#include "bilibili/result/home_hots_history_result.h"
#include "bilibili/result/home_hots_rank.h"

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
            static void get_recommend_old(int rid, int num, const std::function<void(VideoList)>& callback);
            static void get_playurl(int cid, int quality, const std::function<void(VideoPage)>& callback);

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

            /// get video detail by aid
            static void get_video_detail(const int aid,
                                         const std::function<void(VideoDetailResult)>& callback= nullptr,
                                         const ErrorCallback& error= nullptr);

            /// get video detail by bvid
            static void get_video_detail(const std::string& bvid,
                                         const std::function<void(VideoDetailResult)>& callback= nullptr,
                                         const ErrorCallback& error= nullptr);


            /// get video pagelist by aid
            static void get_video_pagelist(const int aid,
                                         const std::function<void(VideoDetailPageListResult)>& callback= nullptr,
                                         const ErrorCallback& error= nullptr);

            /// get video pagelist by bvid
            static void get_video_pagelist(const std::string& bvid,
                                           const std::function<void(VideoDetailPageListResult)>& callback= nullptr,
                                           const ErrorCallback& error= nullptr);


            /// get video url by aid & cid
            static void get_video_url(const int aid, const int cid, const int qn=64,
                                      const std::function<void(VideoUrlResult)>& callback= nullptr,
                                      const ErrorCallback& error= nullptr);


            /// get video url by bvid & cid
            static void get_video_url(const std::string& bvid, const int cid, const int qn=64,
                                      const std::function<void(VideoUrlResult)>& callback= nullptr,
                                      const ErrorCallback& error= nullptr);

            /// 主页 推荐
            static void get_recommend(const int index=1, const int num=24,
                                      const std::function<void(RecommendVideoListResult)>& callback= nullptr,
                                      const ErrorCallback& error= nullptr);

            /// 主页 热门 热门综合
            static void get_hots_all(const int index=1, const int num=40,
                                      const std::function<void(HotsAllVideoListResult, bool)>& callback= nullptr,
                                      const ErrorCallback& error= nullptr);

            /// 主页 热门 每周推荐列表
            static void get_hots_weekly_list(const std::function<void(HotsWeeklyListResult)>& callback= nullptr,
                                     const ErrorCallback& error= nullptr);

            /// 主页 热门 每周推荐
            static void get_hots_weekly(const int number,
                                     const std::function<void(HotsWeeklyVideoListResult, string, string)>& callback= nullptr,
                                     const ErrorCallback& error= nullptr);

            /// 主页 热门 入站必刷
            static void get_hots_history(
                                        const std::function<void(HotsHistoryVideoListResult, string)>& callback= nullptr,
                                        const ErrorCallback& error= nullptr);

            /// 主页 热门 排行榜 投稿视频
            static void get_hots_rank(const int rid, const string type="all",
                    const std::function<void(HotsRankVideoListResult , string)>& callback= nullptr,
                    const ErrorCallback& error= nullptr);

            /// 主页 热门 排行榜 官方
            static void get_hots_rank_pgc(const int season_type, const int day=3,
                    const std::function<void(HotsRankPGCVideoListResult , string)>& callback= nullptr,
                    const ErrorCallback& error= nullptr);

            static void download(std::string url, std::function<void(std::string, size_t)> callback);
            static void get(std::string url, std::function<void(std::string)> callback);
            static void init(Cookies &cookies, std::function<void(Cookies)> writeCookiesCallback);
            static void clean();
    };
}