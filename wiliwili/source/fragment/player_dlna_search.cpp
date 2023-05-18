//
// Created by fang on 2023/5/7.
//
#include <chrono>

#include "fragment/player_dlna_search.hpp"
#include "view/button_close.hpp"
#include "utils/dialog_helper.hpp"
#include "bilibili/result/video_detail_result.h"

using namespace brls::literals;

void RepeatDuratoinTimer::setDuration(brls::Time value) {
    duration = value;
}

void RepeatDuratoinTimer::start(brls::Time value) {
    period = period;
    start();
}

void RepeatDuratoinTimer::start(){
    startTime = brls::getCPUTimeUsec();
    cycleTimes = 0;
    Ticking::start();
}

void RepeatDuratoinTimer::setCallback(const std::function<void(brls::Time)>& cb) {
    durationCallback = cb;
}

bool RepeatDuratoinTimer::onUpdate(brls::Time delta)
{
    this->progress += delta;
    if (this->progress >= this->period)
    {
        this->durationCallback(cycleTimes++);
        this->progress = 0;
        if (duration <= 0) return true;

        brls::Time now = brls::getCPUTimeUsec();
        if (now - startTime > duration * 1000) {
            return false;
        }
    }

    return true;
}

void PlayerDlnaSearch::searchStop() {
    running = false;
    if (dlnaSearchThread.joinable()) dlnaSearchThread.join();
    brls::Logger::debug("PlayerDlnaSearch::searchStop()");
}

void PlayerDlnaSearch::searchStart() {
    if (running || waitingUrl || waitingRenderer) {
        brls::Logger::error("DLNA searching is already running");
    }
    running = true;
    if (dlnaSearchThread.joinable()) dlnaSearchThread.join();
    ASYNC_RETAIN
    dlnaSearchThread = std::thread([ASYNC_TOKEN](){
        auto list = UpnpDlna::instance().searchRenderer(TIMEOUT * 1000);
        brls::sync([ASYNC_TOKEN, list](){
            ASYNC_RELEASE
            brls::Logger::debug("got renderer: {}", list.size());
            for(auto i : list) {
                i.print();
                auto* l = new brls::RadioCell();
                l->title->setText(i.friendlyName);
                deviceBox->addView(l);
                l->registerClickAction([this, i, l](...) {
                    if (running || waitingUrl || waitingRenderer) return true;
                    waitingUrl = true;
                    currentCell = l;
                    currentRenderer = i;
                    l->title->setText(i.friendlyName + " " + "wiliwili/player/cast/request_url"_i18n);
                    MPV_CE->fire("REQUEST_CAST_URL", nullptr);
                    return true;
                });
            }
            btnRefresh->title->setText("wiliwili/home/common/refresh"_i18n);
            btnRefresh->title->setTextColor(brls::Application::getTheme().getColor("brls/text"));
            running = false;
        });
    });
}


PlayerDlnaSearch::PlayerDlnaSearch() {
    this->inflateFromXMLRes("xml/fragment/player_dlna_search.xml");
    brls::Logger::debug("Fragment PlayerDlnaSearch: create");

    this->registerAction("hints/cancel"_i18n, brls::BUTTON_B, [](...) {
        brls::Application::popActivity();
        return true;
    });

    closebtn->registerClickAction([](...) {
        if (running || waitingUrl || waitingRenderer) return true;
        brls::Application::popActivity();
        return true;
    });

    btnRefresh->title->setText("wiliwili/home/common/refresh"_i18n);
    btnRefresh->registerClickAction([this](...){
        this->refreshRenderer();
       return true;
    });
    btnRefresh->addGestureRecognizer(new brls::TapGestureRecognizer(this));

    this->registerAction("hints/back"_i18n, brls::BUTTON_B,
                         [](View* view) {
                             if (running || waitingUrl || waitingRenderer) return true;
                             brls::Application::popActivity();
                             return true;
                         });

    customEventSubscribeID = MPV_CE->subscribe([this](const std::string& event, void* data){
        if (event == "CAST_URL") {
            waitingUrl = false;
            waitingRenderer = true;
            if (currentCell) currentCell->title->setText(currentRenderer.friendlyName + " " + "wiliwili/player/cast/cast_to_renderer"_i18n);
            auto* castData = (bilibili::VideoCastData*)data;
            ASYNC_RETAIN
            currentRenderer.play(castData->url, castData->title, [ASYNC_TOKEN]() {
                    ASYNC_RELEASE
                    waitingRenderer = false;
                    auto renderer   = currentRenderer;
                    brls::Application::popActivity(
                        brls::TransitionAnimation::FADE, [renderer]() {
                            auto dialog = new brls::Dialog(
                                renderer.friendlyName + " " +
                                "wiliwili/player/cast/casting"_i18n);
                            dialog->setCancelable(false);
                            dialog->addButton(
                                "wiliwili/player/cast/cancel"_i18n,
                                [renderer]() {
                                    renderer.stop(
                                        []() {},
                                        []() {
                                            DialogHelper::showDialog(
                                                "wiliwili/player/cast/err_connect"_i18n);
                                        });
                                });
                            dialog->open();
                        });
                }, [ASYNC_TOKEN](){
                    ASYNC_RELEASE
                    DialogHelper::showDialog("wiliwili/player/cast/err_connect"_i18n);
                    waitingRenderer = false;
                    if (currentCell) currentCell->title->setText(currentRenderer.friendlyName);
            });
        } else if (event == "CAST_URL_ERROR") {
            waitingUrl = false;
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
            DialogHelper::showDialog("wiliwili/player/cast/err_url"_i18n);
            if (currentCell) currentCell->title->setText(currentRenderer.friendlyName);
        }
    });

    this->refreshRenderer();
}

void PlayerDlnaSearch::refreshRenderer() {
    if (running || waitingUrl || waitingRenderer) return;
    btnRefresh->title->setTextColor(brls::Application::getTheme().getColor("brls/text_disabled"));
    searchCounter.setDuration(TIMEOUT * 1000);
    searchCounter.setCallback([this](int cycleTimes){
        this->btnRefresh->title->setText("wiliwili/player/cast/countdown"_i18n + fmt::format(": {:.1f}s", TIMEOUT * 1.0 - cycleTimes * 0.1));
    });
    searchCounter.setEndCallback([this](bool finished){
        btnRefresh->title->setTextColor(brls::Application::getTheme().getColor("brls/text"));
        btnRefresh->title->setText("wiliwili/home/common/refresh"_i18n);
    });
    searchCounter.setPeriod(100);
    searchCounter.start();
    deviceBox->clearViews();
    currentRenderer.setValid(false);
    currentCell = nullptr;
    PlayerDlnaSearch::searchStart();
}

PlayerDlnaSearch::~PlayerDlnaSearch() {
    brls::Logger::debug("Fragment PlayerDlnaSearch: delete");
    MPV_CE->unsubscribe(customEventSubscribeID);
    searchStop();
}

bool PlayerDlnaSearch::isTranslucent() { return true; }