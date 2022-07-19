//
// Created by fang on 2022/7/16.
//

#pragma once

#include <borealis.hpp>


class VideoView;

class SplashActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/splash_activity.xml");

    SplashActivity();

    void onContentAvailable() override;

    ~SplashActivity();

private:
//    BRLS_BIND(VideoView, video, "video/detail/video");
};