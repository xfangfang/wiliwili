//
// Created by fang on 2022/4/22.
//

#pragma once

#include <chrono>
#include "presenter/video_detail.hpp"

#include "view/video_comment.hpp"
#include "view/recycling_grid.hpp"
#include "view/auto_tab_frame.hpp"
#include "utils/event_helper.hpp"

class VideoView;
class UserInfoView;
class SVGImage;

typedef brls::Event<size_t> ChangeIndexEvent;
typedef brls::Event<bilibili::Video> ChangeVideoEvent;

using namespace brls::literals;

enum PlayerStrategy {
    RCMD = 0,
    NEXT,
    LOOP,
    SINGLE,
};

class BasePlayerActivity : public brls::Activity, public VideoDetail {
public:
    CONTENT_FROM_XML_RES("activity/player_activity.xml");

    BasePlayerActivity() = default;

    void onContentAvailable() override;

    void onVideoPlayUrl(const bilibili::VideoUrlResult& result) override;
    void onCommentInfo(const bilibili::VideoCommentResultWrapper& result) override;
    void onError(const std::string& error) override;
    void onRequestCommentError(const std::string& error) override;
    void onVideoOnlineCount(const bilibili::VideoOnlineTotal& count) override;
    void onVideoRelationInfo(const bilibili::VideoRelation& result) override;
    void onHighlightProgress(const bilibili::VideoHighlightProgress& result) override;

    // 初始化设置 播放界面通用内容
    void setCommonData();

    // 设置 点赞、收藏、投币 三个按钮的样式
    void setRelationButton(bool liked, bool coin, bool favorite);

    // 展示收藏列表对话框
    void showCollectionDialog(int64_t id, int videoType);

    // 展示投币对话框
    void showCoinDialog(size_t aid);

    // 设置清晰度
    void setVideoQuality();

    // 切换评论模式
    void setCommentMode();

    // 设定当前的播放进度，获取视频链接后会自动跳转到该进度
    // 目前有两个使用场景：
    // 1. 从历史记录进入视频时
    // 2. 切换清晰度前
    virtual void setProgress(int p) = 0;

    // 获取当前设定的播放进度
    // 在任何视频播放前都会从此接口读入播放进度，并在此基础上 -5s 进行播放
    virtual int getProgress() = 0;

    // 切换分集
    virtual void onIndexChange(size_t index) = 0;

    // 切换到下一集
    virtual void onIndexChangeToNext() = 0;

    // 上报播放进度
    virtual void reportCurrentProgress(size_t progress, size_t duration) = 0;

    // 获取当前视频的aid
    virtual size_t getAid() = 0;

    // 获取投屏链接
    virtual void requestCastUrl() = 0;

    void willDisappear(bool resetState = false) override;

    void willAppear(bool resetState = false) override;

    ~BasePlayerActivity() override;

    // 分集播放结束后，自动播放推荐视频
    // 视频播放策略
    // 0: 分集播放结束后，自动播放推荐视频
    // 1: 自动播放下一分集
    // 2: 循环播放当前视频
    // 3: 播完暂停
    inline static int PLAYER_STRATEGY = PlayerStrategy::RCMD;

    // 自动跳过片头
    inline static bool PLAYER_SKIP_OPENING_CREDITS = true;

protected:
    BRLS_BIND(VideoView, video, "video/detail/video");
    BRLS_BIND(brls::AppletFrame, appletFrame, "video/detail/frame");
    BRLS_BIND(UserInfoView, videoUserInfo, "video_author");
    BRLS_BIND(brls::Box, videoTitleBox, "video/title/box");
    BRLS_BIND(brls::Label, videoTitleLabel, "video/title");
    BRLS_BIND(brls::Label, videoBVIDLabel, "video/bvid");
    BRLS_BIND(brls::Label, videoCountLabel, "video/count");
    BRLS_BIND(brls::Label, videoTimeLabel, "video/time");
    BRLS_BIND(brls::Label, videoDanmakuLabel, "video/danmaku");
    BRLS_BIND(brls::Label, videoPeopleLabel, "video/people");
    BRLS_BIND(brls::Box, videoCopyRightBox, "video/right/box");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "video/comment/recyclingGrid");
    BRLS_BIND(AutoTabFrame, tabFrame, "player/tab_frame");
    BRLS_BIND(SVGImage, btnAgree, "video/btn/agree");
    BRLS_BIND(SVGImage, btnCoin, "video/btn/coin");
    BRLS_BIND(SVGImage, btnFavorite, "video/btn/favorite");
    BRLS_BIND(SVGImage, btnQR, "video/btn/qr");
    BRLS_BIND(brls::Label, labelAgree, "video/label/agree");
    BRLS_BIND(brls::Label, labelCoin, "video/label/coin");
    BRLS_BIND(brls::Label, labelFavorite, "video/label/favorite");
    BRLS_BIND(brls::Label, labelQR, "video/label/qr");

    // 监控mpv事件
    MPVEvent::Subscription eventSubscribeID;
    CustomEvent::Subscription customEventSubscribeID;

    // 在软件自动切换分集时，传递当前跳转的索引值给列表用于更新ui
    ChangeIndexEvent changeIndexEvent;

private:
    bool activityShown = false;
    std::chrono::system_clock::time_point videoDeadline{};
};

class PlayerActivity : public BasePlayerActivity {
public:
    PlayerActivity(const std::string& bvid, unsigned int cid = 0, int progress = -1);

    void setProgress(int p) override;
    int getProgress() override;
    void onIndexChange(size_t index) override;
    void onIndexChangeToNext() override;
    void reportCurrentProgress(size_t progress, size_t duration) override;
    void requestCastUrl() override;

    void onVideoInfo(const bilibili::VideoDetailResult& result) override;
    void onUpInfo(const bilibili::UserDetailResultWrapper& result) override;
    void onVideoPageListInfo(const bilibili::VideoDetailPageListResult& result) override;
    void onUGCSeasonInfo(const bilibili::UGCSeason& result) override;
    void onUploadedVideos(const bilibili::UserUploadedVideoResultWrapper& result) override;
    void onRelatedVideoList(const bilibili::VideoDetailListResult& result) override;
    void onRedirectToEp(const std::string& url) override;
    void onCastPlayUrl(const bilibili::VideoUrlResult& result) override;
    size_t getAid() override;

    void onContentAvailable() override;

    ~PlayerActivity() override;

private:
    // 切换UP视频
    ChangeVideoEvent changeVideoEvent;
};

class PlayerSeasonActivity : public BasePlayerActivity {
public:
    PlayerSeasonActivity(const unsigned int id, PGC_ID_TYPE type = PGC_ID_TYPE::SEASON_ID, int progress = -1);

    ~PlayerSeasonActivity() override;

    void setProgress(int p) override;

    int getProgress() override;

    void requestCastUrl() override;

    void onContentAvailable() override;

    void onSeasonVideoInfo(const bilibili::SeasonResultWrapper& result) override;

    void onSeasonSeriesInfo(const bilibili::SeasonSeries& result) override;

    void onSeasonRecommend(const bilibili::SeasonRecommendWrapper& result) override;

    void onSeasonStatus(const bilibili::SeasonStatusResult& result) override;

    void onSeasonEpisodeInfo(const bilibili::SeasonEpisodeResult& result) override;

    void onIndexChange(size_t index) override;

    void onIndexChangeToNext() override;

    void reportCurrentProgress(size_t progress, size_t duration) override;

    void onCastPlayUrl(const bilibili::VideoUrlResult& result) override;

    // 正在播放的情况下切换到新的番剧
    void playSeason(size_t season_id);

    size_t getAid() override;

private:
    unsigned int pgc_id;
    PGC_ID_TYPE pgcIdType;

    BRLS_BIND(brls::Box, boxFavorites, "video/favorites/box");
    BRLS_BIND(brls::Label, videoFavoritesLabel, "video/favorites");
};