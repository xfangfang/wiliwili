//
// Created by fang on 2022/5/1.
//

#pragma once

using namespace std;

namespace bilibili {

    namespace Api {

        static const string _apiBase = "https://api.bilibili.com";
        static const string _appBase = "https://app.bilibili.com";
        static const string _vcBase = "https://api.vc.bilibili.com";
        static const string _liveBase = "https://api.live.bilibili.com";
        static const string _passBase = "https://passport.bilibili.com";
        static const string _bangumiBase = "https://bangumi.bilibili.com";
        static const string _grpcBase = "https://grpc.biliapi.net";

        /// ===
        /// 视频API
        /// ===

        /// 视频详情. gRPC
        static const string DetailGRPC = _grpcBase + "/bilibili.app.view.v1.View/View";
        /// 视频详情.
        static const string Detail = _apiBase + "/x/web-interface/view";
        /// 在线观看人数.
        static const string OnlineViewerCount = _appBase + "/x/v2/view/video/online";
        /// 视频播放信息.
        static const string PlayInformation = _apiBase + "/x/player/playurl";
        /// 视频播放地址 TV.
        static const string PlayUrlTV = _apiBase + "/x/tv/card/view_v2";
        /// 视频分P列表.
        static const string PlayPageList = _apiBase + "/x/player/pagelist";
        /// 视频播放信息.
        static const string PlayConfig = _appBase + "/bilibili.app.playurl.v1.PlayURL/PlayConf";
        /// 弹幕元数据.
        static const string DanmakuMetaData = _grpcBase + "/bilibili.community.service.dm.v1.DM/DmView";
        /// 分段弹幕.
        static const string SegmentDanmaku = _grpcBase + "/bilibili.community.service.dm.v1.DM/DmSegMobile";
        /// 历史记录.
        static const string ProgressReport = _apiBase + "/x/v2/history/report";
        /// 点赞视频.
        static const string Like = _appBase + "/x/v2/view/like";
        /// 给视频投币.
        static const string Coin = _appBase + "/x/v2/view/coin/add";
        /// 添加或删除视频收藏.
        static const string ModifyFavorite = _apiBase + "/x/v3/fav/resource/deal";
        /// 一键三连.
        static const string Triple = _appBase + "/x/v2/view/like/triple";
        /// 发送弹幕.
        static const string SendDanmaku = _apiBase + "/x/v2/dm/post";
        /// 获取视频字幕.
        static const string Subtitle = _apiBase + "/x/player.so";
        /// 获取互动视频选项.
        static const string InteractionEdge = _apiBase + "/x/stein/edgeinfo_v2";
        /// 获取视频参数.
        static const string Stat = _apiBase + "/x/web-interface/archive/stat";

        /// ===
        /// 主页API
        /// ===

        /// 主页 推荐
        static const string Recommend = _apiBase + "/x/web-interface/index/top/feed/rcmd";
        /// 主页 热门 热门综合
        static const string HotsAll = _apiBase + "/x/web-interface/popular";
        /// 主页 热门 每周推荐列表
        static const string HotsWeeklyList = _apiBase + "/x/web-interface/popular/series/list";
        /// 主页 热门 每周推荐
        static const string HotsWeekly = _apiBase + "/x/web-interface/popular/series/one";
        /// 主页 热门 入站必刷
        static const string HotsHistory = _apiBase + "/x/web-interface/popular/precious";
        /// 主页 热门 用户投稿排行榜
        static const string HotsRank= _apiBase + "/x/web-interface/ranking/v2";
        /// 主页 热门 官方视频(番剧 电影...)排行榜
        static const string HotsRankPGC = _apiBase + "/pgc/season/rank/web/list";
        /// 主页 直播推荐
        static const string LiveFeed = _liveBase + "/xlive/app-interface/v2/index/feedV2";
        /// 主页 追番
        static const string Bangumi = _apiBase + "/pgc/page/pc/bangumi/tab";
        /// 主页 影视
        static const string Cinema = _apiBase + "/pgc/page/pc/cinema/tab";


        /// 视频 评论
        static const string Comment = _apiBase + "/x/v2/reply/main";

        /// 热门 - gRPC.
        static const string PopularGRPC = _grpcBase + "/bilibili.app.show.v1.Popular/Index";
        /// 排行榜 - Web.
        static const string Ranking = _apiBase + "/x/web-interface/ranking/v2";
        /// 排行榜 - gRPC.
        static const string RankingGRPC = _grpcBase + "/bilibili.app.show.v1.Rank/RankRegion";


        /// ===
        /// 个人页API
        /// ===
        static const string QrLoginUrl = _passBase + "/qrcode/getLoginUrl";
        static const string QrLoginInfo = _passBase + "/qrcode/getLoginInfo";
        static const string MyInfo = _apiBase + "/x/space/myinfo";
        static const string HistoryVideo = _apiBase + "/x/web-interface/history/cursor";
        static const string CollectionList = _apiBase + "/x/v3/fav/folder/created/list";
        static const string CollectionVideoList = _apiBase + "/x/v3/fav/resource/list";
        static const string UserUploadedVideo = _apiBase + "/x/space/arc/search";


        /// ===
        /// 搜索页API
        /// ===
        static const string Search = _apiBase + "/x/web-interface/search/type";

    }
}