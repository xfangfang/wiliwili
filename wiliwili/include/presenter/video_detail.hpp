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
    virtual void onVideoRecommend(){}
    virtual void onError(){}
    // todo: 获取视频合集

    /// 请求视频数据
    void requestData(const bilibili::VideoDetailResult& video) {
        this->requestVideoInfo(video.bvid);
    }

    /// 请求番剧数据
    void requestData(int seasonID) {
        this->requestSeasonInfo(seasonID);
    }

    /// 获取番剧信息
    void requestSeasonInfo(const int seasonID){
        ASYNC_RETAIN
        bilibili::BilibiliClient::get_season_detail(seasonID, [ASYNC_TOKEN](const bilibili::SeasonResultWrapper& result){
            brls::sync([ASYNC_TOKEN, result](){
                ASYNC_RELEASE
                brls::Logger::debug("bilibili::BilibiliClient::get_season_detail");
                seasonInfo = result;
                this->onSeasonVideoInfo(result);
                for (auto i: result.episodes) {
                    brls::Logger::debug("P{} {} cid: {}", i.title, i.long_title, i.cid);
                    this->changeEpisode(i);
                    break;
                }
            });
        }, [](const std::string &error) {
            brls::Logger::error(error);
        });
    }

    /// 获取视频信息：标题、作者、简介、分P等
    void requestVideoInfo(const string bvid) {
        ASYNC_RETAIN
        brls::Logger::debug("请求视频信息: {}", bvid);
        bilibili::BilibiliClient::get_video_detail(bvid,
                                                   [ASYNC_TOKEN](const bilibili::VideoDetailResult &result) {
                                                       brls::sync([ASYNC_TOKEN, result](){
                                                           ASYNC_RELEASE
                                                           brls::Logger::debug("bilibili::BilibiliClient::get_video_detail");
                                                           this->videoDetailResult = result;

                                                           // 请求视频评论
                                                           this->requestVideoComment(result.aid);

                                                           // 请求用户投稿列表 todo: 加载多页
                                                           this->requestUploadedVideos(result.owner.mid, 1);

                                                           //  请求视频播放地址
                                                           this->onVideoPageListInfo(result.pages);
                                                           for(const auto& i: result.pages){
                                                               brls::Logger::debug("获取视频分P列表: PV1 {}", i.cid);
                                                               // 播放PV1
                                                               this->requestVideoUrl(result.bvid, i.cid);
                                                               break;
                                                           }

                                                           this->onVideoInfo(result);
                                                       });

            }, [](const std::string &error) {
                    brls::Logger::error(error);
        });
    }

    /// 获取视频地址
    void requestVideoUrl(std::string bvid, int cid){
        ASYNC_RETAIN
        brls::Logger::debug("请求视频播放地址: {}/{}", bvid, cid);
        bilibili::BilibiliClient::get_video_url(bvid, cid, 64,
                                        [ASYNC_TOKEN](const bilibili::VideoUrlResult & result) {
                                            brls::sync([ASYNC_TOKEN, result](){
                                                ASYNC_RELEASE
                                                brls::Logger::debug("bilibili::BilibiliClient::get_video_url");
                                                this->videoUrlResult = result;
                                                this->onVideoPlayUrl(result);
                                            });
                                        }, [](const std::string &error) {
                    brls::Logger::error(error);
        });
    }

    /// 获取番剧地址
    void requestSeasonVideoUrl(int cid){
        ASYNC_RETAIN
        brls::Logger::debug("请求番剧视频播放地址: {}", cid);
        bilibili::BilibiliClient::get_season_url(cid, 64,
                                                [ASYNC_TOKEN](const bilibili::VideoUrlResult & result) {
                                                    brls::sync([ASYNC_TOKEN, result](){
                                                        ASYNC_RELEASE
                                                        brls::Logger::debug("bilibili::BilibiliClient::get_video_url");
                                                        this->videoUrlResult = result;
                                                        this->onVideoPlayUrl(result);
                                                    });
                                                }, [](const std::string &error) {
                    brls::Logger::error(error);
                });
    }

    /// 切换番剧分集
    void changeEpisode(const bilibili::SeasonEpisodeResult& i){
        this->onSeasonEpisodeInfo(i);
        this->requestVideoComment(i.aid);
        this->requestSeasonVideoUrl(i.cid);
    }

    /// 获取视频评论
    void requestVideoComment(int aid, int next=1, int mode=3){
        ASYNC_RETAIN
        bilibili::BilibiliClient::get_comment(aid, next, mode,
                                                [ASYNC_TOKEN](const bilibili::VideoCommentResultWrapper& result) {
            brls::sync([ASYNC_TOKEN, result](){
                ASYNC_RELEASE
                this->onCommentInfo(result);
            });
        }, [](const std::string &error) {
            brls::Logger::error(error);
        });
    }

    /// 获取Up主的其他视频
    void requestUploadedVideos(int mid, int pn = 1, int ps = 5){
        brls::Logger::debug("请求投稿视频: {}", mid);
        ASYNC_RETAIN
        bilibili::BilibiliClient::get_user_videos(mid, pn, ps, [ASYNC_TOKEN](const bilibili::UserUploadedVideoResultWrapper& result){
            brls::sync([ASYNC_TOKEN, result](){
                ASYNC_RELEASE
                this->userUploadedVideo = result;
                this->onUploadedVideos(result);
            });
        }, [](const std::string &error) {
            brls::Logger::error(error);
        });
    }

protected:
    bilibili::VideoDetailResult videoDetailResult;
    bilibili::VideoUrlResult videoUrlResult;
    bilibili::UserUploadedVideoResultWrapper userUploadedVideo;
    bilibili::SeasonResultWrapper seasonInfo; // 番剧数据
};