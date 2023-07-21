//
// Created by fang on 2023/7/18.
//

#pragma once

#include <borealis/core/activity.hpp>
#include <libpdr.h>

#include "view/mpv_core.hpp"


class VideoView;
class DLNAActivity : public brls::Activity {
public:
    CONTENT_FROM_XML_RES("activity/video_activity.xml");

    DLNAActivity();

    void onContentAvailable() override;

    ~DLNAActivity() override;

private:
    BRLS_BIND(VideoView, video, "fullscreen/video");

    // 监控mpv事件
    MPVEvent::Subscription mpvEventSubscribeID;
    // 监控DLNA事件
    pdr::Subscription dlnaEventSubscribeID;
    std::shared_ptr<pdr::DLNA> dlna;

    std::string ip, uuid;
    size_t port;
};