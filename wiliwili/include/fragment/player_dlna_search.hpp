//
// Created by fang on 2023/5/7.
//

// register this fragment in main.cpp
//#include "fragment/player_dlna_search.hpp"
//    brls::Application::registerXMLView("PlayerDlnaSearch", PlayerDlnaSearch::create);
// <brls:View xml=@res/xml/fragment/player_dlna_search.xml/>

#pragma once

#include <atomic>
#include <thread>
#include <borealis/core/timer.hpp>
#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

#include "dlna/dlna.h"
#include "utils/event_helper.hpp"

class ButtonClose;
namespace brls {
class ScrollingFrame;
class RadioCell;
}  // namespace brls

class RepeatDuratoinTimer : public brls::RepeatingTimer {
public:
    // 指定时间后停止重复
    void setDuration(brls::Time value);

    void start(brls::Time value);

    void start();

    bool onUpdate(brls::Time delta) override;

    void setCallback(const std::function<void(brls::Time)>& cb);

private:
    brls::Time startTime;
    brls::Time duration                              = -1;
    int cycleTimes                                   = 0;
    std::function<void(brls::Time)> durationCallback = [](brls::Time) {};
};

class PlayerDlnaSearch : public brls::Box {
public:
    PlayerDlnaSearch();

    bool isTranslucent() override;

    ~PlayerDlnaSearch() override;

    void refreshRenderer();

    void searchStart();

    void searchStop();

    static bool isRunning();

    const int TIMEOUT = 3;

    // 是否正在搜索中
    inline static std::atomic<bool> running = false;

    // 是否正在等待返回播放链接
    inline static std::atomic<bool> waitingUrl = false;

    // 是否正在等待投屏结果
    inline static std::atomic<bool> waitingRenderer = false;

private:
    BRLS_BIND(ButtonClose, closebtn, "button/close");
    BRLS_BIND(brls::ScrollingFrame, settings, "player/dlna");
    BRLS_BIND(brls::RadioCell, btnRefresh, "setting/dlna/search");
    BRLS_BIND(brls::Box, deviceBox, "setting/dlna/device/box");
    BRLS_BIND(brls::Box, cancel, "player/cancel");

    RepeatDuratoinTimer searchCounter;
    DlnaRenderer currentRenderer;
    brls::RadioCell* currentCell = nullptr;
    CustomEvent::Subscription customEventSubscribeID;

    inline static std::thread dlnaSearchThread;
};