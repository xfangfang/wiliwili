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
#include <memory>
#include <map>
#include <chrono>
#include <atomic>

class VideoView;
class UserInfoView;
class LiveDanmakuItemView; // 添加前向声明

class LiveActivity : public brls::Activity, 
                     public LiveDataRequest {
public:
    // Declare that the content of this activity is the given XML file
    // CONTENT_FROM_XML_RES("activity/live_player_activity.xml");
    brls ::View* createContentView() override {
        if (maxSidebarDanmakuCount != 0)
            return brls ::View ::createFromXMLResource("activity/live_player_activity.xml");
        else
            return brls ::View ::createFromXMLResource("activity/video_activity.xml");
    }

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
    
    // 处理接收到的超级留言(SC)，展示在侧边栏
    void processSuperChatForSidebar(const std::vector<LiveDanmakuItem>& sc_list);
    
    // 处理SC置顶相关
    // 添加SC置顶
    void addPinnedSuperChat(const LiveDanmakuItem& sc, const std::string& scToken);
    // 移除SC置顶
    void removePinnedSuperChat(const std::string& sc_token);
    // 检查SC是否置顶
    bool isPinnedSuperChat(const std::string& sc_token) const;
    // 定时器回调，检查SC过期
    void checkPinnedSuperChatExpiry();
    // 启动SC过期检查定时器
    void startSuperChatExpiryTimer();
    // 生成SC唯一标识
    std::string generateSuperChatToken(const LiveDanmakuItem& sc) const;

    // 添加方法检查是否应该显示侧边栏
    bool shouldShowSidebar() const {
        return maxSidebarDanmakuCount > 0;
    }

    ~LiveActivity() override;

private:
    VideoView* video = nullptr;
    UserInfoView* liveAuthor = nullptr;
    brls::Box* liveDanmakuContainer = nullptr;
    brls::ScrollingFrame* liveDanmakuList = nullptr;
    brls::Label* liveTitleLabel = nullptr;
    brls::Label* anchorTitleLabel = nullptr;
    brls::Box* liveDanmakuSidebar = nullptr;
    brls::Box* liveDetailLeftBox = nullptr;

    // 暂停的延时函数 handle
    size_t toggleDelayIter = 0;
    // 遇到错误重试的延时函数 handle
    size_t errorDelayIter = 0;
    // SC检查过期的延时函数 handle
    size_t scExpiryCheckIter = 0;

    LiveDanmaku danmaku;

    bilibili::LiveVideoResult liveData;
    std::string anchorTitle = ""; // 新增：主播称号
    
    // SC置顶管理
    // 键：SC的唯一标识，值：过期时间点
    std::map<std::string, std::chrono::time_point<std::chrono::system_clock>> pinnedSuperChats;
    // 保存SC视图项的引用，用于更新状态
    std::map<std::string, LiveDanmakuItemView*> pinnedSuperChatViews;
    
    // 侧边栏弹幕最大数量
    int maxSidebarDanmakuCount = 100;

    //更新timeLabel
    MPVEvent::Subscription tl_event_id;
    //视频清晰度
    CustomEvent::Subscription event_id;
    //弹幕事件
    CustomEvent::Subscription danmaku_event_id;
    
    //使用共享状态对象
    struct ThreadSafeState {
        std::atomic<bool> isActive{true};
    };
    std::shared_ptr<ThreadSafeState> threadState{std::make_shared<ThreadSafeState>()};
};