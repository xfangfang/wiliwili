//
// Created by fang on 2022/4/22.
//

#pragma once

#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <borealis.hpp>
#include <utility>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nanovg_gl.h>
#include "utils/singleton.hpp"

class VideoView;
class MPVCore;

struct GLShader {
    GLuint prog;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

static inline void check_error(int status)
{
    if (status < 0) {
        brls::Application::notify(mpv_error_string(status));
        brls::Logger::error(mpv_error_string(status));
    }
}

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
        if(this->fullscreen)
            return this;
        return Box::getNextFocus(direction, currentView);
    }

    void setFullScreen(bool fs){
        if (fs){
            this->setSize(brls::Size(brls::Application::contentWidth, brls::Application::contentHeight));
        }
        this->fullscreen = fs;
    }

    /// Video control
    void start(std::string url){
        brls::Logger::error("start mpv: {}", url);
        this->setUrl(url);
        this->resume();
    }

    void setUrl(std::string url);

    void resume();

    void pause();

    void stop();

    void togglePlay(){
        if (this->videoState != VideoState::PLAYING){
            this->resume();
        } else if(this->videoState != VideoState::PAUSED){
            this->pause();
        }
    }

    /// OSD
    void showOSD(){
        this->osdLastShowTime = unix_time() + VideoView::OSD_SHOW_TIME;
        brls::Logger::error("time: {}", osdLastShowTime);
    }

    void hideOSD(){
        this->osdLastShowTime = 0;
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

private:
    bool fullscreen = false;
    VideoState videoState = VideoState::STOPPED;


    ///OSD
    BRLS_BIND(brls::Label, videoTitleLabel, "video/osd/title");
    time_t osdLastShowTime = 0;
    const time_t OSD_SHOW_TIME = 5; //默认显示五秒
    MPVCore* mpvCore;
};
