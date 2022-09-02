//
// Created by fang on 2022/5/2.
//

#pragma once

#include "presenter.h"
#include "bilibili.h"
#include "bilibili/result/video_detail_result.h"

class VideoDetail: public Presenter{
public:
    virtual void onVideoInfo(const bilibili::VideoDetailResult &result){}
    virtual void onSeasonVideoInfo(const bilibili::SeasonResultWrapper& result){}
    virtual void onSeasonEpisodeInfo(const bilibili::SeasonEpisodeResult& result){}
    virtual void onVideoPageListInfo(const bilibili::VideoDetailPageListResult& result){}
    virtual void onVideoPlayUrl(const bilibili::VideoUrlResult& result){}
    virtual void onUploadedVideos(const bilibili::UserUploadedVideoResultWrapper& result){}
    virtual void onDanmakuInfo(){}
    virtual void onCommentInfo(const bilibili::VideoCommentResultWrapper& result){}
    virtual void onRequestCommentError(const std::string &error){}
    virtual void onVideoRecommend(){}
    virtual void onError(const std::string &error){}
    virtual void onVideoOnlineCount(const bilibili::VideoOnlineTotal& result){}
    virtual void onVideoRelationInfo(const bilibili::VideoRelation& result){}
    virtual void onRelatedVideoList(const bilibili::VideoDetailListResult& result){}
    virtual void onDanmaku(const std::string& filePath){}


    // todo: 获取视频合集

    /// 请求视频数据
    void requestData(const bilibili::VideoDetailResult& video);

    /// 请求番剧数据
    void requestData(int seasonID);

    /// 获取番剧信息
    void requestSeasonInfo(const int seasonID);

    /// 获取视频信息：标题、作者、简介、分P等
    void requestVideoInfo(const string bvid);

    /// 获取视频地址
    void requestVideoUrl(std::string bvid, int cid);

    /// 获取番剧地址
    void requestSeasonVideoUrl(const std::string& bvid, int cid);

    /// 切换番剧分集
    void changeEpisode(const bilibili::SeasonEpisodeResult& i);

    /// 获取视频评论: next 为0 自动获取下一页
    void requestVideoComment(int aid, int next = 0, int mode = 3);

    /// 获取Up主的其他视频: pn 为0 自动获取下一页
    void requestUploadedVideos(int mid, int pn = 0, int ps = 5);

    /// 获取单个视频播放人数
    void requestVideoOnline(const std::string& bvid, int cid);

    /// 获取视频的 点赞、投币、收藏情况
    void requestVideoRelationInfo(const std::string& bvid);

    /// 获取视频弹幕
    void requestVideoDanmaku(const uint cid);

protected:
    bilibili::VideoDetailResult videoDetailResult; //  视频数据
    bilibili::UserDetailResultWrapper userDetailResult; // 作者数据
    bilibili::VideoUrlResult videoUrlResult;
    bilibili::SeasonResultWrapper seasonInfo; // 番剧/综艺/影视 数据
    bilibili::SeasonEpisodeResult episodeResult; // 番剧/综艺/影视 单集数据

    uint commentRequestIndex = 1;
    uint userUploadedVideoRequestIndex = 1;
};