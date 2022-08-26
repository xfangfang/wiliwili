//
// Created by fang on 2022/8/4.
//

#pragma once

#include <borealis.hpp>
#include "bilibili.h"
#include "bilibili/result/home_live_result.h"

class VideoView;

class LiveActivity : public brls::Activity {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/live_player_activity.xml");

    LiveActivity(const bilibili::LiveVideoResult& live);

    void onContentAvailable() override;

    ~LiveActivity();

private:
    BRLS_BIND(VideoView, video, "live/video");

    bilibili::LiveVideoResult liveData;
};