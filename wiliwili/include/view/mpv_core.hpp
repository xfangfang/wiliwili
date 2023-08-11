//
// Created by fang on 2022/8/12.
//

#pragma once

#include <borealis.hpp>
#include <borealis/core/singleton.hpp>
#include <mpv/client.h>
#include <mpv/render.h>
#include <fmt/format.h>
#ifndef MPV_SW_RENDER
#include <mpv/render_gl.h>
#ifdef __PSV__
#include <GLES2/gl2.h>
#else
#include <glad/glad.h>
#endif
#ifdef __SDL2__
#include <SDL2/SDL.h>
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
    MPV_FILE_ERROR,
    LOADING_START,
    LOADING_END,
    UPDATE_DURATION,
    UPDATE_PROGRESS,
    START_FILE,
    END_OF_FILE,
    CACHE_SPEED_CHANGE,
    VIDEO_SPEED_CHANGE,
    VIDEO_VOLUME_CHANGE,
    VIDEO_MUTE,
    VIDEO_UNMUTE,
} MpvEventEnum;

typedef brls::Event<MpvEventEnum> MPVEvent;
typedef brls::Event<std::string, void *> MPVCustomEvent;
#define MPV_E MPVCore::instance().getEvent()
#define MPV_CE MPVCore::instance().getCustomEvent()

class MPVCore : public brls::Singleton<MPVCore> {
public:
    MPVCore();

    ~MPVCore();

    /// Get MPV States

    bool isStopped() const;

    bool isPaused() const;

    double getSpeed() const;

    double getPlaybackTime() const;

    std::string getCacheSpeed() const;

    int64_t getVolume() const;

    bool isValid();

    // todo: remove these sync function
    std::string getString(const std::string &key);
    double getDouble(const std::string &key);
    int64_t getInt(const std::string &key);
    std::unordered_map<std::string, mpv_node> getNodeMap(
        const std::string &key);

    /// Set MPV States

    /**
     * 设置播放链接
     * @param url  播放链接
     * @param extra 额外的参数，如 referrer、audio 等，详情见 mpv 文档
     * @param method 行为，默认为替换当前视频，详情见 mpv 文档
     */
    void setUrl(const std::string &url, const std::string &extra = "",
                const std::string &method = "replace");

    /**
     * 设置备用链接（可多次调用）
     * 内部使用 mpv 的播放列表实现。当前链接无法播放时，自动跳转到播放列表的下一项
     *
     * 注：如果是dash链接，同时存在备用的音频链接，可以将音频列表通过 extra 逐项传入，
     * 这样就能实现无论是音频还是视频，在播放失败时自动切换到备用链接
     *
     *
     * @param url 备用播放链接
     * @param extra 额外的参数，定义同上
     */
    void setBackupUrl(const std::string &url, const std::string &extra = "");

    void setVolume(int64_t value);
    void setVolume(const std::string& value);

    void resume();

    void pause();

    void stop();

    /**
     * 跳转视频
     * @param p 秒
     */
    void seek(int64_t p);
    void seek(const std::string& p);

    /**
     * 相对于当前播放的时间点跳转视频
     * @param p 秒
     */
    void seekRelative(int64_t p);

    /**
     * 跳转视频到指定百分比位置
     * @param value 0-1
     */
    void seekPercent(double value);

    /**
     * 设置视频播放速度
     * @param value 1.0 为元速
     */
    void setSpeed(double value);

    /**
     * 设置视频渲染区域大小
     * @param rect 视频区域
     */
    void setFrameSize(brls::Rect rect);

    void showOsdText(const std::string &value, int duration = 2000);

    /**
     * 禁用系统锁屏
     */
    static void disableDimming(bool disable);

    void openglDraw(brls::Rect rect, float alpha = 1.0);

    mpv_render_context *getContext();

    mpv_handle *getHandle();

    /**
     * 播放器内部事件
     * 传递内容为: 事件类型
     */
    MPVEvent *getEvent();

    /**
     * 可以用于共享自定义事件
     * 传递内容为: string类型的事件名与一个任意类型的指针
     */
    MPVCustomEvent *getCustomEvent();

    /**
     * 重启 MPV，用于某些需要重启才能设置的选项
     */
    void restart();

    void reset();

    void setShader(const std::string &profile, const std::string &shaders,
                   bool showHint = true);

    void clearShader(bool showHint = true);

    /// Send command to mpv
    template <typename... Args>
    void command_async(Args &&...args) {
        std::vector<std::string> commands = {
            fmt::format("{}", std::forward<Args>(args))...};

        std::vector<const char *> res;
        res.reserve(commands.size() + 1);
        for (auto &i : commands) {
            res.emplace_back(i.c_str());
        }
        res.emplace_back(nullptr);

        mpv_command_async(mpv, 0, res.data());
    }

    // core states
    int64_t duration       = 0;  // second
    int64_t cache_speed    = 0;  // Bps
    int64_t volume         = 100;
    double video_speed     = 0;
    bool video_paused      = false;
    bool video_stopped     = false;
    bool video_seeking     = false;
    double playback_time   = 0;
    double percent_pos     = 0;
    int64_t video_progress = 0;

    // 低画质解码，剔除解码过程中的部分步骤，可以用来节省cpu
    inline static bool LOW_QUALITY = false;

    // 视频缓存（是否使用内存缓存视频，值为缓存的大小，单位MB）
    inline static int INMEMORY_CACHE = 0;

    // 硬件解码
    inline static bool HARDWARE_DEC               = false;
    inline static std::string PLAYER_HWDEC_METHOD = "auto-safe";

    // 此变量为真时，加载结束后自动播放视频
    inline static bool AUTO_PLAY = true;

    // 若值大于0 则当前时间大于 CLOSE_TIME 时，自动暂停播放
    inline static size_t CLOSE_TIME = 0;

    // 触发倍速时的默认值，单位为 %
    inline static int VIDEO_SPEED = 200;

    // 默认的音量
    inline static int VIDEO_VOLUME = 100;

private:
    mpv_handle *mpv                 = nullptr;
    mpv_render_context *mpv_context = nullptr;
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

    // MPV 内部事件，传递内容为: 事件类型
    MPVEvent mpvCoreEvent;

    // 自定义的事件，传递内容为: string类型的事件名与一个任意类型的指针
    MPVCustomEvent mpvCoreCustomEvent;

    // 当前软件是否在前台的回调
    brls::Event<bool>::Subscription focusSubscription;

    /// Will be called in main thread to get events from mpv core
    void eventMainLoop();

    void deleteFrameBuffer();

    void deleteShader();

    void initializeGL();

    void init();

    void clean();

    /// MPV callbacks

    static void on_update(void *self);

    static void on_wakeup(void *self);
};
