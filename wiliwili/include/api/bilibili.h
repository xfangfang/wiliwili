#pragma once

#include <string>
#include <vector>
#include <future>
#include "bilibili/api.h"
#include "bilibili/util/md5.hpp"
#include "bilibili/util/http.hpp"
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
class DynamicVideoListResultWrapper;  // 动态 全部关注的视频列表
class DynamicUpListResultWrapper;     // 动态 最近更新的UP主列表
class UserDynamicVideoResultWrapper;  // 动态 单个up主视频列表
class PGCIndexResultWrapper;
class PGCIndexFilterWrapper;
class PGCResultWrapper;
typedef std::map<std::string, PGCIndexFilterWrapper> PGCIndexFilters;
class SearchHotsResultWrapper;  // 搜索页 获取热搜榜
class VideoOnlineTotal;         // 某个视频在线人数，30s刷新一次
class VideoRelation;            // 某个视频点赞收藏情况
class VideoUrlResult;           // 视频播放地址
class VideoDetailPage;
typedef std::vector<VideoDetailPage>
    VideoDetailPageListResult;  // 视频分P列表 （视频详情API可以直接过去分P列表）
class VideoCommentResultWrapper;  // 视频评论
class VideoDetailResult;          // 视频详情
class VideoDetailAllResult;  // 更详细的视频详情，包括 分P、合集、推荐、评论
class UserRelationStat;  // 用户关注/粉丝/黑名单 数量
class UserDynamicCount;  // 用户动态的数量
class UnixTimeResult;

using Cookies = std::map<std::string, std::string>;

class BilibiliClient {
    inline static std::function<void(Cookies)> writeCookiesCallback = nullptr;

public:
    static Cookies cookies;
    inline static std::string FNVAL = "1744";

    /// get qrcode for login
    static void get_login_url(
        const std::function<void(std::string, std::string)>& callback = nullptr,
        const ErrorCallback& error = nullptr);

    /// check if qrcode has been scanned
    static void get_login_info(
        const std::string oauthKey,
        const std::function<void(enum LoginInfo)>& callback = nullptr,
        const ErrorCallback& error                          = nullptr);

    /// get person info (if login)
    static void get_my_info(
        const std::function<void(UserResult)>& callback = nullptr,
        const ErrorCallback& error                      = nullptr);

    /// 获取用户 关注/粉丝/黑名单数量
    static void get_user_relation(
        const std::string& mid,
        const std::function<void(UserRelationStat)>& callback = nullptr,
        const ErrorCallback& error                            = nullptr);

    /// 获取用户动态的数量
    static void get_user_dynamic_count(
        const std::string& mid,
        const std::function<void(UserDynamicCount)>& callback = nullptr,
        const ErrorCallback& error                            = nullptr);

    /// get person history videos
    static void get_my_history(
        const HistoryVideoListCursor& cursor,
        const std::function<void(HistoryVideoResultWrapper)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// get person collection list
    static void get_my_collection_list(
        const int64_t mid, const int index = 1, const int num = 20,
        const std::function<void(CollectionListResultWrapper)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    static void get_my_collection_list(
        const std::string& mid, const int index = 1, const int num = 20,
        const std::function<void(CollectionListResultWrapper)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// get collection video list
    static void get_collection_video_list(
        int media_id, const int index = 1, const int num = 20,
        const std::function<void(CollectionVideoListResultWrapper)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// get user's upload videos
    static void get_user_videos(
        const int64_t mid, int pn, int ps,
        const std::function<void(UserUploadedVideoResultWrapper)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// get user's dynamic videos
    static void get_user_videos2(
        const int64_t mid, int pn, int ps,
        const std::function<void(UserDynamicVideoResultWrapper)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// get season detail by seasonID
    static void get_season_detail(
        const int seasonID, const int epID = 0,
        const std::function<void(SeasonResultWrapper)>& callback = nullptr,
        const ErrorCallback& error                               = nullptr);

    /// get video detail by aid
    static void get_video_detail(
        const int aid,
        const std::function<void(VideoDetailResult)>& callback = nullptr,
        const ErrorCallback& error                             = nullptr);

    /// get video detail by bvid
    static void get_video_detail(
        const std::string& bvid,
        const std::function<void(VideoDetailResult)>& callback = nullptr,
        const ErrorCallback& error                             = nullptr);

    static void get_video_detail_all(
        const std::string& bvid,
        const std::function<void(VideoDetailAllResult)>& callback = nullptr,
        const ErrorCallback& error                                = nullptr);

    /// get video pagelist by aid
    static void get_video_pagelist(
        const int aid,
        const std::function<void(VideoDetailPageListResult)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// get video pagelist by bvid
    static void get_video_pagelist(
        const std::string& bvid,
        const std::function<void(VideoDetailPageListResult)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// get video url by aid & cid
    static void get_video_url(
        const int aid, const int cid, const int qn = 64,
        const std::function<void(VideoUrlResult)>& callback = nullptr,
        const ErrorCallback& error                          = nullptr);

    /// get video url by bvid & cid
    static void get_video_url(
        const std::string& bvid, const int cid, const int qn = 64,
        const std::function<void(VideoUrlResult)>& callback = nullptr,
        const ErrorCallback& error                          = nullptr);

    /// get season video url by cid
    static void get_season_url(
        const int cid, const int qn = 64,
        const std::function<void(VideoUrlResult)>& callback = nullptr,
        const ErrorCallback& error                          = nullptr);

    /// get live video url by roomid
    static void get_live_url(
        const int roomid, const int qn = 10000,
        const std::function<void(LiveUrlResultWrapper)>& callback = nullptr,
        const ErrorCallback& error                                = nullptr);

    /// 主页 推荐
    static void get_recommend(
        const int index = 1, const int num = 24,
        const std::function<void(RecommendVideoListResultWrapper)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// 主页 热门 热门综合
    static void get_hots_all(
        const int index = 1, const int num = 40,
        const std::function<void(HotsAllVideoListResult, bool)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// 主页 热门 每周推荐列表
    static void get_hots_weekly_list(
        const std::function<void(HotsWeeklyListResult)>& callback = nullptr,
        const ErrorCallback& error                                = nullptr);

    /// 主页 热门 每周推荐
    static void get_hots_weekly(
        const int number,
        const std::function<void(HotsWeeklyVideoListResult, std::string,
                                 std::string)>& callback = nullptr,
        const ErrorCallback& error                       = nullptr);

    /// 主页 热门 入站必刷
    static void get_hots_history(
        const std::function<void(HotsHistoryVideoListResult, std::string)>&
            callback               = nullptr,
        const ErrorCallback& error = nullptr);

    /// 主页 热门 排行榜 投稿视频
    static void get_hots_rank(
        const int rid, const std::string type = "all",
        const std::function<void(HotsRankVideoListResult, std::string)>&
            callback               = nullptr,
        const ErrorCallback& error = nullptr);

    /// 主页 热门 排行榜 官方
    static void get_hots_rank_pgc(
        const int season_type, const int day = 3,
        const std::function<void(HotsRankPGCVideoListResult, std::string)>&
            callback               = nullptr,
        const ErrorCallback& error = nullptr);

    /// 主页 直播推荐
    static void get_live_recommend(
        int parent_area_id, int area_id, int page,
        const std::string& source                              = "pc",
        const std::function<void(LiveResultWrapper)>& callback = nullptr,
        const ErrorCallback& error                             = nullptr);

    /// 主页 追番列表
    static void get_bangumi(
        int is_refresh, const std::string& cursor,
        const std::function<void(PGCResultWrapper)>& callback = nullptr,
        const ErrorCallback& error                            = nullptr);

    /// 主页 影视列表
    static void get_cinema(
        int is_refresh, const std::string& cursor,
        const std::function<void(PGCResultWrapper)>& callback = nullptr,
        const ErrorCallback& error                            = nullptr);

    /// 主页 追番/影视 分类检索
    static void get_pgc_index(
        const std::string& param, int page = 1,
        const std::function<void(PGCIndexResultWrapper)>& callback = nullptr,
        const ErrorCallback& error                                 = nullptr);

    /// 主页 追番/影视 获取分类
    static void get_pgc_filter(
        const std::string& index_type,
        const std::function<void(PGCIndexFilterWrapper)>& callback = nullptr,
        const ErrorCallback& error                                 = nullptr);

    /// 主页 追番/影视 获取全部分类
    static void get_pgc_all_filter(
        const std::function<void(PGCIndexFilters)>& callback = nullptr,
        const ErrorCallback& error                           = nullptr);

    /// 视频页 获取评论
    /// 3: 热门评论、2：最新评论 1：评论
    static void get_comment(
        int aid, int next, int mode = 3,
        const std::function<void(VideoCommentResultWrapper)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// 视频页 获取单个视频播放人数
    static void get_video_online(
        int aid, int cid,
        const std::function<void(VideoOnlineTotal)>& callback = nullptr,
        const ErrorCallback& error                            = nullptr);

    static void get_video_online(
        const std::string& bvid, int cid,
        const std::function<void(VideoOnlineTotal)>& callback = nullptr,
        const ErrorCallback& error                            = nullptr);

    /// 视频页 获取点赞/收藏/投屏情况
    static void get_video_relation(
        const std::string& bvid,
        const std::function<void(VideoRelation)>& callback = nullptr,
        const ErrorCallback& error                         = nullptr);

    /// 视频页 获取弹幕的xml文件
    static void get_danmaku(
        const unsigned int cid,
        const std::function<void(std::string)>& callback = nullptr,
        const ErrorCallback& error                       = nullptr);

    /// 视频页 上报历史记录
    static void report_history(const std::string& mid,
                               const std::string& access_key, unsigned int aid,
                               unsigned int cid, int type = 3,
                               unsigned int progress = 0,
                               unsigned int duration = 0, unsigned int sid = 0,
                               unsigned int epid                     = 0,
                               const std::function<void()>& callback = nullptr,
                               const ErrorCallback& error            = nullptr);

    /// 直播页 上报观看记录
    static void report_live_history(
        const int room, const std::string& csrf,
        const std::function<void()>& callback = nullptr,
        const ErrorCallback& error            = nullptr);

    /// 点赞
    static void be_agree(const std::string& access_key, int aid, bool is_like,
                         const std::function<void()>& callback = nullptr,
                         const ErrorCallback& error            = nullptr);

    /// 投币
    static void add_coin(const std::string& access_key, int aid,
                         unsigned int coin_number, bool is_like,
                         const std::function<void()>& callback = nullptr,
                         const ErrorCallback& error            = nullptr);

    /// 收藏
    static void add_resource(const std::string& access_key, int aid,
                             const std::function<void()>& callback = nullptr,
                             const ErrorCallback& error            = nullptr);

    /// 搜索页 获取搜索视频内容
    static void search_video(
        const std::string& key, const std::string& search_type,
        unsigned int index = 1, const std::string& order = "",
        const std::function<void(SearchResult)>& callback = nullptr,
        const ErrorCallback& error                        = nullptr);
    /// 搜索页 获取热搜榜
    static void get_search_hots(
        int limit                                                    = 20,
        const std::function<void(SearchHotsResultWrapper)>& callback = nullptr,
        const ErrorCallback& error                                   = nullptr);

    /// 动态页 获取全部关注用户的最近动态
    static void dynamic_video(
        const unsigned int page, const std::string& offset = "",
        const std::function<void(DynamicVideoListResultWrapper)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// 动态页 获取有最近动态的关注用户列表
    static void dynamic_up_list(
        const std::function<void(DynamicUpListResultWrapper)>& callback =
            nullptr,
        const ErrorCallback& error = nullptr);

    /// 设置页 获取网络时间
    static void get_unix_time(
        const std::function<void(UnixTimeResult)>& callback = nullptr,
        const ErrorCallback& error                          = nullptr);

    /// 初始化设置Cookie
    static void init(Cookies& cookies,
                     std::function<void(Cookies)> writeCookiesCallback,
                     int timeout = 10000);
};
}  // namespace bilibili