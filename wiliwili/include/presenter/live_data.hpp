//
// Created by fang on 2022/9/13.
//

#pragma once

#include "bilibili/result/home_live_result.h"
#include "bilibili/result/live_danmaku_result.h"
#include "presenter/presenter.h"

class LiveDataRequest : public Presenter {
public:
    virtual void onLiveData(const bilibili::LiveRoomPlayInfo& result) {}

    virtual void onError(const std::string& error) {}

    virtual void onNeedPay(const std::string& msg, const std::string& link, const std::string& startTime,
                           const std::string& endTime) {}

    virtual void onDanmakuInfo(int roomid, const bilibili::LiveDanmakuinfo& info) {}

    void requestData(int roomid);

    void reportHistory(int roomid);

    void requestPayLiveInfo(int roomid);

    void requestLiveDanmakuToken(int roomid);

    std::string getQualityDescription(int qn);

    static inline int defaultQuality = 0;
    bilibili::LiveRoomPlayInfo liveRoomPlayInfo{};
    bilibili::LiveStreamFormatCodec liveUrl{};
    std::unordered_map<int, std::string> qualityDescriptionMap{};
};