//
// Created by fang on 2022/8/12.
//

#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdlib>
#include <fmt/format.h>
#include <borealis/core/geometry.hpp>
#include <borealis/core/singleton.hpp>
#include <borealis/core/logger.hpp>
#include <mpv/client.h>
#include <mpv/render.h>
#if defined(MPV_SW_RENDER)
#elif defined(BOREALIS_USE_DEKO3D)
#include <mpv/render_dk3d.h>
#elif defined(BOREALIS_USE_D3D11)
#include <mpv/render_dxgi.h>
#elif defined(BOREALIS_USE_OPENGL)
#include <mpv/render_gl.h>
#if defined(__PSV__) || defined(PS4)
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
#if defined(USE_GL2)
// 对于 OpenGL 2.0 平台，不支持独立创建 framebuffer
#define MPV_NO_FB
#endif
#if !defined(MPV_NO_FB) && !defined(MPV_SW_RENDER) && !defined(USE_GL2)
// 将视频绘制到独立的 framebuffer
#define MPV_USE_FB
#if !defined(USE_GLES2) && !defined(USE_GLES3)
// 虽然 gles3 理论上也支持 vao 但是部分平台上实际不支持（比如 ANGLE）
#define MPV_USE_VAO
#endif
struct GLShader {
    GLuint prog;
    GLuint vbo;
    GLuint ebo;
#ifdef MPV_USE_VAO
    GLuint vao;
#endif
};
#endif
#endif

#include "utils/event_helper.hpp"

#ifdef MPV_BUNDLE_DLL
#include <MemoryModule.h>
typedef int (*mpvSetOptionStringFunc)(mpv_handle *ctx, const char *name, const char *data);
typedef int (*mpvObservePropertyFunc)(mpv_handle *mpv, uint64_t reply_userdata, const char *name, mpv_format format);
typedef mpv_handle *(*mpvCreateFunc)(void);
typedef int (*mpvInitializeFunc)(mpv_handle *ctx);
typedef void (*mpvTerminateDestroyFunc)(mpv_handle *ctx);
typedef void (*mpvSetWakeupCallbackFunc)(mpv_handle *ctx, void (*cb)(void *d), void *d);
typedef int (*mpvCommandStringFunc)(mpv_handle *ctx, const char *args);
typedef const char *(*mpvErrorStringFunc)(int error);
typedef mpv_event *(*mpvWaitEventFunc)(mpv_handle *ctx, double timeout);
typedef int (*mpvGetPropertyFunc)(mpv_handle *ctx, const char *name, mpv_format format, void *data);
typedef int (*mpvCommandAsyncFunc)(mpv_handle *ctx, uint64_t reply_userdata, const char **args);
typedef char *(*mpvGetPropertyStringFunc)(mpv_handle *ctx, const char *name);
typedef void (*mpvFreeNodeContentsFunc)(mpv_node *node);
typedef int (*mpvSetOptionFunc)(mpv_handle *ctx, const char *name, mpv_format format, void *data);
typedef void (*mpvFreeFunc)(void *data);
typedef int (*mpvRenderContextCreateFunc)(mpv_render_context **res, mpv_handle *mpv, mpv_render_param *params);
typedef void (*mpvRenderContextSetUpdateCallbackFunc)(mpv_render_context *ctx, mpv_render_update_fn callback,
                                                      void *callback_ctx);
typedef int (*mpvRenderContextRenderFunc)(mpv_render_context *ctx, mpv_render_param *params);
typedef void (*mpvRenderContextReportSwapFunc)(mpv_render_context *ctx);
typedef uint64_t (*mpvRenderContextUpdateFunc)(mpv_render_context *ctx);
typedef void (*mpvRenderContextFreeFunc)(mpv_render_context *ctx);
typedef unsigned long (*mpvClientApiVersionFunc)();

extern mpvSetOptionStringFunc mpvSetOptionString;
extern mpvObservePropertyFunc mpvObserveProperty;
extern mpvCreateFunc mpvCreate;
extern mpvInitializeFunc mpvInitialize;
extern mpvTerminateDestroyFunc mpvTerminateDestroy;
extern mpvSetWakeupCallbackFunc mpvSetWakeupCallback;
extern mpvCommandStringFunc mpvCommandString;
extern mpvErrorStringFunc mpvErrorString;
extern mpvWaitEventFunc mpvWaitEvent;
extern mpvGetPropertyFunc mpvGetProperty;
extern mpvCommandAsyncFunc mpvCommandAsync;
extern mpvGetPropertyStringFunc mpvGetPropertyString;
extern mpvFreeNodeContentsFunc mpvFreeNodeContents;
extern mpvSetOptionFunc mpvSetOption;
extern mpvFreeFunc mpvFree;
extern mpvRenderContextCreateFunc mpvRenderContextCreate;
extern mpvRenderContextSetUpdateCallbackFunc mpvRenderContextSetUpdateCallback;
extern mpvRenderContextRenderFunc mpvRenderContextRender;
extern mpvRenderContextReportSwapFunc mpvRenderContextReportSwap;
extern mpvRenderContextUpdateFunc mpvRenderContextUpdate;
extern mpvRenderContextFreeFunc mpvRenderContextFree;
extern mpvClientApiVersionFunc mpvClientApiVersion;
#else
#define mpvSetOptionString mpv_set_option_string
#define mpvObserveProperty mpv_observe_property
#define mpvCreate mpv_create
#define mpvInitialize mpv_initialize
#define mpvTerminateDestroy mpv_terminate_destroy
#define mpvSetWakeupCallback mpv_set_wakeup_callback
#define mpvCommandString mpv_command_string
#define mpvErrorString mpv_error_string
#define mpvWaitEvent mpv_wait_event
#define mpvGetProperty mpv_get_property
#define mpvCommandAsync mpv_command_async
#define mpvGetPropertyString mpv_get_property_string
#define mpvFreeNodeContents mpv_free_node_contents
#define mpvSetOption mpv_set_option
#define mpvFree mpv_free
#define mpvRenderContextCreate mpv_render_context_create
#define mpvRenderContextSetUpdateCallback mpv_render_context_set_update_callback
#define mpvRenderContextRender mpv_render_context_render
#define mpvRenderContextReportSwap mpv_render_context_report_swap
#define mpvRenderContextUpdate mpv_render_context_update
#define mpvRenderContextFree mpv_render_context_free
#define mpvClientApiVersion mpv_client_api_version
#endif

class MPVCore : public brls::Singleton<MPVCore> {
public:
    MPVCore();

    ~MPVCore();

    /// Get MPV States

    bool isStopped() const;

    bool isPlaying() const;

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
    std::unordered_map<std::string, mpv_node> getNodeMap(const std::string &key);

    /// Set MPV States

    /**
     * 设置播放链接
     * @param url  播放链接
     * @param extra 额外的参数，如 referrer、audio 等，详情见 mpv 文档
     * @param method 行为，默认为替换当前视频，详情见 mpv 文档
     */
    void setUrl(const std::string &url, const std::string &extra = "", const std::string &method = "replace");

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
    void setVolume(const std::string &value);

    void resume();

    void pause();

    void stop();

    /**
     * 跳转视频
     * @param p 秒
     */
    void seek(int64_t p);
    void seek(const std::string &p);

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

    void showOsdText(const std::string &value, int duration = 2000);

    /**
     * 强制设置视频比例
     * @param value -1 为自动, 可设置 16:9 或 1.333 这两种形式的字符串
     */
    void setAspect(const std::string &value);

    void setMirror(bool value);

    /**
     * 设置视频亮度
     * @param value [-100, 100]
     */
    void setBrightness(int value);

    void setContrast(int value);

    void setSaturation(int value);

    void setGamma(int value);

    void setHue(int value);

    int getBrightness() const;

    int getContrast() const;

    int getSaturation() const;

    int getGamma() const;

    int getHue() const;

    /**
     * 禁用系统锁屏
     */
    static void disableDimming(bool disable);

    /**
     * 绘制视频
     *
     * 支持三种绘制方式 （通过编译参数切换）
     * 1. MPV_SW_RENDER: CPU绘制；优点：适合任意图形API，缺点：效率低。
     *    wiliwili 的 UWP 版本因为用了dx12，所以使用此方式绘制。
     *    在向新的平台移植时推荐优先使用此方式绘制，便于移植，之后再考虑优化问题。
     * 2. MPV_NO_FB: 绘制到默认的 framebuffer 上； 性能适中。
     *    适合 树莓派 等只支持 OpenGL 2.0 的平台；因为不支持独立创建 framebuffer，
     *    所以让 mpv 先绘制到整个屏幕上，再用UI来覆盖。
     *    使用 deko3d (switch only) 时， 暂时也采用 MPV_NO_FB 方式绘制。
     * 3. 默认绘制方式: 将视频绘制到独立的 framebuffer 上，
     *    需要显示在屏幕上时再贴到默认的 framebuffer 上的某个位置；性能最佳。
     *    支持图形API OpenGL 3.2+ / OpenGL ES 2.0+。
     *    在部分平台上支持 direct rendering, 减少 CPU 和 GPU 间的拷贝进一步提升了性能。
     */
    void draw(brls::Rect rect, float alpha = 1.0);

    mpv_render_context *getContext();

    mpv_handle *getHandle();

    /**
     * 播放器内部事件
     * 传递内容为: 事件类型
     */
    MPVEvent *getEvent();

    /**
     * 重启 MPV，用于某些需要重启才能设置的选项
     */
    void restart();

    void reset();

    /**
     * 设置着色器配置
     * @param profile 配置名
     * @param shaders 着色器文件
     * @param settings mpv 配置
     * @param reset 在设置前是否需要重置
     */
    void setShader(const std::string &profile, const std::string &shaders,
                   const std::vector<std::vector<std::string>> &settings, bool reset = true);

    void clearShader(bool showHint = true);

    /// Send command to mpv
    template <typename... Args>
    void command_async(Args &&...args) {
        if (!mpv) {
            brls::Logger::error("mpv is not initialized");
            return;
        }
        std::vector<std::string> commands = {fmt::format("{}", std::forward<Args>(args))...};
        _command_async(commands);
    }

    void _command_async(const std::vector<std::string>& commands);

    // core states
    int64_t duration       = 0;  // second
    int64_t cache_speed    = 0;  // Bps
    int64_t volume         = 100;
    double video_speed     = 0;
    bool video_paused      = false;
    bool video_stopped     = true;
    bool video_seeking     = false;
    bool video_playing     = false;
    bool video_eof         = false;
    float video_aspect     = -1;
    double playback_time   = 0;
    double percent_pos     = 0;
    int64_t video_progress = 0;
    int mpv_error_code     = 0;
    std::string hwCurrent;
    std::string filepath;
    std::string currentShaderProfile;  // 当前着色器脚本名
    std::string currentShader;         // 当前着色器脚本
    std::vector<std::vector<std::string>> currentSetting; // 当前着色器脚本附加的mpv配置

    double video_brightness = 0;
    double video_contrast   = 0;
    double video_saturation = 0;
    double video_hue        = 0;
    double video_gamma      = 0;

    // 低画质解码，剔除解码过程中的部分步骤，可以用来节省cpu
    inline static bool LOW_QUALITY = false;

    // 视频缓存（是否使用内存缓存视频，值为缓存的大小，单位MB）
    inline static int INMEMORY_CACHE = 0;

    // 开启 Terminal 日志
    inline static bool TERMINAL = false;

    // 硬件解码
    inline static bool HARDWARE_DEC = false;

    // 硬解方式
#ifdef __SWITCH__
    inline static std::string PLAYER_HWDEC_METHOD = "auto";
#elif defined(__PSV__)
    inline static std::string PLAYER_HWDEC_METHOD = "vita-copy";
#elif defined(PS4)
    inline static std::string PLAYER_HWDEC_METHOD = "no";
#else
    inline static std::string PLAYER_HWDEC_METHOD = "auto-safe";
#endif

    // 此变量为真时，加载结束后自动播放视频
    inline static bool AUTO_PLAY = true;

    // 若值大于0 则当前时间大于 CLOSE_TIME 时，自动暂停播放
    inline static size_t CLOSE_TIME = 0;

    // 触发倍速时的默认值，单位为 %
    inline static int VIDEO_SPEED = 200;

    // 默认的音量
    inline static int VIDEO_VOLUME = 100;

    // 是否镜像视频
    inline static bool VIDEO_MIRROR = false;

    // 强制的视频比例 (-1 为自动)
    inline static std::string VIDEO_ASPECT = "-1";

    inline static double VIDEO_BRIGHTNESS = 0;
    inline static double VIDEO_CONTRAST   = 0;
    inline static double VIDEO_SATURATION = 0;
    inline static double VIDEO_HUE        = 0;
    inline static double VIDEO_GAMMA      = 0;

private:
    mpv_handle *mpv                 = nullptr;
    mpv_render_context *mpv_context = nullptr;
    brls::Rect rect                 = {0, 0, 1920, 1080};
#ifdef MPV_SW_RENDER
    const int PIXCEL_SIZE          = 4;
    int nvg_image                  = 0;
    const char *sw_format          = "rgba";
    int sw_size[2]                 = {1920, 1080};
    size_t pitch                   = PIXCEL_SIZE * sw_size[0];
    void *pixels                   = nullptr;
    bool redraw                    = false;
    mpv_render_param mpv_params[5] = {
        {MPV_RENDER_PARAM_SW_SIZE, &sw_size[0]}, {MPV_RENDER_PARAM_SW_FORMAT, (void *)sw_format},
        {MPV_RENDER_PARAM_SW_STRIDE, &pitch},    {MPV_RENDER_PARAM_SW_POINTER, pixels},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
#elif defined(BOREALIS_USE_DEKO3D)
    DkFence doneFence;
    DkFence readyFence;
    mpv_deko3d_fbo mpv_fbo{
        nullptr, &readyFence, &doneFence, 1280, 720, DkImageFormat_RGBA8_Unorm,
    };
    mpv_render_param mpv_params[3] = {
        {MPV_RENDER_PARAM_DEKO3D_FBO, &mpv_fbo},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
#elif defined(BOREALIS_USE_D3D11)
    mpv_render_param mpv_params[1] = {
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
#elif defined(MPV_NO_FB)
    GLint default_framebuffer = 0;
    mpv_opengl_fbo mpv_fbo{0, 1920, 1080};
    int flip_y{1};
    mpv_render_param mpv_params[3] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
#else
    GLint default_framebuffer = 0;
    GLuint media_framebuffer  = 0;
    GLuint media_texture      = 0;
    GLShader shader{0};
    mpv_opengl_fbo mpv_fbo{0, 1920, 1080};
    int flip_y{1};
    bool redraw                    = false;
    mpv_render_param mpv_params[3] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
    float vertices[20] = {1.0f,  1.0f,  0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
                          -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f,  0.0f, 0.0f, 1.0f};
#endif

#ifdef MPV_BUNDLE_DLL
    HMEMORYMODULE dll;
#endif

    // MPV 内部事件，传递内容为: 事件类型
    MPVEvent mpvCoreEvent;

    // 当前软件是否在前台的回调
    brls::Event<bool>::Subscription focusSubscription;

    /// Will be called in main thread to get events from mpv core
    void eventMainLoop();

    void initializeVideo();

    void uninitializeVideo();

    void init();

    void clean();

    /**
     * 设置视频渲染区域大小
     * @param rect 视频区域
     */
    void setFrameSize(brls::Rect rect);

    /// MPV callbacks

    static void on_update(void *self);

    static void on_wakeup(void *self);
};
