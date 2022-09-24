//
// Created by fang on 2022/8/9.
//
#include <tinyxml2.h>
#include "pystring.h"
#include "borealis.hpp"
#include "presenter/video_detail.hpp"
#include "utils/config_helper.hpp"
#include "view/mpv_core.hpp"

/// 请求视频数据
void VideoDetail::requestData(const bilibili::VideoDetailResult& video) {
    this->requestVideoInfo(video.bvid);
}

/// 请求番剧数据
void VideoDetail::requestData(int id, PGC_ID_TYPE type) {
    if (type == PGC_ID_TYPE::SEASON_ID)
        this->requestSeasonInfo(id, 0);
    else if (type == PGC_ID_TYPE::EP_ID)
        this->requestSeasonInfo(0, id);
}

/// 获取番剧信息
void VideoDetail::requestSeasonInfo(const int seasonID, const int epID) {
    // 重置MPV
    MPVCore::instance().reset();

    ASYNC_RETAIN
    bilibili::BilibiliClient::get_season_detail(
        seasonID, epID,
        [ASYNC_TOKEN, epID](const bilibili::SeasonResultWrapper& result) {
            brls::sync([ASYNC_TOKEN, result, epID]() {
                ASYNC_RELEASE
                brls::Logger::debug(
                    "bilibili::BilibiliClient::get_season_detail");
                seasonInfo = result;
                this->onSeasonVideoInfo(result);

                // Try load episode: epID
                // todo: 番剧中除了正片之外的视频也可能被调用
                if (epID != 0) {
                    for (auto i : result.episodes) {
                        if (i.id == (unsigned int)epID) {
                            brls::Logger::debug("P{} {} epid: {}", i.title,
                                                i.long_title, i.id);
                            int progress  = episodeResult.progress;
                            episodeResult = i;
                            // 用于从历史记录加载数据
                            episodeResult.progress = progress;
                            this->changeEpisode(episodeResult);
                            return;
                        }
                    }
                }

                // Default: load first episode
                // todo: load from history
                for (auto i : result.episodes) {
                    brls::Logger::debug("P{} {} epid: {}", i.title,
                                        i.long_title, i.id);
                    this->changeEpisode(i);
                    break;
                }
            });
        },
        [ASYNC_TOKEN](const std::string& error) {
            ASYNC_RELEASE
            brls::Logger::error(error);
        });
}

/// 获取视频信息：标题、作者、简介、分P等
void VideoDetail::requestVideoInfo(const std::string bvid) {
    // 重置MPV
    MPVCore::instance().reset();
    // 请求视频点赞情况
    this->requestVideoRelationInfo(bvid);

    ASYNC_RETAIN
    brls::Logger::debug("请求视频信息: {}", bvid);
    bilibili::BilibiliClient::get_video_detail_all(
        bvid,
        [ASYNC_TOKEN](const bilibili::VideoDetailAllResult& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                brls::Logger::debug(
                    "bilibili::BilibiliClient::get_video_detail");
                this->videoDetailResult = result.View;
                this->userDetailResult  = result.Card;

                // 展示分P数据
                this->onVideoPageListInfo(this->videoDetailResult.pages);

                // 尝试打开指定的分P
                if (videoDetailPage.cid != 0)
                    for (const auto& i : this->videoDetailResult.pages) {
                        if (i.cid == videoDetailPage.cid) {
                            brls::Logger::debug("获取视频分P列表: PV {}",
                                                i.cid);
                            int progress    = videoDetailPage.progress;
                            videoDetailPage = i;
                            //用于从历史记录加载进播放页面，视频开始播放时自动跳转
                            videoDetailPage.progress = progress;

                            //上报历史记录
                            if (progress < 0) progress = 0;
                            this->reportHistory(this->videoDetailResult.aid,
                                                videoDetailPage.cid, progress);
                            break;
                        }
                    }

                // 默认打开PV1，没有找到指定cid的PV也打开PV1
                if (videoDetailPage.cid == 0)
                    for (const auto& i : this->videoDetailResult.pages) {
                        brls::Logger::debug("获取视频分P列表: PV1 {}", i.cid);
                        videoDetailPage = i;
                        //上报历史记录
                        this->reportHistory(this->videoDetailResult.aid,
                                            videoDetailPage.cid, 0);
                        break;
                    }

                // 请求视频播放地址
                this->requestVideoUrl(this->videoDetailResult.bvid,
                                      this->videoDetailPage.cid);

                // 请求视频评论
                this->requestVideoComment(this->videoDetailResult.aid, 1);

                // 请求用户投稿列表
                this->requestUploadedVideos(videoDetailResult.owner.mid, 1);

                this->onVideoInfo(this->videoDetailResult);
                this->onRelatedVideoList(result.Related);
            });
        },
        [ASYNC_TOKEN](const std::string& error) {
            ASYNC_RELEASE
            brls::Logger::error("ERROR:请求视频信息 {}", error);
            this->onError(error);
        });
}

/// 获取视频地址
void VideoDetail::requestVideoUrl(std::string bvid, int cid) {
    // 重置MPV
    MPVCore::instance().reset();

    ASYNC_RETAIN
    brls::Logger::debug("请求视频播放地址: {}/{}", bvid, cid);
    bilibili::BilibiliClient::get_video_url(
        bvid, cid, 116,
        [ASYNC_TOKEN](const bilibili::VideoUrlResult& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                brls::Logger::debug(
                    "bilibili::BilibiliClient::get_video_url : {}",
                    result.quality);
                this->videoUrlResult = result;
                this->onVideoPlayUrl(result);
            });
        },
        [ASYNC_TOKEN](const std::string& error) {
            ASYNC_RELEASE
            brls::Logger::error(error);
        });

    // 请求当前视频在线人数
    this->requestVideoOnline(bvid, cid);
    // 请求弹幕
    this->requestVideoDanmaku(cid);
}

/// 获取番剧地址
void VideoDetail::requestSeasonVideoUrl(const std::string& bvid, int cid) {
    // 重置MPV
    MPVCore::instance().reset();
    // 请求当前视频在线人数
    this->requestVideoOnline(bvid, cid);
    // 请求弹幕
    this->requestVideoDanmaku(cid);

    ASYNC_RETAIN
    brls::Logger::debug("请求番剧视频播放地址: {}", cid);
    bilibili::BilibiliClient::get_season_url(
        cid, 116,
        [ASYNC_TOKEN](const bilibili::VideoUrlResult& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                brls::Logger::debug("bilibili::BilibiliClient::get_video_url");
                this->videoUrlResult = result;
                this->onVideoPlayUrl(result);
            });
        },
        [ASYNC_TOKEN](const std::string& error) {
            ASYNC_RELEASE
            brls::Logger::error(error);
        });
}

/// 切换番剧分集
void VideoDetail::changeEpisode(const bilibili::SeasonEpisodeResult& i) {
    episodeResult = i;

    //上报历史记录
    int progress = 0;
    if (episodeResult.progress > 0) progress = episodeResult.progress;

    this->reportHistory(i.aid, i.cid, progress, 4);

    // 重置MPV
    MPVCore::instance().reset();

    this->onSeasonEpisodeInfo(i);
    this->requestVideoComment(i.aid, 1);
    this->requestSeasonVideoUrl(i.bvid, i.cid);
}

/// 获取视频评论
void VideoDetail::requestVideoComment(int aid, int next, int mode) {
    if (next != 0) {
        this->commentRequestIndex = next;
    }
    brls::Logger::debug("请求视频评论: {}", aid);
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_comment(
        aid, commentRequestIndex, mode,
        [ASYNC_TOKEN](const bilibili::VideoCommentResultWrapper& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                if (this->commentRequestIndex != result.cursor.prev) return;
                if (!result.cursor.is_end) {
                    this->commentRequestIndex = result.cursor.next;
                }
                this->onCommentInfo(result);
            });
        },
        [ASYNC_TOKEN](const std::string& error) {
            ASYNC_RELEASE
            this->onRequestCommentError(error);
        });
}

/// 获取Up主的其他视频
void VideoDetail::requestUploadedVideos(int mid, int pn, int ps) {
    if (pn != 0) {
        this->userUploadedVideoRequestIndex = pn;
    }
    brls::Logger::debug("请求投稿视频: {}/{}", mid,
                        userUploadedVideoRequestIndex);
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_user_videos(
        mid, userUploadedVideoRequestIndex, ps,
        [ASYNC_TOKEN](const bilibili::UserUploadedVideoResultWrapper& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                if (result.page.pn != this->userUploadedVideoRequestIndex)
                    return;
                if (!result.list.empty()) {
                    this->userUploadedVideoRequestIndex = result.page.pn + 1;
                }
                this->onUploadedVideos(result);
            });
        },
        [ASYNC_TOKEN](const std::string& error) {
            ASYNC_RELEASE
            brls::Logger::error(error);
        });
}

/// 获取单个视频播放人数
void VideoDetail::requestVideoOnline(const std::string& bvid, int cid) {
    brls::Logger::debug("请求当前视频在线人数: bvid: {} cid: {}", bvid, cid);
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_video_online(
        bvid, cid,
        [ASYNC_TOKEN](const bilibili::VideoOnlineTotal& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                this->onVideoOnlineCount(result);
            });
        },
        [ASYNC_TOKEN](const std::string& error) {
            ASYNC_RELEASE
            brls::Logger::error(error);
        });
}

/// 获取视频的 点赞、投币、收藏情况
void VideoDetail::requestVideoRelationInfo(const std::string& bvid) {
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_video_relation(
        bvid,
        [ASYNC_TOKEN](const bilibili::VideoRelation& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                this->onVideoRelationInfo(result);
            });
        },
        [ASYNC_TOKEN](const std::string& error) {
            ASYNC_RELEASE
            brls::Logger::error(error);
        });
}

/// 获取视频弹幕
void VideoDetail::requestVideoDanmaku(const unsigned int cid) {
    brls::Logger::debug("请求弹幕：cid: {}", cid);
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_danmaku(
        cid,
        [ASYNC_TOKEN](const std::string& result) {
            ASYNC_RELEASE
            brls::Logger::debug("DANMAKU: start decode");

            // Load XML
            tinyxml2::XMLDocument document = tinyxml2::XMLDocument();
            tinyxml2::XMLError error       = document.Parse(result.c_str());

            if (error != tinyxml2::XMLError::XML_SUCCESS) {
                brls::Logger::error("Error decode danmaku xml[1]: {}",
                                    std::to_string(error));
                return;
            }
            tinyxml2::XMLElement* element = document.RootElement();
            if (!element) {
                brls::Logger::error(
                    "Error decode danmaku xml[2]: no root element");
                return;
            }

            std::vector<DanmakuItem> items;
            for (auto child = element->FirstChildElement(); child != nullptr;
                 child      = child->NextSiblingElement()) {
                if (child->Name()[0] != 'd') continue;  // 简易判断是不是弹幕
                try {
                    items.emplace_back(
                        DanmakuItem(child->GetText(), child->Attribute("p")));
                } catch (...) {
                    brls::Logger::error("DANMAKU: error decode: {}",
                                        child->GetText());
                }
            }

            brls::sync(
                [items]() { MPVCore::instance().loadDanmakuData(items); });

            brls::Logger::debug("DANMAKU: decode done: {}", items.size());
        },
        [ASYNC_TOKEN](const std::string& error) {
            ASYNC_RELEASE
            brls::Logger::error(error);
        });
}

/// 上报历史记录
void VideoDetail::reportHistory(unsigned int aid, unsigned int cid,
                                unsigned int progress, int type) {
    if (!REPORT_HISTORY) return;
    brls::Logger::debug("reportHistory: aid{} cid{} progress{}", aid, cid,
                        progress);
    std::string mid   = ProgramConfig::instance().getUserID();
    std::string token = ProgramConfig::instance().getCSRF();
    if (mid == "" || token == "") return;
    unsigned int sid = 0, epid = 0;
    if (type == 4) {
        sid  = seasonInfo.season_id;
        epid = episodeResult.id;
    }

    bilibili::BilibiliClient::report_history(mid, token, aid, cid, type,
                                             progress, sid, epid);
}

/// 点赞
void VideoDetail::beAgree(int aid) {
    std::string token = ProgramConfig::instance().getCSRF();
    if (token == "") return;
    bilibili::BilibiliClient::be_agree(token, aid, true);
}

/// 投币
void VideoDetail::addCoin(int aid) {
    std::string token = ProgramConfig::instance().getCSRF();
    if (token == "") return;
    bilibili::BilibiliClient::add_coin(token, aid, 1, true);
}

/// 收藏
void VideoDetail::addResource(int aid) {
    std::string token = ProgramConfig::instance().getCSRF();
    if (token == "") return;
    bilibili::BilibiliClient::add_resource(token, aid);
}