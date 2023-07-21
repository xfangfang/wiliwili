//
// Created by fang on 2022/8/4.
//

#pragma once

#include <borealis.hpp>
#include "view/mpv_core.hpp"
#include "presenter/live_data.hpp"

class VideoView;

class LiveActivity : public brls::Activity, public LiveDataRequest {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/video_activity.xml");

    explicit LiveActivity(const bilibili::LiveVideoResult& live);
    LiveActivity(int roomid, const std::string& name = "",
                 const std::string& views = "");

    void setCommonData();

    void setVideoQuality();

    void onContentAvailable() override;

    void onLiveData(const bilibili::LiveUrlResultWrapper& result) override;
    void onError(const std::string& error) override;

    std::vector<std::string> getQualityDescriptionList();
    int getCurrentQualityIndex();

    ~LiveActivity() override;

private:
    BRLS_BIND(VideoView, video, "fullscreen/video");

    bilibili::LiveVideoResult liveData;

    // 监控mpv事件
    MPVCustomEvent::Subscription eventSubscribeID;
};