//
// Created by fang on 2022/5/2.
//

#pragma once

#include "bilibili.h"
#include "bilibili/result/video_detail_result.h"

class VideoDetail {
public:
    virtual void onVideoInfo(const bilibili::VideoDetailResult &result){}
    virtual void onVideoPageListInfo(const bilibili::VideoDetailPageListResult &result){}
    virtual void onVideoPlayUrl(const bilibili::VideoUrlResult & result){}
    virtual void onDanmakuInfo(){}
    virtual void onCommentInfo(const bilibili::VideoCommentResultWrapper& result){}
    virtual void onVideoRecommend(){}
    virtual void onError(){}
    // todo: 获取视频合集

    void requestData(const bilibili::Video& video) {
        this->requestVideoInfo(video.bvid);
        this->requestVideoPageList(video.bvid);
    }

    /// 获取视频信息：标题、作者、简介等
    void requestVideoInfo(const string bvid) {
        brls::Logger::debug("请求视频信息: {}", bvid);
        bilibili::BilibiliClient::get_video_detail(bvid,
                                                   [this](const bilibili::VideoDetailResult &result) {
            brls::Logger::debug("bilibili::BilibiliClient::get_video_detail");
            this->videoDetailResult = result;
            this->requestVideoComment(result.aid);
            this->onVideoInfo(result);
            }, [](const std::string &error) {
                    brls::Logger::error(error);
        });
    }

    /// 获取视频分P列表
    void requestVideoPageList(std::string bvid) {
        brls::Logger::debug("请求视频分P列表: {}", bvid);
        bilibili::BilibiliClient::get_video_pagelist(bvid,
                                                     [this, bvid](const bilibili::VideoDetailPageListResult &result) {
            this->videoDetailPageListResult = result;
            this->onVideoPageListInfo(result);
            for(const auto& i: result){
                brls::Logger::debug("获取视频分P列表: {}", i.cid);
                //todo 记忆上次选择的分辨率
                this->requestVideoUrl(bvid, i.cid);
                break;
            }
            }, [](const std::string &error) {
                    brls::Logger::error(error);
        });
    }

    /// 获取视频地址
    void requestVideoUrl(std::string bvid, int cid){
        brls::Logger::debug("请求视频播放地址: {}/{}", bvid, cid);
        bilibili::BilibiliClient::get_video_url(bvid, cid, 64,
                                        [this](const bilibili::VideoUrlResult & result) {
                                            brls::Logger::debug("bilibili::BilibiliClient::get_video_url");
                                            this->videoUrlResult = result;
                                            this->onVideoPlayUrl(result);
                                        }, [](const std::string &error) {
                    brls::Logger::error(error);
        });
    }

    /// 获取视频评论
    void requestVideoComment(int aid, int next=1, int mode=3){
        bilibili::BilibiliClient::get_comment(aid, next, mode,
                                                [this](const bilibili::VideoCommentResultWrapper& result) {
                                                    this->onCommentInfo(result);
                                                }, [](const std::string &error) {
                    brls::Logger::error(error);
                });
    }
private:
    bilibili::VideoDetailResult videoDetailResult;
    bilibili::VideoDetailPageListResult videoDetailPageListResult;
    bilibili::VideoUrlResult videoUrlResult;
};