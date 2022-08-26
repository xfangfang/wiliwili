#pragma once

#include <string>
#include <vector>
#include <future>
#include <nlohmann/json.hpp>
#include "ThreadPool.hpp"
#include "bilibili/api.h"
#include "bilibili/util/md5.hpp"

#include "bilibili/util/http.hpp"
#include "bilibili/result/video_detail_result.h"
#include "bilibili/result/home_result.h"
#include "bilibili/result/home_hots_all_result.h"
#include "bilibili/result/home_hots_weekly_result.h"
#include "bilibili/result/home_hots_history_result.h"
#include "bilibili/result/home_hots_rank.h"
#include "bilibili/result/mine_result.h"
#include "bilibili/result/mine_history_result.h"
#include "bilibili/result/mine_collection_result.h"
#include "bilibili/result/home_pgc_season_result.h"

namespace bilibili {

    class LiveResultWrapper;
    class LiveUrlResultWrapper;
    class SearchResult;
    class DynamicVideoListResultWrapper; // 动态 全部关注的视频列表
    class DynamicUpListResultWrapper;    // 动态 最近更新的UP主列表
    class UserDynamicVideoResultWrapper; // 动态 单个up主视频列表
    class PGCIndexResultWrapper;
    class PGCIndexFilterWrapper;
    class PGCResultWrapper;
    typedef std::map<std::string, PGCIndexFilterWrapper> PGCIndexFilters;
    class SearchHotsResultWrapper; // 搜索页 获取热搜榜

    using Cookies = std::map<std::string, std::string>;

    class BilibiliClient {
        inline static std::function<void(Cookies)> writeCookiesCallback = nullptr;
        public:
            static Cookies cookies;

            /// get qrcode for login
            static void get_login_url(const std::function<void(std::string, std::string)>& callback= nullptr,
                                      const ErrorCallback& error= nullptr);

            /// check if qrcode has been scanned
            static void get_login_info(const std::string oauthKey,
                                       const std::function<void(enum LoginInfo)> &callback = nullptr,
                                               const ErrorCallback& error = nullptr);

            /// get person info (if login)
            static void get_my_info(const std::function<void(UserResult)>& callback = nullptr,
                                    const ErrorCallback& error = nullptr);

            /// get person history videos
            static void get_my_history(const HistoryVideoListCursor& cursor,
                                       const std::function<void(HistoryVideoResultWrapper)>& callback = nullptr,
                                       const ErrorCallback& error = nullptr);


            /// get person collection list
            static void get_my_collection_list(const int mid, const int index=1, const int num=20,
                                               const std::function<void(CollectionListResultWrapper)>& callback = nullptr,
                                               const ErrorCallback& error = nullptr);

            static void get_my_collection_list(const std::string& mid, const int index=1, const int num=20,
                                               const std::function<void(CollectionListResultWrapper)>& callback = nullptr,
                                               const ErrorCallback& error = nullptr);

            /// get collection video list
            static void get_collection_video_list(int media_id, const int index=1, const int num=20,
                                                  const std::function<void(CollectionVideoListResultWrapper)>& callback = nullptr,
                                                  const ErrorCallback& error = nullptr);

            /// get user's upload videos
            static void get_user_videos(int mid, int pn, int ps,
                                        const std::function<void(UserUploadedVideoResultWrapper)>& callback = nullptr,
                                        const ErrorCallback& error = nullptr);

            /// get user's dynamic videos
            static void get_user_videos2(int mid, int pn, int ps,
                                        const std::function<void(UserDynamicVideoResultWrapper)>& callback = nullptr,
                                        const ErrorCallback& error = nullptr);

            /// get season detail by seasonID
            static void get_season_detail(const int seasonID,
                                         const std::function<void(SeasonResultWrapper)>& callback= nullptr,
                                         const ErrorCallback& error= nullptr);

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

            /// get season video url by cid
            static void get_season_url(const int cid, const int qn=64,
                                       const std::function<void(VideoUrlResult)>& callback= nullptr,
                                       const ErrorCallback& error= nullptr);

            /// get live video url by roomid
            static void get_live_url(const int roomid,
                                       const std::function<void(LiveUrlResultWrapper)>& callback= nullptr,
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

            /// 主页 直播推荐
            static void get_live_recommend(int parent_area_id, int area_id, int page, const std::string& source="pc",
                                          const std::function<void(LiveResultWrapper)>& callback= nullptr,
                                          const ErrorCallback& error= nullptr);

            /// 主页 追番列表
            static void get_bangumi(int is_refresh, const std::string& cursor,
                                           const std::function<void(PGCResultWrapper)>& callback= nullptr,
                                           const ErrorCallback& error= nullptr);


            /// 主页 影视列表
            static void get_cinema(int is_refresh, const std::string& cursor,
                                    const std::function<void(PGCResultWrapper)>& callback= nullptr,
                                    const ErrorCallback& error= nullptr);

            /// 主页 追番/影视 分类检索
            static void get_pgc_index(const string& param, int page=1,
                                   const std::function<void(PGCIndexResultWrapper)>& callback= nullptr,
                                   const ErrorCallback& error= nullptr);

            /// 主页 追番/影视 获取分类
            static void get_pgc_filter(const string& index_type,
                                      const std::function<void(PGCIndexFilterWrapper)>& callback= nullptr,
                                      const ErrorCallback& error= nullptr);

            /// 主页 追番/影视 获取全部分类
            static void get_pgc_all_filter(const std::function<void(PGCIndexFilters)>& callback= nullptr,
                                           const ErrorCallback& error= nullptr);

            /// 视频页 获取评论
            /// 3: 热门评论、2：最新评论 1：评论
            static void get_comment(int aid, int next, int mode=3,
                                   const std::function<void(VideoCommentResultWrapper)>& callback= nullptr,
                                   const ErrorCallback& error= nullptr);

            /// 搜索页 获取搜索视频内容
            static void search_video(const std::string& key, const std::string& search_type, uint index = 1,
                                     const std::string& order = "",
                                     const std::function<void(SearchResult)>& callback= nullptr,
                                     const ErrorCallback& error= nullptr);
            /// 搜索页 获取热搜榜
            static void get_search_hots(int limit = 20,
                                        const std::function<void(SearchHotsResultWrapper)>& callback= nullptr,
                                        const ErrorCallback& error= nullptr);

            /// 动态页 获取全部关注用户的最近动态
            static void dynamic_video(const uint page, const std::string& offset = "",
                                     const std::function<void(DynamicVideoListResultWrapper)>& callback= nullptr,
                                     const ErrorCallback& error= nullptr);

            /// 动态页 获取有最近动态的关注用户列表
            static void dynamic_up_list(const std::function<void(DynamicUpListResultWrapper)>& callback= nullptr,
                                      const ErrorCallback& error= nullptr);

            /// 初始化设置Cookie
            static void init(Cookies &cookies, std::function<void(Cookies)> writeCookiesCallback);
    };
}