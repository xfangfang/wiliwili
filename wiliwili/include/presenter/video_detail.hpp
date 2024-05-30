//
// Created by fang on 2022/5/2.
//

#pragma once

#include <borealis/core/event.hpp>

#include "presenter.h"
#include "bilibili.h"
#include "bilibili/result/video_detail_result.h"
#include "bilibili/result/home_pgc_season_result.h"
#include "presenter/comment_related.hpp"

// 指明一个id的类型
enum class PGC_ID_TYPE {
    SEASON_ID,  // 剧ID
    EP_ID       // 集ID
};

class VideoDetail : public CommentRequest {
public:
    virtual void onVideoInfo(const bilibili::VideoDetailResult& result) {}
    virtual void onSeasonVideoInfo(const bilibili::SeasonResultWrapper& result) {}
    virtual void onSeasonStatus(const bilibili::SeasonStatusResult& result) {}
    virtual void onSeasonEpisodeInfo(const bilibili::SeasonEpisodeResult& result) {}
    virtual void onSeasonSeriesInfo(const bilibili::SeasonSeries& result) {}
    virtual void onSeasonRecommend(const bilibili::SeasonRecommendWrapper& result) {}
    virtual void onVideoPageListInfo(const bilibili::VideoDetailPageListResult& result) {}
    virtual void onVideoPlayUrl(const bilibili::VideoUrlResult& result) {}
    virtual void onCastPlayUrl(const bilibili::VideoUrlResult& result) {}
    virtual void onUploadedVideos(const bilibili::UserUploadedVideoResultWrapper& result) {}
    virtual void onDanmakuInfo() {}
    virtual void onHighlightProgress(const bilibili::VideoHighlightProgress& result) {}
    virtual void onVideoRecommend() {}
    virtual void onError(const std::string& error) {}
    virtual void onVideoOnlineCount(const bilibili::VideoOnlineTotal& result) {}
    virtual void onVideoRelationInfo(const bilibili::VideoRelation& result) {}
    virtual void onRelatedVideoList(const bilibili::VideoDetailListResult& result) {}
    virtual void onUpInfo(const bilibili::UserDetailResultWrapper& result) {}
    virtual void onRedirectToEp(const std::string& url) {}
    virtual void onUGCSeasonInfo(const bilibili::UGCSeason& result) {}

    /// 请求视频数据
    void requestData(const bilibili::VideoDetailResult& video);

    /// 请求番剧数据
    void requestData(uint64_t id, PGC_ID_TYPE type = PGC_ID_TYPE::SEASON_ID);

    /// 获取番剧信息
    void requestSeasonInfo(uint64_t seasonID, uint64_t epID = 0);

    /// 获取番剧相关推荐
    void requestSeasonRecommend(uint64_t seasonID);

    /// 获取番剧播放进度，追剧情况
    void requestSeasonStatue(uint64_t seasonID);

    /// 获取视频信息：标题、作者、简介、分P等
    void requestVideoInfo(const std::string& bvid);

    /**
     * 获取视频地址
     * @param requestHistoryInfo 是否获取历史播放进度
     */
    void requestVideoUrl(const std::string& bvid, uint64_t cid, bool requestHistoryInfo = true);

    /**
     * 获取番剧地址
     * @param requestHistoryInfo 是否获取历史播放进度
     */
    void requestSeasonVideoUrl(const std::string& bvid, uint64_t cid, bool requestHistoryInfo = true);

    /// 获取投屏地址
    void requestCastVideoUrl(uint64_t oid, uint64_t cid, int type);

    int getQualityIndex();

    /// 切换番剧分集
    void changeEpisode(const bilibili::SeasonEpisodeResult& i);

    /// 获取Up主的其他视频: pn 为0 自动获取下一页
    void requestUploadedVideos(uint64_t mid, int pn = 0, int ps = 10);

    /// 获取单个视频播放人数
    void requestVideoOnline(const std::string& bvid, uint64_t cid);

    /// 获取视频的 点赞、投币、收藏情况
    void requestVideoRelationInfo(const std::string& bvid);

    /// 获取番剧分集的 点赞、投币、收藏情况
    void requestVideoRelationInfo(uint64_t epid);

    /// 获取视频弹幕
    void requestVideoDanmaku(uint64_t cid);

    /// 获取视频高能进度条
    void requestHighlightProgress(uint64_t cid);

    /// 获取视频分P详情
    void requestVideoPageDetail(const std::string& bvid, uint64_t cid, bool requestHistoryInfo = true);

    /// 上报播放进度
    void reportHistory(uint64_t aid, uint64_t cid, unsigned int progress = 0, unsigned int duration = 0,
                       int type = 3);
    inline static bool REPORT_HISTORY = true;

    /// 视频可以投币的数量
    int getCoinTolerate();

    /// 投币
    void addCoin(uint64_t aid, int num, bool like);

    /// 点赞
    void beAgree(uint64_t aid);

    /**
     * 收藏视频
     * @param rid 视频id，aid或epid
     * @param type 视频类型，2 / 24；普通视频/番剧
     * @param add 添加的收藏夹
     * @param del 移除的收藏夹
     * @param isFavorite 在添加或移除过后是否处于收藏状态
     */
    void addResource(uint64_t rid, int type = 2, bool isFavorite = true, std::string add = "1", std::string del = "");

    void followUp(const std::string& mid, bool follow);

    void followSeason(uint64_t season, bool follow);

    static inline int defaultQuality = 116;

protected:
    bilibili::VideoDetailResult videoDetailResult;       //  视频数据
    bilibili::VideoDetailPage videoDetailPage;           // 视频分P数据
    bilibili::VideoDetailListResult videDetailRelated;   // 推荐视频
    bilibili::UserDetailResultWrapper userDetailResult;  // 作者数据
    bilibili::SeasonStatusResult seasonStatus;           // 番剧关注状态
    bilibili::VideoUrlResult videoUrlResult;
    bilibili::SeasonResultWrapper seasonInfo;     // 番剧/综艺/影视 数据
    bilibili::SeasonEpisodeResult episodeResult;  // 番剧/综艺/影视 单集数据
    bilibili::VideoRelation videoRelation;        // 视频点赞投币收藏情况

    // 番剧/综艺/影视 剧集列表（包括非正片）
    bilibili::SeasonEpisodeListResult episodeList;

    unsigned int userUploadedVideoRequestIndex = 1;

    // 触发此事件，传入 SeasonEpisodeResult， 会播放对应epid的内容
    brls::Event<bilibili::SeasonEpisodeResult> changeEpisodeEvent;
};