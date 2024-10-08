#pragma once

#include <string>
#include <map>
#include <vector>
#include <future>

namespace bilibili {

class LiveResultWrapper;          // 直播推荐
class LiveUrlResultWrapper;       // 直播链接V1
class LiveRoomPlayInfo;           // 直播链接V2
class LiveDanmakuinfo;            // 直播弹幕连接信息
class LivePayInfo;                // 大航海专属直播信息
class LivePayLink;                // 大航海专属直播付费链接
class LiveFullAreaResultWrapper;  // 直播分区列表
class LiveSecondResultWrapper;    // 直播二级分区推荐
class SearchResult;
template <typename Item>
class DynamicListResultWrapper;                                                          // 动态页列表基类
class DynamicVideoResult;                                                                // 一条视频动态
class DynamicArticleResult;                                                              // 一条图文动态
class DynamicArticleResultWrapper;
typedef std::vector<DynamicVideoResult> DynamicVideoListResult;                          // 视频动态列表
typedef std::vector<DynamicArticleResult> DynamicArticleListResult;                      // 图文动态列表
typedef DynamicListResultWrapper<DynamicVideoListResult> DynamicVideoListResultWrapper;  // 动态 全部关注的视频列表
typedef DynamicListResultWrapper<DynamicArticleListResult>
    DynamicArticleListResultWrapper;  // 动态 全部或指定UP主图文列表
class DynamicUpListResultWrapper;     // 动态 最近更新的UP主列表
class UserDynamicVideoResultWrapper;  // 动态 单个up主视频列表
class MsgFeedCursor;                  // 消息页 回复列表游标
class FeedReplyResultWrapper;         // 消息页 回复列表
class FeedAtResultWrapper;            // 消息页 at 列表
class FeedLikeResultWrapper;          // 消息页 赞列表
class UserCardResult;
typedef std::vector<UserCardResult> UserCardListResult;
class InboxChatResultWrapper;
class InboxMessageResultWrapper;
class InboxSendResult;
class DynamicVideoResult;
class PGCIndexResultWrapper;
class PGCIndexFilterWrapper;
class PGCResultWrapper;
typedef std::map<std::string, PGCIndexFilterWrapper> PGCIndexFilters;
class SearchHotsResultWrapper;  // 搜索页 获取热搜榜
class VideoOnlineTotal;         // 某个视频在线人数，30s刷新一次
class VideoRelation;            // 某个视频点赞收藏情况
class VideoEpisodeRelation;     // 番剧的某一集的点赞收藏情况
class VideoUrlResult;           // 视频播放地址
class VideoHighlightProgress;   // 视频高能进度条
class VideoDetailPage;
typedef std::vector<VideoDetailPage> VideoDetailPageListResult;  // 视频分P列表 （视频详情API可以直接获取分P列表）
class VideoPageResult;                                           // 视频分P详情 （主要用来获取cc字幕）
class SubtitleData;                                              // 字幕数据
class VideoCommentResultWrapper;                                 // 视频评论
class VideoSingleCommentDetail;                                  //单条评论的相关回复
class VideoCommentAddResult;                                     // 发布评论的返回
class VideoDetailResult;                                         // 视频详情
class VideoDetailAllResult;  // 更详细的视频详情，包括 分P、合集、推荐、评论
class UserRelationStat;      // 用户关注/粉丝/黑名单 数量
class UserDynamicCount;      // 用户动态的数量
class UnixTimeResult;
class CollectionListResultWrapper;        // 用户收藏列表
class CollectionVideoListResultWrapper;   // 收藏夹 视频列表
class SimpleCollectionListResultWrapper;  // 单个视频在所有收藏夹中的收藏情况
class BangumiCollectionWrapper;           // 用户追番/追剧
enum class LoginInfo;
class UserResult;
class HistoryVideoListCursor;
class HistoryVideoResultWrapper;
class UserUploadedVideoResultWrapper;
class SeasonResultWrapper;
class SeasonRecommendWrapper;
class SeasonStatusResult;
class RecommendVideoListResultWrapper;
class HotsRankPGCVideoResult;
typedef std::vector<HotsRankPGCVideoResult> HotsRankPGCVideoListResult;
class HotsRankVideoResult;
typedef std::vector<HotsRankVideoResult> HotsRankVideoListResult;
class HotsHistoryVideoResult;
typedef std::vector<HotsHistoryVideoResult> HotsHistoryVideoListResult;
class HotsWeeklyVideoResult;
typedef std::vector<HotsWeeklyVideoResult> HotsWeeklyVideoListResult;
class HotsWeeklyResult;
typedef std::vector<HotsWeeklyResult> HotsWeeklyListResult;
class HotsAllVideoResult;
typedef std::vector<HotsAllVideoResult> HotsAllVideoListResult;
class SearchSuggestList;
class WatchLaterListWrapper;

using Cookies       = std::map<std::string, std::string>;
using ErrorCallback = std::function<void(const std::string&, int code)>;

#define BILI bilibili::BilibiliClient
#define BILI_ERR const std::string &error, int code

class BilibiliClient {
    inline static std::function<void(Cookies, std::string)> writeCookiesCallback = nullptr;

public:
    static Cookies cookies;
    inline static std::string FNVAL = "4048";
    inline static int VIDEO_CODEC   = 7;
    inline static int AUDIO_QUALITY = 30280;

    /// get qrcode for login
    static void get_login_url(const std::function<void(std::string, std::string)>& callback = nullptr,
                              const ErrorCallback& error                                    = nullptr);

    static void get_login_url_v2(const std::function<void(std::string, std::string)>& callback = nullptr,
                                 const ErrorCallback& error                                    = nullptr);

    /// check if qrcode has been scanned
    static void get_login_info(const std::string& oauthKey,
                               const std::function<void(enum LoginInfo)>& callback = nullptr,
                               const ErrorCallback& error                          = nullptr);

    static void get_login_info_v2(const std::string& qrcodeKey, const std::string& deviceName,
                                  const std::string& deviceID,
                                  const std::function<void(enum LoginInfo)>& callback = nullptr,
                                  const ErrorCallback& error                          = nullptr);

    /// get person info (if login)
    static void get_my_info(const std::function<void(UserResult)>& callback = nullptr,
                            const ErrorCallback& error                      = nullptr);

    /// 获取用户 关注/粉丝/黑名单数量
    static void get_user_relation(const std::string& mid,
                                  const std::function<void(UserRelationStat)>& callback = nullptr,
                                  const ErrorCallback& error                            = nullptr);

    /// 获取用户动态的数量
    static void get_user_dynamic_count(const std::string& mid,
                                       const std::function<void(UserDynamicCount)>& callback = nullptr,
                                       const ErrorCallback& error                            = nullptr);

    /// 批量获取用户昵称头像
    static void get_user_cards(const std::vector<std::string>& uids,
                               const std::function<void(UserCardListResult)>& callback = nullptr,
                               const ErrorCallback& error                              = nullptr);

    /// 私信消息记录
    static void new_inbox_sessions(time_t begin_ts                                             = 0,
                                   const std::function<void(InboxChatResultWrapper)>& callback = nullptr,
                                   const ErrorCallback& error                                  = nullptr);

    /// 私信消息记录
    static void fetch_inbox_msgs(const std::string& talker_id, size_t size = 20, int session_type = 1,
                                 const std::string& begin_seqno                                 = "",
                                 const std::function<void(InboxMessageResultWrapper)>& callback = nullptr,
                                 const ErrorCallback& error                                     = nullptr);

    static void update_inbox_ack(const std::string& talker_id, int session_type = 1, const std::string& ack_seqno = "",
                                 const std::string& csrf = "", const ErrorCallback& error = nullptr);

    static void send_inbox_msg(const std::string& sender_id, const std::string& receiver_id, const std::string& message,
                               const std::string& csrf                              = "",
                               const std::function<void(InboxSendResult)>& callback = nullptr,
                               const ErrorCallback& error                           = nullptr);

    /// 消息页 回复列表
    static void msg_feed_reply(const MsgFeedCursor& cursor,
                               const std::function<void(FeedReplyResultWrapper)>& callback = nullptr,
                               const ErrorCallback& error                                  = nullptr);

    /// 消息页 at 列表
    static void msg_feed_at(const MsgFeedCursor& cursor,
                            const std::function<void(FeedAtResultWrapper)>& callback = nullptr,
                            const ErrorCallback& error                               = nullptr);

    /// 消息页 收到的赞列表
    static void msg_feed_like(const MsgFeedCursor& cursor,
                              const std::function<void(FeedLikeResultWrapper)>& callback = nullptr,
                              const ErrorCallback& error                                 = nullptr);

    /// get person history videos
    static void get_my_history(const HistoryVideoListCursor& cursor,
                               const std::function<void(HistoryVideoResultWrapper)>& callback = nullptr,
                               const ErrorCallback& error                                     = nullptr);

    // 稍后再看 watch later
    static void getWatchLater(const std::function<void(WatchLaterListWrapper)>& callback = nullptr,
                              const ErrorCallback& error                                 = nullptr);

    /**
     * 获取用户创建的收藏列表或用户订阅的合集
     * @param mid
     * @param index
     * @param num
     * @param type  获取的列表种类是 收藏夹 还是 合集, 1 为收藏夹 2 为合集
     * @param callback
     * @param error
     */
    static void get_my_collection_list(uint64_t mid, int index = 1, int num = 20, int type = 1,
                                       const std::function<void(CollectionListResultWrapper)>& callback = nullptr,
                                       const ErrorCallback& error                                       = nullptr);

    static void get_my_collection_list(const std::string& mid, int index = 1, int num = 20, int type = 1,
                                       const std::function<void(CollectionListResultWrapper)>& callback = nullptr,
                                       const ErrorCallback& error                                       = nullptr);

    /**
     * 获取单个 收藏夹 或 合集 的视频列表
     * 注: 若获取的是合集，则一次性会获得全部列表
     * @param id 收藏夹或视频合集的 id
     * @param index 获取视频的列表的页号
     * @param num 一次获取的视频数量
     * @param type 获取的列表种类是 收藏夹 还是 合集, 1 为收藏夹 2 为合集
     * @param callback
     * @param error
     */
    static void get_collection_video_list(
        uint64_t id, int index = 1, int num = 20, int type = 1,
        const std::function<void(CollectionVideoListResultWrapper)>& callback = nullptr,
        const ErrorCallback& error                                            = nullptr);

    /**
     * 获取用户追番/追剧
     * @param mid 用户id
     * @param type 1: 追番 2: 追剧
     * @param pn 第几页
     * @param ps 一页多少个内容，默认20
     */
    static void get_my_bangumi(const std::string& mid, size_t type, size_t pn, size_t ps = 20,
                               const std::function<void(BangumiCollectionWrapper)>& callback = nullptr,
                               const ErrorCallback& error                                    = nullptr);

    /// get user's upload videos
    static void get_user_videos(uint64_t mid, int pn, int ps,
                                const std::function<void(UserUploadedVideoResultWrapper)>& callback = nullptr,
                                const ErrorCallback& error                                          = nullptr);

    /// get user's dynamic videos
    static void get_user_videos2(uint64_t mid, int pn, int ps,
                                 const std::function<void(UserDynamicVideoResultWrapper)>& callback = nullptr,
                                 const ErrorCallback& error                                         = nullptr);

    /// get season detail by seasonID
    static void get_season_detail(uint64_t seasonID, uint64_t epID = 0,
                                  const std::function<void(SeasonResultWrapper)>& callback = nullptr,
                                  const ErrorCallback& error                               = nullptr);

    /// 获取番剧相关推荐
    static void get_season_recommend(uint64_t seasonID,
                                     const std::function<void(SeasonRecommendWrapper)>& callback = nullptr,
                                     const ErrorCallback& error                                  = nullptr);

    /// 获取番剧的播放进度
    static void get_season_status(uint64_t seasonID, const std::function<void(SeasonStatusResult)>& callback,
                                  const ErrorCallback& error);

    /// get video detail by aid
    static void get_video_detail(uint64_t aid, const std::function<void(VideoDetailResult)>& callback = nullptr,
                                 const ErrorCallback& error = nullptr);

    /// get video detail by bvid
    static void get_video_detail(const std::string& bvid,
                                 const std::function<void(VideoDetailResult)>& callback = nullptr,
                                 const ErrorCallback& error                             = nullptr);

    static void get_video_detail_all(const std::string& bvid,
                                     const std::function<void(VideoDetailAllResult)>& callback = nullptr,
                                     const ErrorCallback& error                                = nullptr);

    /// 获取分P详情 （主要内容为cc字幕）
    static void get_page_detail(uint64_t aid, uint64_t cid, const std::function<void(VideoPageResult)>& callback = nullptr,
                                const ErrorCallback& error = nullptr);

    static void get_page_detail(const std::string& bvid, uint64_t cid,
                                const std::function<void(VideoPageResult)>& callback = nullptr,
                                const ErrorCallback& error                           = nullptr);

    /// 获取视频防遮挡数据
    static void get_webmask(const std::string& url, int64_t rangeStart, int64_t rangeEnd,
                            const std::function<void(std::string)>& callback = nullptr,
                            const ErrorCallback& error                       = nullptr);

    /// get video pagelist by aid
    static void get_video_pagelist(uint64_t aid, const std::function<void(VideoDetailPageListResult)>& callback = nullptr,
                                   const ErrorCallback& error = nullptr);

    /// get video pagelist by bvid
    static void get_video_pagelist(const std::string& bvid,
                                   const std::function<void(VideoDetailPageListResult)>& callback = nullptr,
                                   const ErrorCallback& error                                     = nullptr);

    /// get video url by aid & cid
    static void get_video_url(uint64_t aid, uint64_t cid, int qn = 64,
                              const std::function<void(VideoUrlResult)>& callback = nullptr,
                              const ErrorCallback& error                          = nullptr);

    /**
     * 获取投屏所需的视频链接
     * @param oid aid 或 epic
     * @param type 如果传入的普通视频的 aid 则为 1；如果是番剧类的 epid 则为 2
     */
    static void get_video_url_cast(uint64_t oid, uint64_t cid, int type, int qn = 64, const std::string& csrf = "",
                                   const std::function<void(VideoUrlResult)>& callback = nullptr,
                                   const ErrorCallback& error                          = nullptr);

    /// get video url by bvid & cid
    static void get_video_url(const std::string& bvid, uint64_t cid, int qn = 64,
                              const std::function<void(VideoUrlResult)>& callback = nullptr,
                              const ErrorCallback& error                          = nullptr);

    /// get season video url by cid
    static void get_season_url(uint64_t cid, int qn = 64, const std::function<void(VideoUrlResult)>& callback = nullptr,
                               const ErrorCallback& error = nullptr);

    /// get live video url by roomid
    /// @deprecated 部分直播间无法获取到直播地址
    static void get_live_url(int roomid, int qn = 10000,
                             const std::function<void(LiveUrlResultWrapper)>& callback = nullptr,
                             const ErrorCallback& error                                = nullptr);

    static void get_live_room_play_info(int roomid, int qn = 0,
                                        const std::function<void(LiveRoomPlayInfo)>& callback = nullptr,
                                        const ErrorCallback& error                            = nullptr);

    static void get_live_pay_info(int roomid, const std::function<void(LivePayInfo)>& callback = nullptr,
                                  const ErrorCallback& error = nullptr);

    /// 获取直播间弹幕服务器和连接 token
    static void get_live_danmaku_info(int roomid, const std::function<void(LiveDanmakuinfo)>& callback = nullptr,
                                      const ErrorCallback& error = nullptr);

    static void get_live_pay_link(int roomid, const std::function<void(LivePayLink)>& callback = nullptr,
                                  const ErrorCallback& error = nullptr);

    /**
     * 主页 推荐
     * @param index 页号，从1开始
     * @param num 每页数量 （手动刷新 30， 自动加载 15， 初始加载 10， 精选 10）
     * @param fresh_type 刷新类似（手动刷新是 3， 自动加载是 4，初始加载是 0）
     * @param feed_version 数据类型 （首页是 V1，精选是 CLIENT_SELECTED）
     * @param x_num 一行几个视频 （固定为 3）
     * @param y_num 一列几个视频 （默认为 4）
     * @param callback
     * @param error
     */
    static void get_recommend(int index = 1, int num = 24, int fresh_type = 4, std::string feed_version = "V1",
                              int x_num = 3, int y_num = 4,
                              const std::function<void(RecommendVideoListResultWrapper)>& callback = nullptr,
                              const ErrorCallback& error                                           = nullptr);

    /// 主页 热门 热门综合
    static void get_hots_all(int index = 1, int num = 40,
                             const std::function<void(HotsAllVideoListResult, bool)>& callback = nullptr,
                             const ErrorCallback& error                                        = nullptr);

    /// 主页 热门 每周推荐列表
    static void get_hots_weekly_list(const std::function<void(HotsWeeklyListResult)>& callback = nullptr,
                                     const ErrorCallback& error                                = nullptr);

    /// 主页 热门 每周推荐
    static void get_hots_weekly(
        int number, const std::function<void(HotsWeeklyVideoListResult, std::string, std::string)>& callback = nullptr,
        const ErrorCallback& error = nullptr);

    /// 主页 热门 入站必刷
    static void get_hots_history(const std::function<void(HotsHistoryVideoListResult, std::string)>& callback = nullptr,
                                 const ErrorCallback& error = nullptr);

    /// 主页 热门 排行榜 投稿视频
    static void get_hots_rank(int rid, const std::string& type = "all",
                              const std::function<void(HotsRankVideoListResult, std::string)>& callback = nullptr,
                              const ErrorCallback& error                                                = nullptr);

    /// 主页 热门 排行榜 官方
    static void get_hots_rank_pgc(
        int season_type, int day = 3,
        const std::function<void(HotsRankPGCVideoListResult, std::string)>& callback = nullptr,
        const ErrorCallback& error                                                   = nullptr);

    /// 主页 直播推荐
    static void get_live_recommend(int parent_area_id, int area_id, int page, const std::string& source = "pc",
                                   const std::function<void(LiveResultWrapper)>& callback = nullptr,
                                   const ErrorCallback& error                             = nullptr);

    /// 主页 直播二级分区推荐, 不包含关注
    /// area_id 为 0 则推荐 parent_area 全部视频
    static void get_live_recommend_second(int parent_area_id, int area_id, int page,
                                          const std::function<void(LiveSecondResultWrapper)>& callback = nullptr,
                                          const ErrorCallback& error                                   = nullptr);

    /// 主页 直播分区
    static void get_live_area_list(const std::function<void(LiveFullAreaResultWrapper)>& callback = nullptr,
                                   const ErrorCallback& error                                     = nullptr);

    /// 主页 追番列表
    static void get_bangumi(int is_refresh, const std::string& cursor,
                            const std::function<void(PGCResultWrapper)>& callback = nullptr,
                            const ErrorCallback& error                            = nullptr);

    /// 主页 影视列表
    static void get_cinema(int is_refresh, const std::string& cursor,
                           const std::function<void(PGCResultWrapper)>& callback = nullptr,
                           const ErrorCallback& error                            = nullptr);

    /// 主页 追番/影视 分类检索
    static void get_pgc_index(const std::string& param, int page = 1,
                              const std::function<void(PGCIndexResultWrapper)>& callback = nullptr,
                              const ErrorCallback& error                                 = nullptr);

    /// 主页 追番/影视 获取分类
    static void get_pgc_filter(const std::string& index_type,
                               const std::function<void(PGCIndexFilterWrapper)>& callback = nullptr,
                               const ErrorCallback& error                                 = nullptr);

    /// 主页 追番/影视 获取全部分类
    static void get_pgc_all_filter(const std::function<void(PGCIndexFilters)>& callback = nullptr,
                                   const ErrorCallback& error                           = nullptr);

    /// 获取评论
    /// @mode: 3: 热门评论、2：最新评论 1：评论
    /// @type: 1: 视频 11: 图片动态 17: 文字动态 ...
    static void get_comment(const std::string& oid, int next, int mode = 3, int type = 1,
                            const std::function<void(VideoCommentResultWrapper)>& callback = nullptr,
                            const ErrorCallback& error                                     = nullptr);

    /// 获取单条评论详情
    static void get_comment_detail(const std::string& access_key, const std::string& oid, uint64_t rpid, size_t next = 0,
                                   int type                                                      = 1,
                                   const std::function<void(VideoSingleCommentDetail)>& callback = nullptr,
                                   const ErrorCallback& error                                    = nullptr);

    /// 点赞评论
    static void be_agree_comment(const std::string& access_key, const std::string& oid, uint64_t rpid, bool is_like,
                                 int type = 1, const std::function<void()>& callback = nullptr,
                                 const ErrorCallback& error = nullptr);

    /// 点踩评论
    static void be_disagree_comment(const std::string& access_key, const std::string& oid, uint64_t rpid, bool is_dislike,
                                 int type = 1, const std::function<void()>& callback = nullptr,
                                 const ErrorCallback& error = nullptr);

    /// 点赞动态
    static void be_agree_dynamic(const std::string& access_key, const std::string& id, bool is_like,
                                 const std::function<void()>& callback = nullptr, const ErrorCallback& error = nullptr);

    /**
     * 回复评论
     * @param access_key csrf key
     * @param message 消息内容
     * @param oid 视频的aid
     * @param parent 回复评论的id，若评论视频则无此参数
     * @param root 评论所在楼层的id，若评论视频则无此参数
     * @param callback
     * @param error
     */
    static void add_comment(const std::string& access_key, const std::string& message, const std::string& oid,
                            uint64_t parent = 0, uint64_t root = 0, int type = 1,
                            const std::function<void(VideoCommentAddResult)>& callback = nullptr,
                            const ErrorCallback& error                                 = nullptr);

    /// 删除评论
    static void delete_comment(const std::string& access_key, const std::string& oid, uint64_t rpid, int type = 1,
                               const std::function<void()>& callback = nullptr, const ErrorCallback& error = nullptr);

    /// 视频页 获取单个视频播放人数
    static void get_video_online(uint64_t aid, uint64_t cid, const std::function<void(VideoOnlineTotal)>& callback = nullptr,
                                 const ErrorCallback& error = nullptr);

    static void get_video_online(const std::string& bvid, uint64_t cid,
                                 const std::function<void(VideoOnlineTotal)>& callback = nullptr,
                                 const ErrorCallback& error                            = nullptr);

    /// 视频页 获取点赞/收藏/投币情况
    static void get_video_relation(const std::string& bvid,
                                   const std::function<void(VideoRelation)>& callback = nullptr,
                                   const ErrorCallback& error                         = nullptr);

    static void get_video_relation(uint64_t epid, const std::function<void(VideoEpisodeRelation)>& callback = nullptr,
                                   const ErrorCallback& error = nullptr);

    /// 视频页 获取弹幕的xml文件
    static void get_danmaku(uint64_t cid, const std::function<void(std::string)>& callback = nullptr,
                            const ErrorCallback& error = nullptr);

    /// 视频页 获取字幕
    static void get_subtitle(const std::string& link, const std::function<void(SubtitleData)>& callback = nullptr,
                             const ErrorCallback& error = nullptr);

    /// 视频页 获取高能进度条
    static void get_highlight_progress(uint64_t cid,
                                       const std::function<void(VideoHighlightProgress)>& callback = nullptr,
                                       const ErrorCallback& error                                  = nullptr);

    /// 视频页 上报历史记录
    static void report_history(const std::string& mid, const std::string& access_key, uint64_t aid,
                               uint64_t cid, int type = 3, unsigned int progress = 0, unsigned int duration = 0,
                               uint64_t sid = 0, uint64_t epid = 0,
                               const std::function<void()>& callback = nullptr, const ErrorCallback& error = nullptr);

    /// 直播页 上报观看记录
    static void report_live_history(int room, const std::string& csrf, const std::function<void()>& callback = nullptr,
                                    const ErrorCallback& error = nullptr);

    /// 点赞
    static void be_agree(const std::string& access_key, uint64_t aid, bool is_like,
                         const std::function<void()>& callback = nullptr, const ErrorCallback& error = nullptr);

    /// 投币
    static void add_coin(const std::string& access_key, uint64_t aid, unsigned int coin_number, bool is_like,
                         const std::function<void()>& callback = nullptr, const ErrorCallback& error = nullptr);

    /// 投币经验值
    static void get_coin_exp(const std::function<void(int)>& callback = nullptr, const ErrorCallback& error = nullptr);

    /// 关注或取关UP主
    static void follow_up(const std::string& access_key, const std::string& mid, bool follow = true,
                          const std::function<void()>& callback = nullptr, const ErrorCallback& error = nullptr);

    /// 追剧或取消追剧
    static void follow_season(const std::string& access_key, uint64_t season, bool follow = true,
                              const std::function<void()>& callback = nullptr, const ErrorCallback& error = nullptr);

    /**
     * 收藏视频
     * @param access_key
     * @param rid 普通视频: aid, 番剧: epid
     * @param type 普通视频:2, 番剧: 24; 暂且没找到在哪里可以获取到这个信息，所以hardcode吧
     * @param add_ids eg: 1231243
     * @param del_ids eg: 123123,123213,12321231
     * @param callback 若成功 进行回调
     * @param error 若失败返回失败原因
     */
    static void add_resource(const std::string& access_key, uint64_t rid, int type, const std::string& add_ids,
                             const std::string& del_ids, const std::function<void()>& callback = nullptr,
                             const ErrorCallback& error = nullptr);

    /**
     * 订阅合集
     * @param id 合集 id
     * @param csrf
     * @param callback
     * @param error
     */
    static void ugc_season_subscribe(int id, const std::string& csrf, const std::function<void()>& callback = nullptr,
                                     const ErrorCallback& error = nullptr);

    /**
     * 取消订阅合集
     * @param id 合集 id
     * @param csrf
     * @param callback
     * @param error
     */
    static void ugc_season_unsubscribe(int id, const std::string& csrf, const std::function<void()>& callback = nullptr,
                                       const ErrorCallback& error = nullptr);

    /**
     * 获取对应视频
     * @param rid
     * @param type 普通视频:2, 番剧: 24
     * @param mid
     * @param callback
     * @param error
     */
    static void get_collection_list_all(
        uint64_t rid, int type, const std::string& mid,
        const std::function<void(SimpleCollectionListResultWrapper)>& callback = nullptr,
        const ErrorCallback& error                                             = nullptr);

    /// 三连
    static void triple_like(const std::string& access_key, uint64_t aid, const std::function<void()>& callback = nullptr,
                            const ErrorCallback& error = nullptr);

    /// 搜索页 获取搜索视频内容
    static void search_video(const std::string& key, const std::string& search_type, unsigned int index = 1,
                             const std::string& order = "", const std::function<void(SearchResult)>& callback = nullptr,
                             const ErrorCallback& error = nullptr);
    /// 搜索页 获取热搜榜
    static void get_search_hots(int limit = 20, const std::function<void(SearchHotsResultWrapper)>& callback = nullptr,
                                const ErrorCallback& error = nullptr);
    /// 搜索页 获取TV搜索建议
    static void get_search_suggest_tv(const std::string& key,
                                      const std::function<void(SearchSuggestList)>& callback = nullptr,
                                      const ErrorCallback& error                             = nullptr);

    /// 动态页 获取全部关注用户的最近动态 视频
    static void dynamic_video(const unsigned int page, const std::string& offset = "",
                              const std::function<void(DynamicVideoListResultWrapper)>& callback = nullptr,
                              const ErrorCallback& error                                         = nullptr);

    /// 动态页 获取全部关注用户的最近动态 图文
    static void dynamic_article(const unsigned int page, const std::string& offset = "", uint64_t mid = 0,
                                const std::function<void(DynamicArticleListResultWrapper)>& callback = nullptr,
                                const ErrorCallback& error                                           = nullptr);

    /// 动态页 获取有最近动态的关注用户列表
    static void dynamic_up_list(const std::function<void(DynamicUpListResultWrapper)>& callback = nullptr,
                                const ErrorCallback& error                                      = nullptr);

    /// 获取单个动态详情
    static void get_dynamic_detail(const std::string& id,
                                   const std::function<void(DynamicArticleResultWrapper)>& callback = nullptr,
                                   const ErrorCallback& error                              = nullptr);

    /// 设置页 获取网络时间
    static void get_unix_time(const std::function<void(UnixTimeResult)>& callback = nullptr,
                              const ErrorCallback& error                          = nullptr);

    /// 初始化设置Cookie
    static void init(Cookies& cookies, std::function<void(Cookies, std::string)> writeCookiesCallback,
                     int timeout = 10000, const std::string& httpProxy = "", const std::string& httpsProxy = "",
                     bool tlsVerify = true);

    static void setProxy(const std::string& httpProxy = "", const std::string& httpsProxy = "");

    static void setTlsVerify(bool value);

    static std::string genRandomBuvid3();
};
}  // namespace bilibili