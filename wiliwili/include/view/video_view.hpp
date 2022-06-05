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
#ifdef __SWITCH__
#include <switch.h>
#endif

using namespace brls;

class VideoView;

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

class VideoView : public Box {
public:
    VideoView();

    ~VideoView() override;

    static void on_update(void *view){
//        Logger::debug("==> maybe updated");
    }

    void initializeGL();


    void draw(NVGcontext* vg, float x, float y, float width, float height, Style style, FrameContext* ctx) override;


    View* getDefaultFocus() override{
        return this;
    }

    View* getNextFocus(FocusDirection direction, View* currentView) override{
        if(this->fullscreen)
            return this;
        return Box::getNextFocus(direction, currentView);
    }

    void setFullScreen(bool fs){
        if (fs){
            this->setSize(Size(Application::contentWidth, Application::contentHeight));
        }
        this->fullscreen = fs;
    }

    /// Video control
    void start(std::string url){
        Logger::error("start mpv: {}", url);
        this->setUrl(url);
        this->resume();
    }

    void setUrl(std::string url){
        const char *cmd[] = {"loadfile", url.c_str(), "replace", NULL};
        check_error(mpv_command(mpv, cmd));
    }

    void resume(){
        //todo: 此处设置为 loading
        this->videoState = VideoState::PLAYING;
        check_error(mpv_command_string(mpv,"set pause no"));
#ifdef __SWITCH__
        appletSetMediaPlaybackState(true);
#endif
        brls::Logger::debug("resume");
    }

    void pause(){
        this->videoState = VideoState::PAUSED;
        check_error(mpv_command_string(mpv,"set pause yes"));
#ifdef __SWITCH__
        appletSetMediaPlaybackState(false);
#endif
        brls::Logger::debug("pause");
    }

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
        Logger::error("time: {}", osdLastShowTime);
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

    void deleteFrameBuffer();

    void deleteShader();

    void invalidate() override;

private:
    bool fullscreen = false;
    mpv_handle* mpv = nullptr;
    mpv_render_context* mpv_context = nullptr;
    VideoState videoState = VideoState::STOPPED;

    GLuint media_framebuffer = 0;
    GLuint media_renderbuffer = 0;
    GLuint media_texture = 0;

    GLShader shader{0};
    mpv_opengl_fbo mpv_fbo{
            0,
            1920,
            1080,
            GL_RGBA8
    };
    int flip_y{1};
    mpv_render_param mpv_params[3] = {
            {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
            {MPV_RENDER_PARAM_FLIP_Y,       &flip_y},
            {MPV_RENDER_PARAM_INVALID,      0}
    };

    ///OSD
    BRLS_BIND(Label, videoTitleLabel, "video/osd/title");
    time_t osdLastShowTime = 0;
    const time_t OSD_SHOW_TIME = 5; //默认显示五秒
};
