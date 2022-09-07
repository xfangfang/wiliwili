//
// Created by fang on 2022/4/22.
//

#pragma once

#include <borealis.hpp>
#include "presenter/video_detail.hpp"

#include "view/video_comment.hpp"
#include "view/recycling_grid.hpp"
#include "view/auto_tab_frame.hpp"
#include "utils/singleton.hpp"
#include "view/mpv_core.hpp"

class VideoView;
class UserInfoView;
class SVGImage;

typedef brls::Event<int> ChangeIndexEvent;
typedef brls::Event<bilibili::Video> ChangeVideoEvent;

class PlayerActivity : public brls::Activity, public VideoDetail {
public:
    CONTENT_FROM_XML_RES("activity/player_activity.xml");

    PlayerActivity() = default;

    PlayerActivity(std::string bvid);

    PlayerActivity(std::string bvid, unsigned int cid, int progress = -1);

    void onContentAvailable() override;

    void onVideoInfo(const bilibili::VideoDetailResult& result) override;
    void onVideoPageListInfo(
        const bilibili::VideoDetailPageListResult& result) override;
    void onUploadedVideos(
        const bilibili::UserUploadedVideoResultWrapper& result) override;
    void onVideoPlayUrl(const bilibili::VideoUrlResult& result) override;
    void onCommentInfo(
        const bilibili::VideoCommentResultWrapper& result) override;
    void onError(const std::string& error) override;
    void onRequestCommentError(const std::string& error) override;
    void onVideoOnlineCount(const bilibili::VideoOnlineTotal& count) override;
    void onVideoRelationInfo(const bilibili::VideoRelation& result) override;
    void onRelatedVideoList(
        const bilibili::VideoDetailListResult& result) override;
    void onDanmaku(const std::string& filePath) override;

    // 初始化设置 播放界面通用内容
    void setCommonData();

    // 设置 点赞、收藏、投币 三个按钮的样式
    void setRelationButton(bool liked, bool coin, bool favorite);

    // 展示二维码共享对话框
    void showShareDialog(const std::string link);

    ~PlayerActivity() override;

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

    //    bilibili::VideoDetailResult video_data;
    bool fullscreen                              = false;
    brls::ActionIdentifier videoExitFullscreenID = -1;

    // 切换视频分P
    ChangeIndexEvent changePEvent;

    // 切换UP视频
    ChangeVideoEvent changeVideoEvent;

    // 监控mpv事件
    MPVEvent::Subscription eventSubscribeID;
};

class PlayerSeasonActivity : public PlayerActivity {
public:
    PlayerSeasonActivity(const unsigned int id,
                         PGC_ID_TYPE type = PGC_ID_TYPE::SEASON_ID,
                         int progress     = -1);

    ~PlayerSeasonActivity() override;

    void onContentAvailable() override;

    void onSeasonVideoInfo(
        const bilibili::SeasonResultWrapper& result) override;

    void onSeasonEpisodeInfo(
        const bilibili::SeasonEpisodeResult& result) override;

private:
    unsigned int pgc_id;
    PGC_ID_TYPE pgcIdType;
    ChangeIndexEvent changeEpisodeEvent;
    BRLS_BIND(brls::Box, boxFavorites, "video/favorites/box");
    BRLS_BIND(brls::Label, videoFavoritesLabel, "video/favorites");
};