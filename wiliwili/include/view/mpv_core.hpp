//
// Created by fang on 2022/8/12.
//

#pragma once

#include "borealis.hpp"
#include "borealis/core/singleton.hpp"
#include <mpv/client.h>
#include <mpv/render.h>
#ifndef MPV_SW_RENDER
#include <mpv/render_gl.h>
#include <glad/glad.h>
#ifdef __SDL2__
#include <SDL.h>
#else
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif
#include <nanovg_gl.h>
struct GLShader {
    GLuint prog;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};
#endif

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
    END_OF_FILE,
    DANMAKU_LOADED,
    CACHE_SPEED_CHANGE,
    QUALITY_CHANGED,
    QUALITY_CHANGE_REQUEST,
} MpvEventEnum;

typedef brls::Event<MpvEventEnum> MPVEvent;

class MPVCore : public brls::Singleton<MPVCore> {
public:
    MPVCore();

    ~MPVCore();

    void restart();

    void init();

    void clean();

    static void on_update(void *self);

    static void on_wakeup(void *self);

    void deleteFrameBuffer();

    void deleteShader();

    void initializeGL();

    void command_str(const char *args);

    void command(const char **args);

    void command_async(const char **args);

    int get_property(const char *name, mpv_format format, void *data);

    bool isStopped();

    bool isPaused();

    double getSpeed();

    double getPlaybackTime();

    std::string getCacheSpeed();

    void setUrl(const std::string &url, const std::string &extra = "");

    void resume();

    void pause();

    void stop();

    void setFrameSize(brls::Rect rect);

    bool isValid();

    void openglDraw(brls::Rect rect, float alpha = 1.0);

    mpv_render_context *getContext();

    mpv_handle *getHandle();

    MPVEvent *getEvent();

    void reset();

    // core states
    int core_idle          = 0;
    int64_t duration       = 0;  // second
    int64_t cache_speed    = 0;  // Bps
    double playback_time   = 0;
    double percent_pos     = 0;
    int64_t video_progress = 0;

    // 当前播放的清晰度
    std::string qualityStr;

    // Bottom progress bar
    inline static bool BOTTOM_BAR    = true;
    inline static bool LOW_QUALITY   = false;
    inline static int INMEMORY_CACHE = 0;

    inline static bool HARDWARE_DEC               = false;
    inline static std::string PLAYER_HWDEC_METHOD = "auto-safe";

    // 此变量为真时，加载结束后自动播放视频
    inline static bool AUTO_PLAY = true;

    NVGcolor bottomBarColor =
        brls::Application::getTheme().getColor("color/bilibili");

private:
    mpv_handle *mpv                 = nullptr;
    mpv_render_context *mpv_context = nullptr;
    brls::Rect reportRect;
    brls::RepeatingTimer reportTimer;
    void setFrameSizeInline();
#ifdef MPV_SW_RENDER
    const int PIXCEL_SIZE          = 4;
    int nvg_image                  = 0;
    const char *sw_format          = "rgba";
    int sw_size[2]                 = {1920, 1080};
    size_t pitch                   = PIXCEL_SIZE * sw_size[0];
    void *pixels                   = nullptr;
    mpv_render_param mpv_params[5] = {
        {MPV_RENDER_PARAM_SW_SIZE, &sw_size[0]},
        {MPV_RENDER_PARAM_SW_FORMAT, (void *)sw_format},
        {MPV_RENDER_PARAM_SW_STRIDE, &pitch},
        {MPV_RENDER_PARAM_SW_POINTER, pixels},
        { MPV_RENDER_PARAM_INVALID,
          nullptr }};
#elif defined(MPV_NO_FB)
    mpv_opengl_fbo mpv_fbo{0, 1920, 1080};
    int flip_y{1};
    mpv_render_param mpv_params[3] = {{MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
                                      {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
                                      { MPV_RENDER_PARAM_INVALID,
                                        nullptr }};
#else
    GLuint media_framebuffer = 0;
    GLuint media_texture     = 0;
    GLShader shader{0};
    mpv_opengl_fbo mpv_fbo{0, 1920, 1080};
    int flip_y{1};
    mpv_render_param mpv_params[3] = {{MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
                                      {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
                                      {MPV_RENDER_PARAM_INVALID, nullptr}};
    float vertices[20] = {1.0f, 1.0f,  0.0f, 1.0f,  1.0f,  1.0f, -1.0f,
                          0.0f, 1.0f,  0.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                          0.0f, -1.0f, 1.0f, 0.0f,  0.0f,  1.0f};
#endif

    MPVEvent mpvCoreEvent;

    // 当前软件是否在前台的回调
    brls::Event<bool>::Subscription focusSubscription;

    /// Will be called in main thread to get events from mpv core
    void eventMainLoop();
};
