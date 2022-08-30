//
// Created by fang on 2022/8/9.
//

#include "presenter/video_detail.hpp"
#include "borealis.hpp"

/// 请求视频数据
void VideoDetail::requestData(const bilibili::VideoDetailResult& video) {
    this->requestVideoInfo(video.bvid);
}

/// 请求番剧数据
void VideoDetail::requestData(int seasonID) {
    this->requestSeasonInfo(seasonID);
}

/// 获取番剧信息
void VideoDetail::requestSeasonInfo(const int seasonID){
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
    }, [ASYNC_TOKEN](const std::string &error) {
        ASYNC_RELEASE
        brls::Logger::error(error);
    });
}

/// 获取视频信息：标题、作者、简介、分P等
void VideoDetail::requestVideoInfo(const string bvid) {

    // 请求视频点赞情况
    this->requestVideoRelationInfo(bvid);

    ASYNC_RETAIN
    brls::Logger::debug("请求视频信息: {}", bvid);
    bilibili::BilibiliClient::get_video_detail_all(bvid,
                                               [ASYNC_TOKEN](const bilibili::VideoDetailAllResult &result) {
                                                   brls::sync([ASYNC_TOKEN, result](){
                                                       ASYNC_RELEASE
                                                       brls::Logger::debug("bilibili::BilibiliClient::get_video_detail");
                                                       this->videoDetailResult = result.View;
                                                       this->userDetailResult = result.Card;

                                                       // 请求视频评论
                                                       this->requestVideoComment(this->videoDetailResult.aid, 1);

                                                       // 请求用户投稿列表
                                                       this->requestUploadedVideos(this->videoDetailResult.owner.mid, 1);

                                                       //  请求视频播放地址
                                                       this->onVideoPageListInfo(this->videoDetailResult.pages);
                                                       for(const auto& i: this->videoDetailResult.pages){
                                                           brls::Logger::debug("获取视频分P列表: PV1 {}", i.cid);
                                                           // 播放PV1
                                                           this->requestVideoUrl(this->videoDetailResult.bvid, i.cid);
                                                           break;
                                                       }

                                                       this->onVideoInfo(this->videoDetailResult);
                                                       this->onRelatedVideoList(result.Related);
                                                   });

                                               }, [ASYNC_TOKEN](const std::string &error) {
                ASYNC_RELEASE
                brls::Logger::error("ERROR:请求视频信息 {}", error);
                this->onError(error);

            });
}

/// 获取视频地址
void VideoDetail::requestVideoUrl(std::string bvid, int cid){

    // 请求当前视频在线人数
    this->requestVideoOnline(bvid, cid);

    ASYNC_RETAIN
    brls::Logger::debug("请求视频播放地址: {}/{}", bvid, cid);
    bilibili::BilibiliClient::get_video_url(bvid, cid, 116,
                                            [ASYNC_TOKEN](const bilibili::VideoUrlResult & result) {
                                                brls::sync([ASYNC_TOKEN, result](){
                                                    ASYNC_RELEASE
                                                    brls::Logger::debug("bilibili::BilibiliClient::get_video_url : {}", result.quality);
                                                    this->videoUrlResult = result;
                                                    this->onVideoPlayUrl(result);
                                                });
                                            }, [ASYNC_TOKEN](const std::string &error) {
                ASYNC_RELEASE
                brls::Logger::error(error);
            });
}

/// 获取番剧地址
void VideoDetail::requestSeasonVideoUrl(const std::string& bvid, int cid){

    // 请求当前视频在线人数
    this->requestVideoOnline(bvid, cid);

    ASYNC_RETAIN
    brls::Logger::debug("请求番剧视频播放地址: {}", cid);
    bilibili::BilibiliClient::get_season_url(cid, 116,
                                             [ASYNC_TOKEN](const bilibili::VideoUrlResult & result) {
                                                 brls::sync([ASYNC_TOKEN, result](){
                                                     ASYNC_RELEASE
                                                     brls::Logger::debug("bilibili::BilibiliClient::get_video_url");
                                                     this->videoUrlResult = result;
                                                     this->onVideoPlayUrl(result);
                                                 });
                                             }, [ASYNC_TOKEN](const std::string &error) {
                ASYNC_RELEASE
                brls::Logger::error(error);
            });
}

/// 切换番剧分集
void VideoDetail::changeEpisode(const bilibili::SeasonEpisodeResult& i){
    episodeResult = i;
    this->onSeasonEpisodeInfo(i);
    this->requestVideoComment(i.aid, 1);
    this->requestSeasonVideoUrl(i.bvid, i.cid);
}

/// 获取视频评论
void VideoDetail::requestVideoComment(int aid, int next, int mode){
    if(next != 0){
        this->commentRequestIndex = next;
    }
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_comment(aid, commentRequestIndex, mode,
                                          [ASYNC_TOKEN](const bilibili::VideoCommentResultWrapper& result) {
                                              brls::sync([ASYNC_TOKEN, result](){
                                                  ASYNC_RELEASE
                                                  if(this->commentRequestIndex != result.cursor.prev)
                                                      return;
                                                  if(!result.cursor.is_end){
                                                      this->commentRequestIndex = result.cursor.next;
                                                  }
                                                  this->onCommentInfo(result);
                                              });
                                          }, [ASYNC_TOKEN](const std::string &error) {
                ASYNC_RELEASE
                this->onRequestCommentError(error);
            });
}

/// 获取Up主的其他视频
void VideoDetail::requestUploadedVideos(int mid, int pn, int ps){
    if(pn != 0){
        this->userUploadedVideoRequestIndex = pn;
    }
    brls::Logger::debug("请求投稿视频: {}/{}", mid, userUploadedVideoRequestIndex);
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_user_videos(mid, userUploadedVideoRequestIndex, ps,
                                              [ASYNC_TOKEN](const bilibili::UserUploadedVideoResultWrapper& result){
        brls::sync([ASYNC_TOKEN, result](){
            ASYNC_RELEASE
            if(result.page.pn != this->userUploadedVideoRequestIndex)
                return;
            if(!result.list.empty()){
                this->userUploadedVideoRequestIndex = result.page.pn + 1;
            }
            this->onUploadedVideos(result);
        });
    }, [ASYNC_TOKEN](const std::string &error) {
        ASYNC_RELEASE
        brls::Logger::error(error);
    });
}

/// 获取单个视频播放人数
void VideoDetail::requestVideoOnline(const std::string& bvid, int cid){
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_video_online(bvid, cid,
        [ASYNC_TOKEN](const bilibili::VideoOnlineTotal& result){
            brls::sync([ASYNC_TOKEN, result](){
                ASYNC_RELEASE
                this->onVideoOnlineCount(result);
            });
        }, [ASYNC_TOKEN](const std::string &error) {
            ASYNC_RELEASE
            brls::Logger::error(error);
        });
}

/// 获取视频的 点赞、投币、收藏情况
void VideoDetail::requestVideoRelationInfo(const std::string& bvid){
    ASYNC_RETAIN
    bilibili::BilibiliClient::get_video_relation(bvid,
       [ASYNC_TOKEN](const bilibili::VideoRelation& result){
           brls::sync([ASYNC_TOKEN, result](){
               ASYNC_RELEASE
               this->onVideoRelationInfo(result);
           });
       }, [ASYNC_TOKEN](const std::string &error) {
        ASYNC_RELEASE
        brls::Logger::error(error);
    });
}