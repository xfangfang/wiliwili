//
// Created by fang on 2022/4/22.
//

#pragma once

#include <borealis.hpp>
#include "view/mpv_core.hpp"

class VideoProgressSlider;

static inline time_t unix_time(){
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

typedef enum VideoState{
    PLAYING,
    STOPPED,
    LOADING,
    PAUSED,
} VideoState;

class VideoView : public brls::Box{
public:
    VideoView();

    ~VideoView() override;

    void initializeGL();

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style, brls::FrameContext* ctx) override;

    View* getDefaultFocus() override{
        return this;
    }

    View* getNextFocus(brls::FocusDirection direction, View* currentView) override{
        if(this->isFullscreen())
            return this;
        return Box::getNextFocus(direction, currentView);
    }

    void setFullScreen(bool fs){
        if(!allowFullscreen){
            brls::Logger::error("Not being allowed to set fullscreen");
            return;
        }

        if(fs == isFullscreen()){
            brls::Logger::error("Already set fullscreen state to: {}", fs);
            return;
        }

        brls::Logger::info("VideoView set fullscreen state: {}", fs);
        if (fs){
            auto container = new brls::Box();
            auto video = new VideoView();
            video->setDimensions(1280, 720);
            container->addView(video);
            brls::Application::pushActivity(new brls::Activity(container));
        } else {
            brls::Application::popActivity();
        }
    }

    /// Video control
    void start(std::string url){
        brls::Logger::error("start mpv: {}", url);
        this->setUrl(url);
        brls::Logger::debug("set url to mpv done");
    }

    bool isFullscreen(){
        auto rect = this->getFrame();
        return rect.getHeight() == brls::Application::contentHeight && \
            rect.getWidth() == brls::Application::contentWidth;
    }

    void setUrl(std::string url);

    void resume();

    void pause();

    void stop();

    void togglePlay(){
        if (this->mpvCore->isPaused()){
            this->resume();
        } else {
            this->pause();
        }
    }

    /// OSD
    void showOSD(bool temp = true){
        if(temp)
            this->osdLastShowTime = unix_time() + VideoView::OSD_SHOW_TIME;
        else
            this->osdLastShowTime = 0xffffffff;
    }

    void hideOSD(){
        this->osdLastShowTime = 0;
    }

    bool isOSDShown(){
        return unix_time() < this->osdLastShowTime;
    }

    // Loading
    void showLoading(){
        osdSpinner->setVisibility(brls::Visibility::VISIBLE);
    }

    void hideLoading(){
        osdSpinner->setVisibility(brls::Visibility::GONE);
    }

    void setTitle(std::string title){
        brls::Threading::sync([this, title](){
            this->videoTitleLabel->setText(title);
        });
    }

    std::string getTitle(){
        return this->videoTitleLabel->getFullText();
    }

    static View* create()
    {
        return new VideoView();
    }

    void invalidate() override;
    void onLayout() override;

private:
    bool allowFullscreen = true;
    VideoState videoState = VideoState::STOPPED;

    MPVEvent::Subscription eventSubscribeID;

    ///OSD
    BRLS_BIND(brls::Label, videoTitleLabel, "video/osd/title");
    BRLS_BIND(brls::Box, osdTopBox, "video/osd/top/box");
    BRLS_BIND(brls::Box, osdBottomBox, "video/osd/bottom/box");
    BRLS_BIND(brls::ProgressSpinner, osdSpinner, "video/osd/loading");
    BRLS_BIND(VideoProgressSlider, osdSlider, "video/osd/bottom/progress");
    BRLS_BIND(brls::Label, leftStatusLabel, "video/left/status");
    BRLS_BIND(brls::Label, rightStatusLabel, "video/right/status");
    BRLS_BIND(brls::Image, btnToggle, "video/osd/toggle");
    BRLS_BIND(brls::Box, btnFullscreen, "video/osd/fullscreen");


    time_t osdLastShowTime = 0;
    const time_t OSD_SHOW_TIME = 5; //默认显示五秒
    MPVCore* mpvCore;

    //DEBUG
    BRLS_BIND(brls::Box, videoLayerDebug, "video/layer/debug");
    BRLS_BIND(brls::Box, videoLayerDanmaku, "video/layer/danmaku");
};
