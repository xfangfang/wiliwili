//
// Created by fang on 2022/8/12.
//

#pragma once

#include "utils/singleton.hpp"
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nanovg_gl.h>
#include "borealis.hpp"

struct GLShader {
    GLuint prog;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

typedef enum MpvEventEnum {
    MPV_LOADED,
    MPV_PAUSE,
    MPV_RESUME,
    MPV_STOP,
    LOADING_START,
    LOADING_END,
    UPDATE_DURATION,
    UPDATE_PROGRESS,
    START_FILE,
} MpvEventEnum;

typedef brls::Event<MpvEventEnum> MPVEvent;

class MPVCore: public Singleton<MPVCore>{
public:

    MPVCore();

    ~MPVCore();
    
    static void on_update(void *self);
    
    static void on_wakeup(void *self);
    
    void deleteFrameBuffer();
    
    void deleteShader();
    
    void initializeGL();
    
    void command_str(const char *args);
    
    void command(const char **args);
    
    void command_async(const char **args);

    int get_property(const char *name, mpv_format format, void *data){
        return mpv_get_property(mpv, name, format, data);
    }
    
    bool isStopped(){
        int ret = 1;
        get_property("playback-abort", MPV_FORMAT_FLAG, &ret);
        return ret == 1;
    }

    bool isPaused(){
        int ret = -1;
        get_property("pause", MPV_FORMAT_FLAG, &ret);
        return ret == 1;
    }

    void setUrl(std::string url){
        const char *cmd[] = {"loadfile", url.c_str(), NULL};
        command_async(cmd);
    }

    void resume(){
        command_str("set pause no");
    }

    void pause(){
        command_str("set pause yes");
    }

    void stop(){
        const char *cmd[] = {"stop",  NULL};
        command_async(cmd);
    }

    void setFrameSize(int drawWidth, int drawHeight);
    
    bool isValid();
    
    void openglDraw(brls::Rect rect, float alpha=1.0);
    
    mpv_render_context* getContext();
    
    mpv_handle* getHandle();

    MPVEvent* getEvent();

    // core states
    int core_idle = 0;
    int64_t duration = 0; // second
    int64_t cache_speed = 0; // Bps
    double playback_time = 0;
    int64_t video_progress = 0;

private:
    mpv_handle* mpv = nullptr;
    mpv_render_context* mpv_context = nullptr;

    GLuint media_framebuffer = 0;
    GLuint media_texture = 0;

    GLShader shader{0};
    mpv_opengl_fbo mpv_fbo{
            0,
            1920,
            1080
    };
    int flip_y{1};
    mpv_render_param mpv_params[3] = {
            {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
            {MPV_RENDER_PARAM_FLIP_Y,       &flip_y},
            {MPV_RENDER_PARAM_INVALID,      0}
    };

    MPVEvent mpvCoreEvent;

    /// Will be called in main thread to get events from mpv core
    void eventMainLoop();
};
