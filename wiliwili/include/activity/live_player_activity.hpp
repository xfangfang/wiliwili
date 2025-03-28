//
// Created by fang on 2022/8/4.
//

#pragma once

#include <borealis/core/activity.hpp>
#include <borealis/core/bind.hpp>
#include <borealis/views/scrolling_frame.hpp>
#include <borealis/core/box.hpp>
#include <borealis/views/label.hpp>

#include "utils/event_helper.hpp"
#include "presenter/live_data.hpp"
#include "live/danmaku_live.hpp"
#include "view/live_core.hpp"

class VideoView;
class UserInfoView;

class LiveActivity : public brls::Activity, public LiveDataRequest {
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/live_player_activity.xml");

    explicit LiveActivity(int roomid, const std::string& name = "", const std::string& views = "");

    void setCommonData();

    void setVideoQuality();

    void onContentAvailable() override;

    void onLiveData(const bilibili::LiveRoomPlayInfo& result) override;

    void onError(const std::string& error) override;

    void onNeedPay(const std::string& msg, const std::string& link, const std::string& startTime,
                   const std::string& endTime) override;

    void onDanmakuInfo(int roomid, const bilibili::LiveDanmakuinfo& info) override;

    // 添加主播信息回调函数实现
    void onAnchorInfo(const std::string& face, const std::string& uname) override;

    // 新增：处理主播称号信息
    void onAnchorTitleInfo(const std::string& title) override;

    std::vector<std::string> getQualityDescriptionList();
    int getCurrentQualityIndex();

    void retryRequestData();
    
    // 处理接收到的弹幕，展示在侧边栏
    void processDanmakuForSidebar(const std::vector<LiveDanmakuItem>& danmaku_list);

    ~LiveActivity() override;

private:
    BRLS_BIND(VideoView, video, "video");
    BRLS_BIND(UserInfoView, liveAuthor, "live_author");
    BRLS_BIND(brls::Box, liveDanmakuContainer, "live_danmaku_container");
    BRLS_BIND(brls::ScrollingFrame, liveDanmakuList, "live_danmaku_list");
    BRLS_BIND(brls::Label, liveTitleLabel, "live/title");
    // 新增：主播称号Label
    BRLS_BIND(brls::Label, anchorTitleLabel, "anchor/title");

    // 暂停的延时函数 handle
    size_t toggleDelayIter = 0;
    // 遇到错误重试的延时函数 handle
    size_t errorDelayIter = 0;

    LiveDanmaku danmaku;

    bilibili::LiveVideoResult liveData;
    std::string anchorTitle = ""; // 新增：主播称号

    //更新timeLabel
    MPVEvent::Subscription tl_event_id;
    //视频清晰度
    CustomEvent::Subscription event_id;
    //弹幕事件
    CustomEvent::Subscription danmaku_event_id;
};