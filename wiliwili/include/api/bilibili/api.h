//
// Created by fang on 2022/5/1.
//

#pragma once

namespace bilibili {

namespace Api {

static const std::string _apiBase     = "https://api.bilibili.com";
static const std::string _appBase     = "https://app.bilibili.com";
static const std::string _vcBase      = "https://api.vc.bilibili.com";
static const std::string _liveBase    = "https://api.live.bilibili.com";
static const std::string _passBase    = "https://passport.bilibili.com";
static const std::string _bangumiBase = "https://bangumi.bilibili.com";
static const std::string _grpcBase    = "https://grpc.biliapi.net";

/// ===
/// 视频API
/// ===

/// 视频详情. gRPC
static const std::string DetailGRPC =
    _grpcBase + "/bilibili.app.view.v1.View/View";
/// 视频详情.
static const std::string Detail    = _apiBase + "/x/web-interface/view";
static const std::string DetailAll = _apiBase + "/x/web-interface/view/detail";
/// 番剧详情.
static const std::string SeasonDetail = _apiBase + "/pgc/view/pc/season";
/// 番剧推荐.
static const std::string SeasonRCMD =
    _apiBase + "/pgc/season/web/related/recommend";
/// 番剧播放进度
static const std::string SeasonStatus =
    _apiBase + "/pgc/view/web/season/user/status";
/// 在线观看人数.
static const std::string OnlineViewerCount =
    _apiBase + "/x/player/online/total";
/// 视频播放信息.
static const std::string PlayInformation = _apiBase + "/x/player/playurl";
/// 视频播放地址 TV.
static const std::string PlayUrlTV = _apiBase + "/x/tv/card/view_v2";
/// 番剧播放地址
static const std::string SeasonUrl = _apiBase + "/pgc/player/web/playurl";
/// 视频分P列表.
static const std::string PlayPageList = _apiBase + "/x/player/pagelist";
/// 视频播放信息.
static const std::string PlayConfig =
    _appBase + "/bilibili.app.playurl.v1.PlayURL/PlayConf";
/// 弹幕元数据.
static const std::string DanmakuMetaData =
    _grpcBase + "/bilibili.community.service.dm.v1.DM/DmView";
/// 分段弹幕.
static const std::string SegmentDanmaku =
    _grpcBase + "/bilibili.community.service.dm.v1.DM/DmSegMobile";
/// 历史记录.
static const std::string ProgressReport = _apiBase + "/x/v2/history/report";
/// 点赞视频.
static const std::string Like    = _appBase + "/x/v2/view/like";
static const std::string LikeWeb = _apiBase + "/x/web-interface/archive/like";
/// 给视频投币.
static const std::string Coin    = _appBase + "/x/v2/view/coin/add";
static const std::string CoinWeb = _apiBase + "/x/web-interface/coin/add";
/// 投币经验值
static const std::string CoinExp = _apiBase + "/x/web-interface/coin/today/exp";
/// 添加或删除视频收藏.
static const std::string ModifyFavorite = _apiBase + "/x/v3/fav/resource/deal";
/// 一键三连.
static const std::string Triple = _appBase + "/x/v2/view/like/triple";
static const std::string TripleWeb =
    _apiBase + "x/web-interface/archive/like/triple";
/// 关注 取关
static const std::string Follow = _apiBase + "/x/relation/modify";
/// 番剧 追剧/取消追剧
static const std::string FollowSeason     = _apiBase + "/pgc/app/follow/add";
static const std::string UndoFollowSeason = _apiBase + "/pgc/app/follow/del";
/// 发送弹幕.
static const std::string SendDanmaku = _apiBase + "/x/v2/dm/post";
/// 获取视频字幕.
static const std::string Subtitle = _apiBase + "/x/player.so";
/// 获取互动视频选项.
static const std::string InteractionEdge = _apiBase + "/x/stein/edgeinfo_v2";
/// 获取视频参数.
static const std::string Stat = _apiBase + "/x/web-interface/archive/stat";
/// 获取视频点赞收藏情况
static const std::string VideoRelation =
    _apiBase + "/x/web-interface/archive/relation";
/// 获取番剧视频点赞收藏情况
static const std::string VideoEpisodeRelation =
    _apiBase + "/pgc/season/episode/web/info";
/// 获取视频弹幕
static const std::string VideoDanmaku = _apiBase + "/x/v1/dm/list.so";
/// 直播API
static const std::string LiveUrl = _liveBase + "/room/v1/Room/playUrl";
/// 直播历史记录
static const std::string LiveReport =
    _liveBase + "/xlive/web-room/v1/index/roomEntryAction";

/// ===
/// 主页API
/// ===

/// 主页 推荐
static const std::string Recommend =
    _apiBase + "/x/web-interface/index/top/feed/rcmd";
/// 主页 热门 热门综合
static const std::string HotsAll = _apiBase + "/x/web-interface/popular";
/// 主页 热门 每周推荐列表
static const std::string HotsWeeklyList =
    _apiBase + "/x/web-interface/popular/series/list";
/// 主页 热门 每周推荐
static const std::string HotsWeekly =
    _apiBase + "/x/web-interface/popular/series/one";
/// 主页 热门 入站必刷
static const std::string HotsHistory =
    _apiBase + "/x/web-interface/popular/precious";
/// 主页 热门 用户投稿排行榜
static const std::string HotsRank = _apiBase + "/x/web-interface/ranking/v2";
/// 主页 热门 官方视频(番剧 电影...)排行榜
static const std::string HotsRankPGC = _apiBase + "/pgc/season/rank/web/list";
/// 主页 直播推荐
static const std::string LiveFeed =
    _liveBase + "/xlive/app-interface/v2/index/feedV2";
/// 主页 追番
static const std::string Bangumi = _apiBase + "/pgc/page/pc/bangumi/tab";
/// 主页 影视
static const std::string Cinema = _apiBase + "/pgc/page/pc/cinema/tab";
/// 主页 追番/影视 分类检索
static const std::string PGCIndex = _apiBase + "/pgc/page/index/result";
/// 主页 追番/影视 分类检索过滤器列表
static const std::string PGCIndexFilter =
    _apiBase + "/pgc/page/index/condition";
/// 视频 评论
static const std::string Comment = _apiBase + "/x/v2/reply/main";
/// 单条评论详情
static const std::string CommentDetail = _apiBase + "/x/v2/reply/detail";
/// 点赞评论
static const std::string CommentLike = _apiBase + "/x/v2/reply/action";
/// 发布评论
static const std::string CommentAdd = _apiBase + "/x/v2/reply/add";
/// 删除评论
static const std::string CommentDel = _apiBase + "/x/v2/reply/del";
/// 热门 - gRPC.
static const std::string PopularGRPC =
    _grpcBase + "/bilibili.app.show.v1.Popular/Index";
/// 排行榜 - Web.
static const std::string Ranking = _apiBase + "/x/web-interface/ranking/v2";
/// 排行榜 - gRPC.
static const std::string RankingGRPC =
    _grpcBase + "/bilibili.app.show.v1.Rank/RankRegion";

/// ===
/// 个人页API
/// ===
static const std::string QrLoginUrl  = _passBase + "/qrcode/getLoginUrl";
static const std::string QrLoginInfo = _passBase + "/qrcode/getLoginInfo";
static const std::string MyInfo      = _apiBase + "/x/space/myinfo";
static const std::string HistoryVideo =
    _apiBase + "/x/web-interface/history/cursor";
static const std::string CollectionList =
    _apiBase + "/x/v3/fav/folder/created/list";
static const std::string CollectionListAll =
    _apiBase + "/x/v3/fav/folder/created/list-all";
static const std::string CollectionVideoList =
    _apiBase + "/x/v3/fav/resource/list";
static const std::string CollectionVideoListSave =
    _apiBase + "/x/v3/fav/resource/deal";
static const std::string UserUploadedVideo = _apiBase + "/x/space/arc/search";
static const std::string UserRelationStat  = _apiBase + "/x/relation/stat";
static const std::string UserDynamicStat =
    _vcBase + "/dynamic_svr/v1/dynamic_svr/space_num_ex";

/// ===
/// 搜索页API
/// ===
static const std::string Search = _apiBase + "/x/web-interface/search/type";
static const std::string SearchHots =
    _apiBase + "/x/web-interface/search/square";

/// ===
/// 动态页API
/// ===
static const std::string DynamicVideo =
    _apiBase + "/x/polymer/web-dynamic/desktop/v1/feed/video";
static const std::string DynamicUpList =
    _vcBase + "/dynamic_svr/v1/dynamic_svr/w_dyn_uplist";
static const std::string UserDynamicVideo = _apiBase + "/x/space/arc/list";

/// ===
/// 设置页API
/// ===

//使用http协议避免因为系统时间错误导致https请求失败
static const std::string UnixTime =
    "http://api.bilibili.com/x/click-interface/click/now";
}  // namespace Api
}  // namespace bilibili