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
    virtual void onCommentInfo(){}
    virtual void onVideoRecommend(){}
    virtual void onError(){}
    // todo: 获取视频合集

    void requestData(const bilibili::Video& video) {
        this->requestVideoInfo(video);
        this->requestVideoPageList(video);
    }

    /// 获取视频信息：标题、作者、简介等
    void requestVideoInfo(const bilibili::Video& video) {
        bilibili::BilibiliClient::get_video_detail(video.bvid,
                                                   [this](const bilibili::VideoDetailResult &result) {
            Logger::debug("bilibili::BilibiliClient::get_video_detail");
            this->videoDetailResult = result;
            this->onVideoInfo(result);
            }, [](const std::string &error) {
            Logger::error(error);
        });
    }

    /// 获取视频分P列表
    void requestVideoPageList(const bilibili::Video& video) {
        bilibili::BilibiliClient::get_video_pagelist(video.bvid,
                                                     [this, video](const bilibili::VideoDetailPageListResult &result) {
            this->videoDetailPageListResult = result;
            this->onVideoPageListInfo(result);
            for(const auto& i: result){
                //todo 记忆上次选择的分辨率
                this->requestVideoUrl(video, i.cid);
                break;
            }
            }, [](const std::string &error) {
            Logger::error(error);
        });
    }

    /// 获取视频地址
    void requestVideoUrl(const bilibili::Video& video, int cid){
        bilibili::BilibiliClient::get_video_url(video.bvid, cid, 64,
                                        [this](const bilibili::VideoUrlResult & result) {
                                            Logger::debug("bilibili::BilibiliClient::get_video_url");
                                            this->videoUrlResult = result;
                                            this->onVideoPlayUrl(result);
                                        }, [](const std::string &error) {
            Logger::error(error);
        });
    }
private:
    bilibili::VideoDetailResult videoDetailResult;
    bilibili::VideoDetailPageListResult videoDetailPageListResult;
    bilibili::VideoUrlResult videoUrlResult;
};