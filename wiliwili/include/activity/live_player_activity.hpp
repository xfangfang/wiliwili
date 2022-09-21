//
// Created by fang on 2022/8/4.
//

#pragma once

#include <borealis.hpp>
#include "presenter/live_data.hpp"

class VideoView;

class LiveActivity : public brls::Activity, public LiveDataRequest {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/live_player_activity.xml");

    LiveActivity(const bilibili::LiveVideoResult& live);
    LiveActivity(int roomid);

    void onContentAvailable() override;

    void onLiveData(const bilibili::LiveUrlResultWrapper& result) override;
    void onError(const std::string& error) override;

    ~LiveActivity();

private:
    BRLS_BIND(VideoView, video, "live/video");

    bilibili::LiveVideoResult liveData;

    // 用于缓存全局状态，进入直播时关闭弹幕与底部进度条，退出时恢复
    bool globalShowDanmaku = false;
    bool globalBottomBar = false;
};