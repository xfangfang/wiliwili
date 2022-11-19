//
// Created by fang on 2022/8/12.
//

#pragma once

#include "borealis.hpp"
#include "utils/singleton.hpp"
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <nanovg_gl.h>

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
    END_OF_FILE,
    DANMAKU_LOADED,
    CACHE_SPEED_CHANGE,
} MpvEventEnum;

typedef brls::Event<MpvEventEnum> MPVEvent;

class DanmakuItem {
public:
    DanmakuItem(const std::string &content, const char *attributes);

    std::string msg;  // 弹幕内容
    float time;       // 弹幕出现的时间
    int type;         // 弹幕类型 1/2/3: 普通; 4: 底部; 5: 顶部;
    int fontSize;     // 弹幕字号 18/25/36
    int fontColor;    // 弹幕颜色

    bool isShown         = false;
    bool showing         = false;
    bool canShow         = true;
    float length         = 0;
    int line             = 0;  // 弹幕在屏幕上的行数
    float speed          = 0;
    int64_t startTime    = 0;
    NVGcolor color       = nvgRGBA(255, 255, 255, 160);
    NVGcolor borderColor = nvgRGBA(0, 0, 0, 160);
    //    int pubDate; // 弹幕发送时间
    //    int pool; // 弹幕池类型
    //    char hash[9] = {0};
    //    uint64_t dmid; // 弹幕ID
    //    int level; // 弹幕等级 0-10

    inline static std::vector<std::pair<float, float>> lines =
        std::vector<std::pair<float, float>>(20, {0, 0});

    inline static std::vector<float> centerLines = std::vector<float>(20, {0});

    bool operator<(const DanmakuItem &item) const {
        return this->time < item.time;
    }
};

class MPVCore : public Singleton<MPVCore> {
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

    int get_property(const char *name, mpv_format format, void *data) {
        return mpv_get_property(mpv, name, format, data);
    }

    bool isStopped() {
        int ret = 1;
        get_property("playback-abort", MPV_FORMAT_FLAG, &ret);
        return ret == 1;
    }

    bool isPaused() {
        int ret = -1;
        get_property("pause", MPV_FORMAT_FLAG, &ret);
        return ret == 1;
    }

    double getPlaybackTime() {
        get_property("pause", MPV_FORMAT_DOUBLE, &this->playback_time);
        return this->playback_time;
    }

    std::string getCacheSpeed();

    void setUrl(std::string url) {
        const char *cmd[] = {"loadfile", url.c_str(), NULL};
        command_async(cmd);
    }

    void resume() { command_str("set pause no"); }

    void pause() { command_str("set pause yes"); }

    void stop() {
        const char *cmd[] = {"stop", NULL};
        command_async(cmd);
    }

    void setFrameSize(brls::Rect rect);

    bool isValid();

    void openglDraw(brls::Rect rect, float alpha = 1.0);

    mpv_render_context *getContext();

    mpv_handle *getHandle();

    MPVEvent *getEvent();

    void loadDanmakuData(const std::vector<DanmakuItem> &data);

    void reset();

    void resetDanmakuPosition();

    std::vector<DanmakuItem> getDanmakuData();

    // core states
    int core_idle          = 0;
    int64_t duration       = 0;  // second
    int64_t cache_speed    = 0;  // Bps
    double playback_time   = 0;
    double percent_pos     = 0;
    int64_t video_progress = 0;
    std::mutex danmakuMutex;
    bool showDanmaku    = false;
    bool danmakuLoaded  = false;
    size_t danmakuIndex = 0;  // 当前显示的第一条弹幕序号
    std::vector<DanmakuItem> danmakuData;

    // Bottom progress bar
    inline static bool BOTTOM_BAR    = true;
    inline static bool LOW_QUALITY   = false;
    inline static int INMEMORY_CACHE = 0;
    NVGcolor bottomBarColor =
        brls::Application::getTheme().getColor("color/bilibili");

private:
    mpv_handle *mpv                 = nullptr;
    mpv_render_context *mpv_context = nullptr;

    GLuint media_framebuffer = 0;
    GLuint media_texture     = 0;

    GLShader shader{0};
    mpv_opengl_fbo mpv_fbo{0, 1920, 1080};
    int flip_y{1};
    mpv_render_param mpv_params[3] = {{MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
                                      {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
                                      {MPV_RENDER_PARAM_INVALID, 0}};

    MPVEvent mpvCoreEvent;

    float vertices[20] = {1.0f, 1.0f,  0.0f, 1.0f,  1.0f,  1.0f, -1.0f,
                          0.0f, 1.0f,  0.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                          0.0f, -1.0f, 1.0f, 0.0f,  0.0f,  1.0f};

    /// Will be called in main thread to get events from mpv core
    void eventMainLoop();
};
