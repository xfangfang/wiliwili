//
// Created by fang on 2022/4/22.
//

#pragma once

#include <borealis.hpp>
#include "presenter/video_detail.hpp"

#include "view/video_comment.hpp"
#include "view/recycling_grid.hpp"

class VideoView;
class UserInfoView;

class PlayerActivity : public brls::Activity, public VideoDetail
{
public:
    CONTENT_FROM_XML_RES("activity/player_activity.xml");

    PlayerActivity(bilibili::Video video);

    PlayerActivity(std::string bvid);

    void onContentAvailable() override;

    void setFullscreen();

    void exitFullscreen();

    void onVideoInfo(const bilibili::VideoDetailResult &result) override;
    void onVideoPageListInfo(const bilibili::VideoDetailPageListResult &result) override;
    void onVideoPlayUrl(const bilibili::VideoUrlResult & result) override;
    void onCommentInfo(const bilibili::VideoCommentResultWrapper &result) override;

    ~PlayerActivity() override;

private:
    BRLS_BIND(VideoView, video, "video/detail/video");
    BRLS_BIND(brls::AppletFrame, appletFrame, "video/detail/frame");
    BRLS_BIND(UserInfoView, videoUserInfo, "video_author");
    BRLS_BIND(brls::Label, videoTitleLabel, "video_title");
    BRLS_BIND(brls::Label, videoIntroLabel, "video_intro");
    BRLS_BIND(brls::Label, videoInfoLabel, "video_info");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "video/comment/recyclingGrid");



    bilibili::Video video_data;
    bool fullscreen = false;

    brls::ActionIdentifier videoExitFullscreenID = -1;
};
