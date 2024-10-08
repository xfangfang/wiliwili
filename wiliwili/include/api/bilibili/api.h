//
// Created by fang on 2022/5/1.
//

#pragma once

namespace bilibili {

namespace Api {

const std::string _apiBase     = "https://api.bilibili.com";
const std::string _appBase     = "https://app.bilibili.com";
const std::string _vcBase      = "https://api.vc.bilibili.com";
const std::string _bvcBase     = "https://bvc.bilivideo.com";
const std::string _liveBase    = "https://api.live.bilibili.com";
const std::string _passBase    = "https://passport.bilibili.com";
const std::string _bangumiBase = "https://bangumi.bilibili.com";
const std::string _grpcBase    = "https://grpc.biliapi.net";

/// ===
/// 视频API
/// ===

/// 视频详情. gRPC
const std::string DetailGRPC = _grpcBase + "/bilibili.app.view.v1.View/View";
/// 视频详情.
const std::string Detail    = _apiBase + "/x/web-interface/view";
const std::string DetailAll = _apiBase + "/x/web-interface/view/detail";
/// 视频分P详情
const std::string PageDetail = _apiBase + "/x/player/v2";
/// 番剧详情.
const std::string SeasonDetail = _apiBase + "/pgc/view/pc/season";
/// 番剧推荐.
const std::string SeasonRCMD = _apiBase + "/pgc/season/web/related/recommend";
/// 番剧播放进度
const std::string SeasonStatus = _apiBase + "/pgc/view/web/season/user/status";
/// 在线观看人数.
const std::string OnlineViewerCount = _apiBase + "/x/player/online/total";
/// 视频播放信息.
const std::string PlayInformation = _apiBase + "/x/player/playurl";
/// 视频播放地址 TV.
const std::string PlayUrlTV = _apiBase + "/x/tv/card/view_v2";
/// 视频播放地址 投屏
const std::string PlayUrlCast = _apiBase + "/x/tv/playurl";
/// 番剧播放地址
const std::string SeasonUrl = _apiBase + "/pgc/player/web/playurl";
/// 视频分P列表.
const std::string PlayPageList = _apiBase + "/x/player/pagelist";
/// 视频播放信息.
const std::string PlayConfig = _appBase + "/bilibili.app.playurl.v1.PlayURL/PlayConf";
/// 弹幕元数据.
const std::string DanmakuMetaData = _grpcBase + "/bilibili.community.service.dm.v1.DM/DmView";
/// 分段弹幕.
const std::string SegmentDanmaku = _grpcBase + "/bilibili.community.service.dm.v1.DM/DmSegMobile";
/// 历史记录.
const std::string ProgressReport = _apiBase + "/x/v2/history/report";
/// 点赞视频.
const std::string Like    = _appBase + "/x/v2/view/like";
const std::string LikeWeb = _apiBase + "/x/web-interface/archive/like";
/// 给视频投币.
const std::string Coin    = _appBase + "/x/v2/view/coin/add";
const std::string CoinWeb = _apiBase + "/x/web-interface/coin/add";
/// 投币经验值
const std::string CoinExp = _apiBase + "/x/web-interface/coin/today/exp";
/// 添加或删除视频收藏.
const std::string ModifyFavorite = _apiBase + "/x/v3/fav/resource/deal";
/// 添加视频合集订阅
const std::string UGCSeasonSubscribe = _apiBase + "/x/v3/season/fav";
/// 移除视频合集订阅
const std::string UGCSeasonUnsubscribe = _apiBase + "/x/v3/season/unfav";
/// 一键三连.
const std::string Triple    = _appBase + "/x/v2/view/like/triple";
const std::string TripleWeb = _apiBase + "x/web-interface/archive/like/triple";
/// 关注 取关
const std::string Follow = _apiBase + "/x/relation/modify";
/// 番剧 追剧/取消追剧
const std::string FollowSeason     = _apiBase + "/pgc/app/follow/add";
const std::string UndoFollowSeason = _apiBase + "/pgc/app/follow/del";
/// 发送弹幕.
const std::string SendDanmaku = _apiBase + "/x/v2/dm/post";
/// 获取视频字幕.
const std::string Subtitle = _apiBase + "/x/player.so";
/// 获取互动视频选项.
const std::string InteractionEdge = _apiBase + "/x/stein/edgeinfo_v2";
/// 获取视频参数.
const std::string Stat = _apiBase + "/x/web-interface/archive/stat";
/// 获取视频点赞收藏情况
const std::string VideoRelation = _apiBase + "/x/web-interface/archive/relation";
/// 获取番剧视频点赞收藏情况
const std::string VideoEpisodeRelation = _apiBase + "/pgc/season/episode/web/info";
/// 获取视频弹幕
const std::string VideoDanmaku = _apiBase + "/x/v1/dm/list.so";
/// 获取高能进度条
const std::string VideoHighlight = _bvcBase + "/pbp/data";
/// 获取直播弹幕token
const std::string LiveDanmakuInfo = _liveBase + "/xlive/web-room/v1/index/getDanmuInfo";
/// 直播API
const std::string LiveUrl = _liveBase + "/room/v1/Room/playUrl";
/// 直播API V2
const std::string RoomPlayInfo = _liveBase + "/xlive/web-room/v2/index/getRoomPlayInfo";
/// 是否为大航海专属直播
const std::string RoomPayInfo = _liveBase + "/av/v1/PayLive/liveValidate";
/// 大航海直播的付费链接
const std::string RoomPayLink = _liveBase + "/xlive/web-ucenter/v1/payPlay/getInfo";
/// 直播历史记录
const std::string LiveReport   = _liveBase + "/xlive/web-room/v1/index/roomEntryAction";
const std::string LiveAreaList = _liveBase + "/xlive/app-interface/v2/index/getAreaList";

/// ===
/// 主页API
/// ===

/// 主页 推荐
const std::string Recommend = _apiBase + "/x/web-interface/index/top/feed/rcmd";
/// 主页 热门 热门综合
const std::string HotsAll = _apiBase + "/x/web-interface/popular";
/// 主页 热门 每周推荐列表
const std::string HotsWeeklyList = _apiBase + "/x/web-interface/popular/series/list";
/// 主页 热门 每周推荐
const std::string HotsWeekly = _apiBase + "/x/web-interface/popular/series/one";
/// 主页 热门 入站必刷
const std::string HotsHistory = _apiBase + "/x/web-interface/popular/precious";
/// 主页 热门 用户投稿排行榜
const std::string HotsRank = _apiBase + "/x/web-interface/ranking/v2";
/// 主页 热门 官方视频(番剧 电影...)排行榜
const std::string HotsRankPGC = _apiBase + "/pgc/season/rank/web/list";
/// 主页 直播推荐
const std::string LiveFeed = _liveBase + "/xlive/app-interface/v2/index/feedV2";
/// 主页 直播推荐 second
const std::string LiveFeedSecond = _liveBase + "/xlive/app-interface/v2/second/getList";
/// 主页 追番
const std::string Bangumi = _apiBase + "/pgc/page/pc/bangumi/tab";
/// 主页 影视
const std::string Cinema = _apiBase + "/pgc/page/pc/cinema/tab";
/// 主页 追番/影视 分类检索
const std::string PGCIndex = _apiBase + "/pgc/page/index/result";
/// 主页 追番/影视 分类检索过滤器列表
const std::string PGCIndexFilter = _apiBase + "/pgc/page/index/condition";
/// 视频 评论
const std::string Comment = _apiBase + "/x/v2/reply/main";
/// 单条评论详情
const std::string CommentDetail = _apiBase + "/x/v2/reply/detail";
/// 点赞评论
const std::string CommentLike = _apiBase + "/x/v2/reply/action";
/// 点踩评论
const std::string CommentDisLike = _apiBase + "/x/v2/reply/hate";
/// 发布评论
const std::string CommentAdd = _apiBase + "/x/v2/reply/add";
/// 删除评论
const std::string CommentDel = _apiBase + "/x/v2/reply/del";
/// 热门 - gRPC.
const std::string PopularGRPC = _grpcBase + "/bilibili.app.show.v1.Popular/Index";
/// 排行榜 - Web.
const std::string Ranking = _apiBase + "/x/web-interface/ranking/v2";
/// 排行榜 - gRPC.
const std::string RankingGRPC = _grpcBase + "/bilibili.app.show.v1.Rank/RankRegion";

/// ===
/// 个人页API
/// ===
const std::string QrLoginUrl              = _passBase + "/qrcode/getLoginUrl";
const std::string QrLoginInfo             = _passBase + "/qrcode/getLoginInfo";
const std::string QrLoginUrlV2            = _passBase + "/x/passport-login/web/qrcode/generate";
const std::string QrLoginInfoV2           = _passBase + "/x/passport-login/web/qrcode/poll";
const std::string CheckRefreshToken       = _passBase + "/x/passport-login/web/cookie/info";
const std::string MyInfo                  = _apiBase + "/x/space/myinfo";
const std::string HistoryVideo            = _apiBase + "/x/web-interface/history/cursor";
const std::string CollectionList          = _apiBase + "/x/v3/fav/folder/created/list";
const std::string CollectionListAll       = _apiBase + "/x/v3/fav/folder/created/list-all";
const std::string CollectionVideoList     = _apiBase + "/x/v3/fav/resource/list";
const std::string CollectionVideoListSave = _apiBase + "/x/v3/fav/resource/deal";
const std::string UserUploadedVideo       = _apiBase + "/x/space/arc/search";
const std::string UserRelationStat        = _apiBase + "/x/relation/stat";
const std::string MsgFeedLike             = _apiBase + "/x/msgfeed/like";
const std::string MsgFeedAt               = _apiBase + "/x/msgfeed/at";
const std::string MsgFeedReply            = _apiBase + "/x/msgfeed/reply";
const std::string UserCards               = _vcBase + "/account/v1/user/cards";
const std::string UserDynamicStat         = _apiBase + "/x/dynamic/feed/space/dyn_num";
const std::string ChatSessions            = _vcBase + "/session_svr/v1/session_svr/new_sessions";
const std::string ChatUpdateAct           = _vcBase + "/session_svr/v1/session_svr/update_ack";
const std::string ChatFetchMsgs           = _vcBase + "/svr_sync/v1/svr_sync/fetch_session_msgs";
const std::string ChatSendMsg             = _vcBase + "/web_im/v1/web_im/send_msg";
/// 用户追番/追剧
const std::string UserBangumiCollection = _apiBase + "/x/space/bangumi/follow/list";
/// 用户订阅合集列表
const std::string UserUGCSeason = _apiBase + "/x/v3/fav/folder/collected/list";
/// 用户订阅合集的视频列表
const std::string UserUGCSeasonVideoList = _apiBase + "/x/space/fav/season/list";
// watch later
const std::string WatchLater = _apiBase + "/x/v2/history/toview/web";

/// ===
/// 搜索页API
/// ===
const std::string Search     = _apiBase + "/x/web-interface/search/type";
const std::string TVSuggest  = _apiBase + "/x/tv/suggest";
const std::string SearchHots = _apiBase + "/x/web-interface/search/square";

/// ===
/// 动态页API
/// ===
const std::string DynamicVideo     = _apiBase + "/x/polymer/web-dynamic/desktop/v1/feed/video";
const std::string DynamicArticle   = _apiBase + "/x/polymer/web-dynamic/desktop/v1/feed/all";
const std::string DynamicUpList    = _vcBase + "/dynamic_svr/v1/dynamic_svr/w_dyn_uplist";
const std::string DynamicUpListV2  = _apiBase + "/x/polymer/web-dynamic/v1/uplist";
const std::string DynamicLike      = _vcBase + "/dynamic_like/v1/dynamic_like/thumb";
const std::string UserDynamicVideo = _apiBase + "/x/space/arc/list";
const std::string DynamicDetail    = _apiBase + "/x/polymer/web-dynamic/desktop/v1/detail";

/// ===
/// 设置页API
/// ===

//使用http协议避免因为系统时间错误导致https请求失败
const std::string UnixTime = "http://api.bilibili.com/x/click-interface/click/now";
}  // namespace Api
}  // namespace bilibili