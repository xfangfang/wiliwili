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

class VideoView;
class UserInfoView;

typedef brls::Event<int> ChangeIndexEvent;
typedef brls::Event<bilibili::Video> ChangeVideoEvent;

class PlayerActivity : public brls::Activity, public VideoDetail
{
public:
    CONTENT_FROM_XML_RES("activity/player_activity.xml");

    PlayerActivity() = default;

    PlayerActivity(std::string bvid);

    void onContentAvailable() override;

    void onVideoInfo(const bilibili::VideoDetailResult &result) override;
    void onVideoPageListInfo(const bilibili::VideoDetailPageListResult &result) override;
    void onUploadedVideos(const bilibili::UserUploadedVideoResultWrapper& result) override;
    void onVideoPlayUrl(const bilibili::VideoUrlResult & result) override;
    void onCommentInfo(const bilibili::VideoCommentResultWrapper &result) override;
    void onError(const std::string &error) override;

    ~PlayerActivity() override;

protected:
    BRLS_BIND(VideoView, video, "video/detail/video");
    BRLS_BIND(brls::AppletFrame, appletFrame, "video/detail/frame");
    BRLS_BIND(UserInfoView, videoUserInfo, "video_author");
    BRLS_BIND(brls::Label, videoTitleLabel, "video_title");
    BRLS_BIND(brls::Label, videoIntroLabel, "video_intro");
    BRLS_BIND(brls::Label, videoInfoLabel, "video_info");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "video/comment/recyclingGrid");
    BRLS_BIND(AutoTabFrame, tabFrame, "player/tab_frame");

    bilibili::VideoDetailResult video_data;
    bool fullscreen = false;
    brls::ActionIdentifier videoExitFullscreenID = -1;

    // 切换视频分P
    ChangeIndexEvent changePEvent;

    // 切换视频
    ChangeVideoEvent changeVideoEvent;
};

class PlayerSeasonActivity: public PlayerActivity {
public:

    PlayerSeasonActivity(const u_int seasonID):season(seasonID){
        brls::Logger::debug("open season: {}", seasonID);
    }

    void onContentAvailable() override;

    void onSeasonVideoInfo(const bilibili::SeasonResultWrapper& result) override;

    void onSeasonEpisodeInfo(const bilibili::SeasonEpisodeResult& result) override;
private:
    u_int season;
    ChangeIndexEvent changeEpisodeEvent;
};