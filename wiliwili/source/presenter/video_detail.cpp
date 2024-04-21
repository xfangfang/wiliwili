//
// Created by fang on 2022/8/9.
//
#include <cstdlib>
#include <tinyxml2.h>
#include <pystring.h>
#include <borealis/core/thread.hpp>

#include "presenter/video_detail.hpp"
#include "utils/config_helper.hpp"
#include "utils/number_helper.hpp"
#include "view/danmaku_core.hpp"
#include "view/subtitle_core.hpp"
#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "bilibili/result/mine_collection_result.h"

/// 请求视频数据
void VideoDetail::requestData(const bilibili::VideoDetailResult& video) { this->requestVideoInfo(video.bvid); }

/// 请求番剧数据
void VideoDetail::requestData(size_t id, PGC_ID_TYPE type) {
    if (type == PGC_ID_TYPE::SEASON_ID)
        this->requestSeasonInfo(id, 0);
    else if (type == PGC_ID_TYPE::EP_ID)
        this->requestSeasonInfo(0, id);
}

/// 获取番剧信息
void VideoDetail::requestSeasonInfo(size_t seasonID, size_t epID) {
    // 重置MPV
    MPVCore::instance().reset();

    brls::Logger::debug("请求番剧信息 season: {}; ep: {}", seasonID, epID);
    ASYNC_RETAIN
    BILI::get_season_detail(
        seasonID, epID,
        [ASYNC_TOKEN, epID](const bilibili::SeasonResultWrapper& result) {
            brls::sync([ASYNC_TOKEN, result, epID]() {
                ASYNC_RELEASE
                seasonInfo = result;
                this->requestSeasonStatue(result.season_id);

                // 合并数据，将所有分集合并成一个列表
                // 1. 若存在额外分区, 切正片不为空则在正片分集前添加分区标题 "正片"
                // 2. 将分区标题设置为仅有 title，id为0的分集项，显示时根据id来正确显示样式
                // 3. 修改标题
                episodeList.clear();
                episodeList = seasonInfo.episodes;

                for (auto& e : episodeList) {
                    char* stop = nullptr;
                    auto index = strtol(e.title.c_str(), &stop, 10);
                    bool isNum = strlen(stop) == 0;
                    switch (result.type) {
                        case 1:  // 日漫 ？
                        case 4:  // 国漫 ？
                            if (isNum) {
                                e.title = fmt::format("第{}话 {}", index, e.long_title);
                            } else {
                                e.title = fmt::format("{} {}", e.title, e.long_title);
                            }
                            break;
                        case 3:  // 纪录片
                        case 5:  // 电视剧
                            if (isNum) {
                                e.title = fmt::format("第{}集 {}", index, e.long_title);
                            } else {
                                e.title = fmt::format("{} {}", e.title, e.long_title);
                            }
                            break;
                        case 2:  // 电影
                        case 7:  // 综艺
                        default:
                            break;
                    }
                }
                if (!result.section.empty() && episodeList.size() > 0)
                    episodeList.insert(episodeList.begin(), bilibili::SeasonEpisodeResult{"正片"});

                for (auto& s : seasonInfo.section) {
                    episodeList.emplace_back(bilibili::SeasonEpisodeResult{s.title});
                    for (auto& e : s.episodes) {
                        e.title = e.title + " " + e.long_title;
                        pystring::strip(e.title);
                    }
                    episodeList.insert(episodeList.end(), s.episodes.begin(), s.episodes.end());
                }

                for (size_t i = 0; i < episodeList.size(); i++) episodeList[i].index = i;

                size_t ep_id = epID;
                if (ep_id == 0) {
                    ep_id                  = result.user_status.last_ep_id;
                    episodeResult.progress = result.user_status.last_time;
                }

                // 加载指定epid的视频
                if (ep_id != 0) {
                    for (auto& i : episodeList) {
                        if (i.id == (unsigned int)ep_id) {
                            brls::Logger::debug("Load episode {} from epid: {}", i.long_title, i.id);
                            if (ep_id == result.user_status.last_ep_id)
                                i.progress = result.user_status.last_time;
                            else
                                i.progress = episodeResult.progress;
                            this->changeEpisode(i);
                            this->onSeasonVideoInfo(result);
                            this->onSeasonSeriesInfo(result.seasons);
                            this->requestSeasonRecommend(result.season_id);
                            return;
                        }
                    }
                }
                // 若未指定epid，则按照列表顺序加载
                for (auto& i : episodeList) {
                    if (i.id == 0 || i.cid == 0 || i.aid == 0) continue;
                    brls::Logger::debug("Load episode {} epid: {}", i.long_title, i.id);
                    episodeResult.progress = 0;
                    this->changeEpisode(i);
                    break;
                }
                this->onSeasonVideoInfo(result);
                this->onSeasonSeriesInfo(result.seasons);
                this->requestSeasonRecommend(result.season_id);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->onError(error);
            });
        });
}

void VideoDetail::requestSeasonRecommend(size_t seasonID) {
    ASYNC_RETAIN
    BILI::get_season_recommend(
        seasonID,
        [ASYNC_TOKEN](const bilibili::SeasonRecommendWrapper& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                this->onSeasonRecommend(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("{}", error);
        });
}

void VideoDetail::requestSeasonStatue(size_t seasonID) {
    ASYNC_RETAIN
    BILI::get_season_status(
        seasonID,
        [ASYNC_TOKEN](const bilibili::SeasonStatusResult& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                seasonStatus = result;
                this->onSeasonStatus(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("{}", error);
        });
}

/// 获取视频信息：标题、作者、简介、分P等
void VideoDetail::requestVideoInfo(const std::string& bvid) {
    // 重置MPV
    MPVCore::instance().reset();

    ASYNC_RETAIN
    brls::Logger::debug("请求视频信息: {}", bvid);
    BILI::get_video_detail_all(
        bvid,
        [ASYNC_TOKEN](const bilibili::VideoDetailAllResult& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                brls::Logger::debug("BILI::get_video_detail");
                this->videoDetailResult = result.View;
                this->userDetailResult  = result.Card;
                this->videDetailRelated = result.Related;

                if (!this->videoDetailResult.redirect_url.empty()) {
                    // eg: https://www.bilibili.com/bangumi/play/ep568278
                    std::vector<std::string> items;
                    pystring::split(this->videoDetailResult.redirect_url, items, "/");
                    std::string epid = items[items.size() - 1];
                    if (pystring::startswith(epid, "ep")) {
                        this->onRedirectToEp(pystring::slice(epid, 2));
                        return;
                    } else {
                        brls::Logger::error("unknown redirect url: {}", videoDetailResult.redirect_url);
                    }
                }

                // 如果请求前就设定了指定分P，那么尝试打开指定的分P，两种情况会预设cid
                // 1. 从历史记录打开视频
                // 2. 切换分P播放
                if (videoDetailPage.cid != 0) {
                    for (const auto& i : this->videoDetailResult.pages) {
                        if (i.cid == videoDetailPage.cid) {
                            brls::Logger::debug("获取视频分P列表: PV {}", i.cid);
                            videoDetailPage = i;
                            break;
                        }
                    }
                } else {
                    // 其他两种情况打开PV1
                    // 1. 未指定PV
                    // 2. 指定了错误的PV（比如Up主重新上传过视频，那么历史记录中保存的PV就是错误的）
                    for (const auto& i : this->videoDetailResult.pages) {
                        brls::Logger::debug("获取视频分P列表: PV1 {}", i.cid);
                        videoDetailPage = i;
                        break;
                    }
                }

                if (videoDetailPage.cid == 0) {
                    brls::Logger::error("未获取到视频列表");
                    return;
                }

                // 请求视频播放地址
                this->requestVideoUrl(this->videoDetailResult.bvid, this->videoDetailPage.cid);

                // 展示视频相关信息
                this->onUpInfo(this->userDetailResult);
                this->onVideoInfo(this->videoDetailResult);

                // 展示分P数据
                this->onVideoPageListInfo(this->videoDetailResult.pages);

                // 展示合集数据
                if (!videoDetailResult.ugc_season.sections.empty()) this->onUGCSeasonInfo(videoDetailResult.ugc_season);

                // 请求视频评论
                this->requestVideoComment(std::to_string(this->videoDetailResult.aid), 0, 3);

                // 请求用户投稿列表
                this->requestUploadedVideos(videoDetailResult.owner.mid, 1);

                // 展示相关推荐
                this->onRelatedVideoList(videDetailRelated);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("ERROR:请求视频信息 {}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->onError(error);
            });
        });

    // 请求视频点赞情况
    this->requestVideoRelationInfo(bvid);
    GA("plain_video", {{"bvid", bvid}})
}

/// 获取视频地址
void VideoDetail::requestVideoUrl(std::string bvid, int cid, bool requestHistoryInfo) {
    // 重置MPV
    MPVCore::instance().reset();
    ASYNC_RETAIN
    brls::Logger::debug("请求视频播放地址: {}/{}/{}", bvid, cid, defaultQuality);
    if (cid == 0) return;

    BILI::get_video_url(
        bvid, cid, defaultQuality,
        [ASYNC_TOKEN](const bilibili::VideoUrlResult& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                this->videoUrlResult = result;
                this->onVideoPlayUrl(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->onError("请求视频地址失败\n" + error);
            });
        });
    // 请求当前视频在线人数
    this->requestVideoOnline(bvid, cid);
    // 请求弹幕
    this->requestVideoDanmaku(cid);
    // 请求分P详情 （字幕链接/历史播放记录）
    this->requestVideoPageDetail(bvid, cid, requestHistoryInfo);
    // 请求高能进度条
    this->requestHighlightProgress(cid);
}

/// 获取番剧地址
void VideoDetail::requestSeasonVideoUrl(const std::string& bvid, int cid, bool requestHistoryInfo) {
    // 重置MPV
    MPVCore::instance().reset();

    ASYNC_RETAIN
    brls::Logger::debug("请求番剧视频播放地址: {}", cid);
    BILI::get_season_url(
        cid, defaultQuality,
        [ASYNC_TOKEN](const bilibili::VideoUrlResult& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                brls::Logger::debug("BILI::get_video_url");
                this->videoUrlResult = result;
                this->onVideoPlayUrl(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                this->onError("请求视频地址失败\n" + error);
            });
        });

    // 请求当前视频在线人数
    this->requestVideoOnline(bvid, cid);
    // 请求弹幕
    this->requestVideoDanmaku(cid);
    // 请求分P详情 （字幕链接/历史播放记录）
    this->requestVideoPageDetail(bvid, cid, requestHistoryInfo);
    // 请求高能进度条
    this->requestHighlightProgress(cid);
}

/// 获取投屏地址
void VideoDetail::requestCastVideoUrl(int oid, int cid, int type) {
    ASYNC_RETAIN
    brls::Logger::debug("请求投屏视频播放地址: {}/{}", oid, cid);
    BILI::get_video_url_cast(
        oid, cid, type, 120, ProgramConfig::instance().getCSRF(),
        [ASYNC_TOKEN](const bilibili::VideoUrlResult& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                if (result.durl.empty()) {
                    brls::Logger::error("requestCastVideoUrl: empty durl");
                    APP_E->fire("CAST_URL_ERROR", nullptr);
                    return;
                }
                this->onCastPlayUrl(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                APP_E->fire("CAST_URL_ERROR", nullptr);
            });
        });
}

/// 获取当前清晰度的序号，默认为0，从最高清开始排起
int VideoDetail::getQualityIndex() {
    for (size_t i = 0; i < videoUrlResult.accept_quality.size(); i++) {
        if (videoUrlResult.accept_quality[i] == videoUrlResult.quality) {
            return i;
        }
    }
    return 0;
}

/// 切换番剧分集
void VideoDetail::changeEpisode(const bilibili::SeasonEpisodeResult& i) {
    if (i.id == 0 || i.cid == 0 || i.aid == 0) return;
    episodeResult = i;

    //上报历史记录
    int progress = 0;
    if (episodeResult.progress > 0) progress = episodeResult.progress;

    this->reportHistory(i.aid, i.cid, progress, 0, 4);

    // 重置MPV
    MPVCore::instance().reset();

    this->onSeasonEpisodeInfo(i);
    this->requestSeasonVideoUrl(i.bvid, i.cid);
    this->requestVideoComment(std::to_string(i.aid), 0, 3);
    this->requestVideoRelationInfo(i.id);
    GA("season_video", {{"bvid", i.bvid}})
}

/// 获取Up主的其他视频
void VideoDetail::requestUploadedVideos(int64_t mid, int pn, int ps) {
    if (pn != 0) {
        this->userUploadedVideoRequestIndex = pn;
    }
    brls::Logger::debug("请求投稿视频: {}/{}", mid, userUploadedVideoRequestIndex);
    ASYNC_RETAIN
    BILI::get_user_videos(
        mid, userUploadedVideoRequestIndex, ps,
        [ASYNC_TOKEN](const bilibili::UserUploadedVideoResultWrapper& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                if (result.page.pn != this->userUploadedVideoRequestIndex) return;
                if (!result.list.empty()) {
                    this->userUploadedVideoRequestIndex = result.page.pn + 1;
                }
                this->onUploadedVideos(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("{}", error);
        });
}

/// 获取单个视频播放人数
void VideoDetail::requestVideoOnline(const std::string& bvid, int cid) {
    brls::Logger::debug("请求当前视频在线人数: bvid: {} cid: {}", bvid, cid);
    ASYNC_RETAIN
    BILI::get_video_online(
        bvid, cid,
        [ASYNC_TOKEN](const bilibili::VideoOnlineTotal& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                this->onVideoOnlineCount(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("{}", error);
        });
}

/// 获取视频的 点赞、投币、收藏情况
void VideoDetail::requestVideoRelationInfo(const std::string& bvid) {
    ASYNC_RETAIN
    BILI::get_video_relation(
        bvid,
        [ASYNC_TOKEN](const bilibili::VideoRelation& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                videoRelation = result;
                this->onVideoRelationInfo(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("{}", error);
        });
}

/// 获取番剧分集的 点赞、投币、收藏情况
void VideoDetail::requestVideoRelationInfo(size_t epid) {
    ASYNC_RETAIN
    BILI::get_video_relation(
        epid,
        [ASYNC_TOKEN](const bilibili::VideoEpisodeRelation& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                videoDetailResult.copyright = result.user_community.is_original;
                videoRelation.like          = result.user_community.like;
                videoRelation.coin          = result.user_community.coin_number;
                videoRelation.favorite      = result.user_community.favorite;
                this->onVideoRelationInfo(videoRelation);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("{}", error);
        });
}

/// 获取视频弹幕
void VideoDetail::requestVideoDanmaku(int cid) {
    brls::Logger::debug("请求弹幕：cid: {}", cid);
    ASYNC_RETAIN
    BILI::get_danmaku(
        cid,
        [ASYNC_TOKEN](const std::string& result) {
            ASYNC_RELEASE
            brls::Logger::debug("DANMAKU: start decode");

            // Load XML
            tinyxml2::XMLDocument document = tinyxml2::XMLDocument();
            tinyxml2::XMLError error       = document.Parse(result.c_str());

            if (error != tinyxml2::XMLError::XML_SUCCESS) {
                brls::Logger::error("Error decode danmaku xml[1]: {}", std::to_string(error));
                return;
            }
            tinyxml2::XMLElement* element = document.RootElement();
            if (!element) {
                brls::Logger::error("Error decode danmaku xml[2]: no root element");
                return;
            }

            std::vector<DanmakuItem> items;
            for (auto child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
                if (child->Name()[0] != 'd') continue;  // 简易判断是不是弹幕
                const char* content = child->GetText();
                if (!content) continue;
                try {
                    items.emplace_back(content, child->Attribute("p"));
                } catch (...) {
                    brls::Logger::error("DANMAKU: error decode: {}", child->GetText());
                }
            }

            brls::sync([items]() { DanmakuCore::instance().loadDanmakuData(items); });

            brls::Logger::debug("DANMAKU: decode done: {}", items.size());
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("{}", error);
        });
}

/// 获取视频分P详情
void VideoDetail::requestVideoPageDetail(const std::string& bvid, int cid, bool requestVideoHistory) {
    brls::Logger::debug("请求字幕：bvid: {} cid: {}", bvid, cid);
    ASYNC_RETAIN
    BILI::get_page_detail(
        bvid, cid,
        [ASYNC_TOKEN, requestVideoHistory](const bilibili::VideoPageResult& result) {
#if defined(BOREALIS_USE_D3D11) || defined(BOREALIS_USE_OPENGL) && !defined(__PSV__)
            if (!result.mask_url.empty()) {
                brls::Logger::debug("获取防遮挡数据: {}", result.mask_url);
                auto url = pystring::startswith(result.mask_url, "//") ? "https:" + result.mask_url : result.mask_url;
                DanmakuCore::instance().loadMaskData(url);
            }
#endif
            brls::sync([ASYNC_TOKEN, result, requestVideoHistory]() {
                ASYNC_RELEASE
                SubtitleCore::instance().setSubtitleList(result);
                // 存在UP主设置的字幕
                if (!result.subtitles.empty() && pystring::count(result.subtitles[0].lan, "ai") <= 0) {
                    SubtitleCore::instance().selectSubtitle(0);
                }

                if (!requestVideoHistory) return;

                brls::Logger::debug("历史播放进度：{}/{}", result.last_play_cid, result.last_play_time);
                // 之前播放过此视频，或其他视频分集
                if (videoDetailPage.cid && result.last_play_cid) {
                    if (result.last_play_cid == videoDetailPage.cid) {
                        if (result.last_play_time <= 0) return;
                        // 之前播放过此视频
                        APP_E->fire(VideoView::LAST_TIME, (void*)&(result.last_play_time));
                    } else {
                        // 之前播放过同一合集的其他视频
                        for (auto& p : videoDetailResult.pages) {
                            if (p.cid != result.last_play_cid) continue;
                            std::string hint = fmt::format("上次看到第 {} 个: {}", p.page, p.part);
                            if (result.last_play_time > 0) {
                                hint += " " + wiliwili::sec2Time(result.last_play_time / 1000);
                            } else {
                                hint += " 已看完";
                            }
                            APP_E->fire(VideoView::HINT, (void*)hint.c_str());
                            break;
                        }
                    }
                }
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("{}", error);
        });
}

/// 上报历史记录
void VideoDetail::reportHistory(unsigned int aid, unsigned int cid, unsigned int progress, unsigned int duration,
                                int type) {
    if (!REPORT_HISTORY) return;
    if (aid == 0 || cid == 0) return;
    brls::Logger::debug("reportHistory: aid{} cid{} progress{} duration{}", aid, cid, progress, duration);
    std::string mid   = ProgramConfig::instance().getUserID();
    std::string token = ProgramConfig::instance().getCSRF();
    if (mid.empty() || token.empty()) return;
    unsigned int sid = 0, epid = 0;
    if (type == 4) {
        sid  = seasonInfo.season_id;
        epid = episodeResult.id;
    }

    BILI::report_history(
        mid, token, aid, cid, type, progress, duration, sid, epid,
        []() { brls::Logger::debug("reportHistory: success"); },
        [](BILI_ERR) { brls::Logger::error("reportHistory: {}", error); });
}

int VideoDetail::getCoinTolerate() {
    // 非番剧视频
    int total = videoDetailResult.copyright == 1 ? 2 : 1;
    return total - videoRelation.coin;
}

/// 点赞
void VideoDetail::beAgree(int aid) {
    std::string csrf = ProgramConfig::instance().getCSRF();
    if (csrf.empty()) return;

    // 在返回前预先设置状态
    bool like                    = !videoRelation.like;
    bilibili::VideoRelation temp = videoRelation;
    temp.like                    = like;
    this->onVideoRelationInfo(temp);

    ASYNC_RETAIN
    BILI::be_agree(
        csrf, aid, like,
        [ASYNC_TOKEN, like]() {
            ASYNC_RELEASE
            videoRelation.like = like;
        },
        [ASYNC_TOKEN](BILI_ERR) {
            // 请求失败 恢复默认状态
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN]() {
                ASYNC_RELEASE
                this->onVideoRelationInfo(videoRelation);
            });
        });
}

/// 投币
void VideoDetail::addCoin(int aid, int num, bool like) {
    std::string csrf = ProgramConfig::instance().getCSRF();
    if (csrf.empty()) return;
    if (num < 1 || num > 2) return;

    // 在返回前预先设置状态
    bilibili::VideoRelation temp = videoRelation;
    temp.coin += num;
    temp.like |= like;
    this->onVideoRelationInfo(temp);

    // 若请求过慢，且在加载期间切换到其他视频可能会导致其他视频显示错误数据
    // 不过应该是小概率事件
    ASYNC_RETAIN
    BILI::add_coin(
        csrf, aid, num, like,
        [ASYNC_TOKEN, num, like]() {
            ASYNC_RELEASE
            videoRelation.coin += num;
            videoRelation.like |= like;
        },
        [ASYNC_TOKEN](BILI_ERR) {
            // 请求失败 恢复默认状态
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN, error]() {
                ASYNC_RELEASE
                // 投币达到上限
                if (pystring::count(error, "34005")) videoRelation.coin = 2;
                this->onVideoRelationInfo(videoRelation);
            });
        });
}

/// 收藏
void VideoDetail::addResource(int aid, int type, bool isFavorite, std::string add, std::string del) {
    std::string csrf = ProgramConfig::instance().getCSRF();
    if (csrf.empty()) return;

    if (add.empty() && del.empty()) return;

    // 在返回前预先设置状态
    bilibili::VideoRelation temp = videoRelation;
    temp.favorite                = isFavorite;
    this->onVideoRelationInfo(temp);

    brls::Logger::debug("addResource: {} {} {} {}", aid, type, add, del);
    ASYNC_RETAIN
    BILI::add_resource(
        csrf, aid, type, add, del,
        [ASYNC_TOKEN, isFavorite]() {
            ASYNC_RELEASE
            videoRelation.favorite = isFavorite;
        },
        [ASYNC_TOKEN](BILI_ERR) {
            // 请求失败 恢复默认状态
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN]() {
                ASYNC_RELEASE
                this->onVideoRelationInfo(videoRelation);
            });
        });
}

void VideoDetail::requestHighlightProgress(int cid) {
    brls::Logger::debug("请求高能进度条：cid: {}", cid);
    ASYNC_RETAIN
    BILI::get_highlight_progress(
        cid,
        [ASYNC_TOKEN](const bilibili::VideoHighlightProgress& result) {
            brls::sync([ASYNC_TOKEN, result]() {
                ASYNC_RELEASE
                this->onHighlightProgress(result);
            });
        },
        [ASYNC_TOKEN](BILI_ERR) {
            ASYNC_RELEASE
            brls::Logger::error("HighlightProgress: {}", error);
            this->onHighlightProgress(bilibili::VideoHighlightProgress{});
        });
}

void VideoDetail::followUp(const std::string& mid, bool follow) {
    std::string csrf = ProgramConfig::instance().getCSRF();
    if (csrf.empty()) return;

    // 返回前预先设置状态
    bilibili::UserDetailResultWrapper temp = this->userDetailResult;
    temp.following                         = follow;
    this->onUpInfo(temp);

    ASYNC_RETAIN
    BILI::follow_up(
        csrf, mid, follow,
        [ASYNC_TOKEN, follow]() {
            ASYNC_RELEASE
            userDetailResult.following = follow;
        },
        [ASYNC_TOKEN](BILI_ERR) {
            // 请求失败 恢复默认状态
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN]() {
                ASYNC_RELEASE
                this->onUpInfo(this->userDetailResult);
            });
        });
}

void VideoDetail::followSeason(size_t season, bool follow) {
    std::string csrf = ProgramConfig::instance().getCSRF();
    if (csrf.empty()) return;

    // 返回前预先设置状态
    bilibili::SeasonStatusResult temp = this->seasonStatus;
    temp.follow                       = follow;
    this->onSeasonStatus(temp);

    ASYNC_RETAIN
    BILI::follow_season(
        csrf, season, follow,
        [ASYNC_TOKEN, follow]() {
            ASYNC_RELEASE
            seasonStatus.follow = follow;
        },
        [ASYNC_TOKEN](BILI_ERR) {
            // 请求失败 恢复默认状态
            brls::Logger::error("{}", error);
            brls::sync([ASYNC_TOKEN]() {
                ASYNC_RELEASE
                this->onSeasonStatus(this->seasonStatus);
            });
        });
}