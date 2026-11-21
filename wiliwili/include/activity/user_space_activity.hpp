//
// Created by fang on 2024/12/21.
//

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>
#include <borealis/views/applet_frame.hpp>

#include "presenter/video_detail.hpp"
#include "view/recycling_grid.hpp"
#include "view/auto_tab_frame.hpp"
#include "view/user_info.hpp"

namespace brls {
class Label;
class Image;
}  // namespace brls

class SVGImage;

class UserSpaceActivity : public brls::Activity, public VideoDetail {
public:
    explicit UserSpaceActivity(uint64_t mid);

    void onContentAvailable() override;

    void onUpInfo(const bilibili::UserDetailResultWrapper& result) override;

    void onUploadedVideos(const bilibili::UserUploadedVideoResultWrapper& result) override;

    void onError(const std::string& error) override;

    brls::View* createContentView() override;

private:
    uint64_t user_mid;

    BRLS_BIND(brls::AppletFrame, appletFrame, "user_space/applet_frame");
    BRLS_BIND(brls::Box, headerBox, "user_space/header");
    BRLS_BIND(brls::Image, userAvatar, "user_space/avatar");
    BRLS_BIND(brls::Label, userName, "user_space/name");
    BRLS_BIND(brls::Label, userSign, "user_space/sign");
    BRLS_BIND(brls::Label, userFollowing, "user_space/following");
    BRLS_BIND(brls::Label, userFollower, "user_space/follower");
    BRLS_BIND(brls::Box, btnFollowBox, "user_space/follow_box");
    BRLS_BIND(brls::Label, btnFollowLabel, "user_space/follow_label");
    BRLS_BIND(SVGImage, btnFollowIcon, "user_space/follow_icon");
    BRLS_BIND(AutoTabFrame, tabFrame, "user_space/tab_frame");
    BRLS_BIND(RecyclingGrid, recyclingGrid, "user_space/recycling_grid");

    // State
    bool isFollowing = false;
    unsigned int uploadPage = 1;
    bool hasMoreUploads = true;
};
