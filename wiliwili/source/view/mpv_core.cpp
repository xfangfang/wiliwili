//
// Created by fang on 2022/8/12.
//

#include <cstdlib>
#include <clocale>
#include <pystring.h>
#include <borealis/core/thread.hpp>
#include <borealis/core/application.hpp>

#include "utils/config_helper.hpp"
#include "utils/number_helper.hpp"
#include "utils/crash_helper.hpp"
#include "view/mpv_core.hpp"

#ifdef MPV_BUNDLE_DLL
mpvSetOptionStringFunc mpvSetOptionString;
mpvObservePropertyFunc mpvObserveProperty;
mpvCreateFunc mpvCreate;
mpvInitializeFunc mpvInitialize;
mpvTerminateDestroyFunc mpvTerminateDestroy;
mpvSetWakeupCallbackFunc mpvSetWakeupCallback;
mpvCommandStringFunc mpvCommandString;
mpvErrorStringFunc mpvErrorString;
mpvWaitEventFunc mpvWaitEvent;
mpvGetPropertyFunc mpvGetProperty;
mpvCommandAsyncFunc mpvCommandAsync;
mpvGetPropertyStringFunc mpvGetPropertyString;
mpvFreeNodeContentsFunc mpvFreeNodeContents;
mpvSetOptionFunc mpvSetOption;
mpvFreeFunc mpvFree;
mpvRenderContextCreateFunc mpvRenderContextCreate;
mpvRenderContextSetUpdateCallbackFunc mpvRenderContextSetUpdateCallback;
mpvRenderContextRenderFunc mpvRenderContextRender;
mpvRenderContextReportSwapFunc mpvRenderContextReportSwap;
mpvRenderContextUpdateFunc mpvRenderContextUpdate;
mpvRenderContextFreeFunc mpvRenderContextFree;
mpvClientApiVersionFunc mpvClientApiVersion;
#endif

#ifdef MPV_USE_FB
#ifdef PS4
#include "utils/ps4_mpv_shaders.hpp"
#endif
#if defined(USE_GLES2)
#define SHADER_VERSION "#version 100\n"
#define SHADER_PRECISION "precision mediump float;\n"
#elif defined(USE_GLES3)
#define SHADER_VERSION "#version 300 es\n"
#define SHADER_PRECISION "precision mediump float;\n"
#else
#define SHADER_VERSION "#version 150 core\n"
#define SHADER_PRECISION ""
#endif

const char *vertexShaderSource = SHADER_VERSION
#ifdef USE_GLES2
    "attribute vec3 aPos;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 TexCoord;\n"
#else
    "in vec3 aPos;\n"
    "in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
#endif
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   TexCoord = aTexCoord;\n"
    "}\0";
const char *fragmentShaderSource = SHADER_VERSION SHADER_PRECISION
#ifdef USE_GLES2
    "varying vec2 TexCoord;\n"
#else
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
#endif
    "uniform sampler2D ourTexture;\n"
    "uniform float Alpha;\n"
    "void main()\n"
    "{\n"
#ifdef USE_GLES2
    "   gl_FragColor = texture2D(ourTexture, TexCoord);\n"
    "   gl_FragColor.a = Alpha;\n"
#else
    "   FragColor = texture(ourTexture, TexCoord);\n"
    "   FragColor.a = Alpha;\n"
#endif
    "}\n\0";

#define CONCATENATE3(s1, s2, s3) s1##s2##s3
#define CONCATENATE2(s1, s2) s1##s2
#define checkGLError(type, id)                                                 \
    {                                                                          \
        char *info = nullptr;                                                  \
        int length = 0;                                                        \
        CONCATENATE3(glGet, type, iv)(id, GL_COMPILE_STATUS, &length);         \
        if (length > 0) {                                                      \
            info = (char *)malloc(length);                                     \
            if (info) {                                                        \
                CONCATENATE3(glGet, type, InfoLog)(id, length, &length, info); \
            }                                                                  \
        }                                                                      \
        if (info) {                                                            \
            brls::Logger::error("Failed to load the shader: {}", info);        \
            free(info);                                                        \
        } else {                                                               \
            brls::Logger::error("Failed to load the shader");                  \
        }                                                                      \
        CONCATENATE2(glDelete, type)(id);                                      \
    }

static GLuint createShader(GLint type, const char *source) {
    GLuint shader = glCreateShader(type);
#ifdef PS4
    glShaderBinary(1, &shader, 2, type == GL_VERTEX_SHADER ? PS4_MPV_SHADER_VERT : PS4_MPV_SHADER_FRAG,
                   type == GL_VERTEX_SHADER ? PS4_MPV_SHADER_VERT_LENGTH : PS4_MPV_SHADER_FRAG_LENGTH);
#else
    int success;
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    // check for shader compile errors
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        checkGLError(Shader, shader);
        return 0;
    }
#endif
    return shader;
}

static GLuint linkProgram(GLuint s1, GLuint s2) {
    if (s1 == 0 || s2 == 0) return 0;
    GLuint program = glCreateProgram();
    int success;
    glAttachShader(program, s1);
    glAttachShader(program, s2);
    glLinkProgram(program);
    // check for linking errors
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        checkGLError(Program, program);
        return 0;
    }
    return program;
}
#endif

#ifdef BOREALIS_USE_DEKO3D
#include <borealis/platforms/switch/switch_video.hpp>
#elif defined(BOREALIS_USE_D3D11)
#include <borealis/platforms/driver/d3d11.hpp>
extern std::unique_ptr<brls::D3D11Context> D3D11_CONTEXT;
#elif defined(USE_GL2)
#undef glBindFramebuffer
#define glBindFramebuffer(a, b) void()
#endif

static inline void check_error(int status) {
    if (status < 0) {
        brls::Logger::error("MPV ERROR ====> {}", mpvErrorString(status));
    }
}

#if defined(BOREALIS_USE_OPENGL) && !defined(MPV_SW_RENDER)
static void *get_proc_address(void *unused, const char *name) {
#ifdef __SDL2__
    SDL_GL_GetCurrentContext();
    return (void *)SDL_GL_GetProcAddress(name);
#else
    glfwGetCurrentContext();
    return (void *)glfwGetProcAddress(name);
#endif
}
#endif

static inline float aspectConverter(const std::string &value) {
    try {
        if (value.empty()) {
            return -1;
        } else if (pystring::count(value, ":")) {
            // 比例模式
            auto num = pystring::split(value, ":");
            if (num.size() != 2) return -1;
            return std::stof(num[0]) / std::stof(num[1]);
        } else {
            // 纯数字
            return std::stof(value);
        }
    } catch (const std::exception &e) {
        return -1;
    }
}

void MPVCore::on_update(void *self) {
    brls::sync([]() {
        uint64_t flags = mpvRenderContextUpdate(MPVCore::instance().getContext());
#if !defined(MPV_SW_RENDER) && !defined(MPV_USE_FB)
        (void)flags;
#else
        MPVCore::instance().redraw = flags & MPV_RENDER_UPDATE_FRAME;
        if (MPVCore::instance().redraw) {
#ifdef MPV_SW_RENDER
            if (!MPVCore::instance().pixels) return;
            mpvRenderContextRender(MPVCore::instance().mpv_context, MPVCore::instance().mpv_params);
            mpvRenderContextReportSwap(MPVCore::instance().mpv_context);
#else
            mpvRenderContextRender(MPVCore::instance().mpv_context, MPVCore::instance().mpv_params);
            glBindFramebuffer(GL_FRAMEBUFFER, MPVCore::instance().default_framebuffer);
            glViewport(0, 0, (GLsizei)brls::Application::windowWidth, (GLsizei)brls::Application::windowHeight);
            mpvRenderContextReportSwap(MPVCore::instance().mpv_context);
#endif
        }
#endif
    });
}

void MPVCore::on_wakeup(void *self) {
    brls::sync([]() { MPVCore::instance().eventMainLoop(); });
}

#if defined(MPV_BUNDLE_DLL)
template <typename Module, typename fnGetProcAddress>
void initMpvProc(Module dll, fnGetProcAddress pGetProcAddress) {
    mpvSetOptionString     = (mpvSetOptionStringFunc)pGetProcAddress(dll, "mpv_set_option_string");
    mpvObserveProperty     = (mpvObservePropertyFunc)pGetProcAddress(dll, "mpv_observe_property");
    mpvCreate              = (mpvCreateFunc)pGetProcAddress(dll, "mpv_create");
    mpvInitialize          = (mpvInitializeFunc)pGetProcAddress(dll, "mpv_initialize");
    mpvTerminateDestroy    = (mpvTerminateDestroyFunc)pGetProcAddress(dll, "mpv_terminate_destroy");
    mpvSetWakeupCallback   = (mpvSetWakeupCallbackFunc)pGetProcAddress(dll, "mpv_set_wakeup_callback");
    mpvCommandString       = (mpvCommandStringFunc)pGetProcAddress(dll, "mpv_command_string");
    mpvErrorString         = (mpvErrorStringFunc)pGetProcAddress(dll, "mpv_error_string");
    mpvWaitEvent           = (mpvWaitEventFunc)pGetProcAddress(dll, "mpv_wait_event");
    mpvGetProperty         = (mpvGetPropertyFunc)pGetProcAddress(dll, "mpv_get_property");
    mpvCommandAsync        = (mpvCommandAsyncFunc)pGetProcAddress(dll, "mpv_command_async");
    mpvGetPropertyString   = (mpvGetPropertyStringFunc)pGetProcAddress(dll, "mpv_get_property_string");
    mpvFreeNodeContents    = (mpvFreeNodeContentsFunc)pGetProcAddress(dll, "mpv_free_node_contents");
    mpvSetOption           = (mpvSetOptionFunc)pGetProcAddress(dll, "mpv_set_option");
    mpvFree                = (mpvFreeFunc)pGetProcAddress(dll, "mpv_free");
    mpvRenderContextCreate = (mpvRenderContextCreateFunc)pGetProcAddress(dll, "mpv_render_context_create");
    mpvRenderContextUpdate = (mpvRenderContextUpdateFunc)pGetProcAddress(dll, "mpv_render_context_update");
    mpvRenderContextFree   = (mpvRenderContextFreeFunc)pGetProcAddress(dll, "mpv_render_context_free");
    mpvRenderContextRender = (mpvRenderContextRenderFunc)pGetProcAddress(dll, "mpv_render_context_render");
    mpvRenderContextSetUpdateCallback =
        (mpvRenderContextSetUpdateCallbackFunc)pGetProcAddress(dll, "mpv_render_context_set_update_callback");
    mpvRenderContextReportSwap = (mpvRenderContextReportSwapFunc)pGetProcAddress(dll, "mpv_render_context_report_swap");
    mpvClientApiVersion        = (mpvClientApiVersionFunc)pGetProcAddress(dll, "mpv_client_api_version");
}
#endif

MPVCore::MPVCore() {
#if defined(MPV_BUNDLE_DLL)
    HMODULE hMpv = ::LoadLibraryW(L"libmpv-2.dll");
    if (!hMpv) {
        HRSRC hSrc   = ::FindResource(nullptr, "MPV", RT_RCDATA);
        HGLOBAL hRes = ::LoadResource(nullptr, hSrc);
        DWORD dwSize = ::SizeofResource(nullptr, hSrc);
        dll          = MemoryLoadLibrary(::LockResource(hRes), dwSize);
        ::FreeResource(hRes);

        brls::Logger::info("Load bundled libmpv-2.dll, size: {}", dwSize);
        initMpvProc(dll, MemoryGetProcAddress);
    } else {
        char dllPath[MAX_PATH];
        ::GetModuleFileNameA(hMpv, dllPath, sizeof(dllPath));

        brls::Logger::info("Load external `{}`", dllPath);
        initMpvProc(hMpv, GetProcAddress);
    }
#endif
    this->init();
    // Destroy mpv when application exit
    brls::Application::getExitDoneEvent()->subscribe([this]() {
        this->clean();
#ifdef MPV_SW_RENDER
        if (pixels) {
            free(pixels);
            pixels             = nullptr;
            mpv_params[3].data = nullptr;
        }
#endif
    });
}

void MPVCore::init() {
    setlocale(LC_NUMERIC, "C");
    this->mpv = mpvCreate();
    if (!mpv) {
        brls::fatal("Error Create mpv Handle");
    }

    // misc
    mpvSetOptionString(mpv, "config", "yes");
    mpvSetOptionString(mpv, "config-dir", ProgramConfig::instance().getConfigDir().c_str());
    mpvSetOptionString(mpv, "ytdl", "no");
    mpvSetOptionString(mpv, "audio-channels", "stereo");
    mpvSetOptionString(mpv, "idle", "yes");
    mpvSetOptionString(mpv, "loop-file", "no");
    mpvSetOptionString(mpv, "osd-level", "0");
    mpvSetOptionString(mpv, "video-timing-offset", "0");  // 60fps
    mpvSetOptionString(mpv, "keep-open", "yes");
    mpvSetOptionString(mpv, "hr-seek", "yes");
    mpvSetOptionString(mpv, "reset-on-next-file", "speed,pause");
    mpvSetOptionString(mpv, "vo", "libmpv");
    mpvSetOptionString(mpv, "pulse-latency-hacks", "no");

    mpvSetOption(mpv, "brightness", MPV_FORMAT_DOUBLE, &MPVCore::VIDEO_BRIGHTNESS);
    mpvSetOption(mpv, "contrast", MPV_FORMAT_DOUBLE, &MPVCore::VIDEO_CONTRAST);
    mpvSetOption(mpv, "saturation", MPV_FORMAT_DOUBLE, &MPVCore::VIDEO_SATURATION);
    mpvSetOption(mpv, "hue", MPV_FORMAT_DOUBLE, &MPVCore::VIDEO_HUE);
    mpvSetOption(mpv, "gamma", MPV_FORMAT_DOUBLE, &MPVCore::VIDEO_GAMMA);

    if (MPVCore::LOW_QUALITY) {
        // Less cpu cost
        brls::Logger::info("lavc: skip loop filter and set fast decode");
        mpvSetOptionString(mpv, "vd-lavc-skiploopfilter", "all");
        mpvSetOptionString(mpv, "vd-lavc-fast", "yes");
        if (mpvClientApiVersion() >= MPV_MAKE_VERSION(2, 2)) {
            mpvSetOptionString(mpv, "profile", "fast");
        } else {
            mpvSetOptionString(mpv, "scale", "bilinear");
            mpvSetOptionString(mpv, "dscale", "bilinear");
            mpvSetOptionString(mpv, "dither", "no");
            mpvSetOptionString(mpv, "correct-downscaling", "no");
            mpvSetOptionString(mpv, "linear-downscaling", "no");
            mpvSetOptionString(mpv, "sigmoid-upscaling", "no");
            mpvSetOptionString(mpv, "hdr-compute-peak", "no");
            mpvSetOptionString(mpv, "allow-delayed-peak-detect", "yes");
        }
    }

    if (MPVCore::INMEMORY_CACHE) {
        // cache
        brls::Logger::info("set memory cache: {}MB", MPVCore::INMEMORY_CACHE);
        mpvSetOptionString(mpv, "demuxer-max-bytes", fmt::format("{}MiB", MPVCore::INMEMORY_CACHE).c_str());
        mpvSetOptionString(mpv, "demuxer-max-back-bytes", fmt::format("{}MiB", MPVCore::INMEMORY_CACHE / 2).c_str());
    } else {
        mpvSetOptionString(mpv, "cache", "no");
    }

    // hardware decoding
    if (HARDWARE_DEC) {
        mpvSetOptionString(mpv, "hwdec", PLAYER_HWDEC_METHOD.c_str());
        brls::Logger::info("MPV hardware decode: {}", PLAYER_HWDEC_METHOD);
    } else {
        mpvSetOptionString(mpv, "hwdec", "no");
    }

    // Making the loading process faster
#if defined(__SWITCH__)
    mpvSetOptionString(mpv, "vd-lavc-dr", "no");
    mpvSetOptionString(mpv, "vd-lavc-threads", "4");
    // This should fix random crash, but I don't know why.
    mpvSetOptionString(mpv, "opengl-glfinish", "yes");
#elif defined(PS4)
    mpvSetOptionString(mpv, "vd-lavc-threads", "6");
#elif defined(__PSV__)
    mpvSetOptionString(mpv, "vd-lavc-dr", "no");
    mpvSetOptionString(mpv, "vd-lavc-threads", "4");

    // Fix vo_wait_frame() cannot be wakeup
    mpvSetOptionString(mpv, "video-latency-hacks", "yes");
#endif
    // 过低的值可能导致部分直播流无法正确播放
    mpvSetOptionString(mpv, "demuxer-lavf-analyzeduration", "0.4");
    mpvSetOptionString(mpv, "demuxer-lavf-probescore", "24");

    // log
    // mpvSetOptionString(mpv, "msg-level", "ffmpeg=trace");
    // mpvSetOptionString(mpv, "msg-level", "all=no");
    if (MPVCore::TERMINAL) {
        mpvSetOptionString(mpv, "terminal", "yes");
        if ( brls::Logger::getLogLevel() >= brls::LogLevel::LOG_DEBUG ) {
            mpvSetOptionString(mpv, "msg-level", "all=v");
        }
    }

    if (mpvInitialize(mpv) < 0) {
        mpvTerminateDestroy(mpv);
        brls::fatal("Could not initialize mpv context");
    }

    // set observe properties
    check_error(mpvObserveProperty(mpv, 1, "core-idle", MPV_FORMAT_FLAG));
    check_error(mpvObserveProperty(mpv, 2, "eof-reached", MPV_FORMAT_FLAG));
    check_error(mpvObserveProperty(mpv, 3, "duration", MPV_FORMAT_INT64));
    check_error(mpvObserveProperty(mpv, 4, "playback-time", MPV_FORMAT_DOUBLE));
    check_error(mpvObserveProperty(mpv, 5, "cache-speed", MPV_FORMAT_INT64));
    check_error(mpvObserveProperty(mpv, 6, "percent-pos", MPV_FORMAT_DOUBLE));
    check_error(mpvObserveProperty(mpv, 7, "paused-for-cache", MPV_FORMAT_FLAG));
    //    check_error(mpvObserveProperty(mpv, 8, "demuxer-cache-time", MPV_FORMAT_DOUBLE));
    //    check_error(mpvObserveProperty(mpv, 9, "demuxer-cache-state", MPV_FORMAT_NODE));
    check_error(mpvObserveProperty(mpv, 10, "speed", MPV_FORMAT_DOUBLE));
    check_error(mpvObserveProperty(mpv, 11, "volume", MPV_FORMAT_INT64));
    check_error(mpvObserveProperty(mpv, 12, "pause", MPV_FORMAT_FLAG));
    check_error(mpvObserveProperty(mpv, 13, "playback-abort", MPV_FORMAT_FLAG));
    check_error(mpvObserveProperty(mpv, 14, "seeking", MPV_FORMAT_FLAG));
    check_error(mpvObserveProperty(mpv, 15, "hwdec-current", MPV_FORMAT_STRING));
    check_error(mpvObserveProperty(mpv, 16, "path", MPV_FORMAT_STRING));
    check_error(mpvObserveProperty(mpv, 17, "brightness", MPV_FORMAT_DOUBLE));
    check_error(mpvObserveProperty(mpv, 18, "contrast", MPV_FORMAT_DOUBLE));
    check_error(mpvObserveProperty(mpv, 19, "saturation", MPV_FORMAT_DOUBLE));
    check_error(mpvObserveProperty(mpv, 20, "gamma", MPV_FORMAT_DOUBLE));
    check_error(mpvObserveProperty(mpv, 21, "hue", MPV_FORMAT_DOUBLE));

    // init renderer params
#ifdef MPV_SW_RENDER
    mpv_render_param params[]{{MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_SW)},
                              {MPV_RENDER_PARAM_INVALID, nullptr}};
#elif defined(BOREALIS_USE_DEKO3D)
    int advanced_control{1};
    auto switchPlatform = (brls::SwitchVideoContext *)brls::Application::getPlatform()->getVideoContext();
    mpv_deko3d_init_params deko_init_params{switchPlatform->getDeko3dDevice()};
    mpv_render_param params[]{{MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_DEKO3D)},
                              {MPV_RENDER_PARAM_DEKO3D_INIT_PARAMS, &deko_init_params},
                              {MPV_RENDER_PARAM_ADVANCED_CONTROL, &advanced_control},
                              {MPV_RENDER_PARAM_INVALID, nullptr}};
#elif defined(BOREALIS_USE_D3D11)
    mpv_dxgi_init_params init_params{D3D11_CONTEXT->getDevice(), D3D11_CONTEXT->getSwapChain()};
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_DXGI)},
        {MPV_RENDER_PARAM_DXGI_INIT_PARAMS, &init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
#else
    int advanced_control{1};
    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr};
    mpv_render_param params[]{{MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
                              {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
                              {MPV_RENDER_PARAM_ADVANCED_CONTROL, &advanced_control},
                              {MPV_RENDER_PARAM_INVALID, nullptr}};
#endif

    if (mpvRenderContextCreate(&mpv_context, mpv, params) < 0) {
        mpvTerminateDestroy(mpv);
        brls::fatal("failed to initialize mpv GL context");
    }
#ifdef BOREALIS_USE_D3D11
    wiliwili::initCrashDump();
#endif
    brls::Logger::info("MPV Version: {}", mpvGetPropertyString(mpv, "mpv-version"));
    brls::Logger::info("FFMPEG Version: {}", mpvGetPropertyString(mpv, "ffmpeg-version"));
    command_async("set", "audio-client-name", APPVersion::getPackageName());
    setVolume(MPVCore::VIDEO_VOLUME);

    // set event callback
    mpvSetWakeupCallback(mpv, on_wakeup, this);
    // set render callback
    mpvRenderContextSetUpdateCallback(mpv_context, on_update, this);

    focusSubscription = brls::Application::getWindowFocusChangedEvent()->subscribe([this](bool focus) {
        static bool playing = false;
        static std::chrono::system_clock::time_point sleepTime{};
        if (focus) {
            // restore AUTO_PLAY
            AUTO_PLAY = ProgramConfig::instance().getBoolOption(SettingItem::PLAYER_AUTO_PLAY);
            // application is on top
            auto timeNow = std::chrono::system_clock::now();
            if (playing && timeNow < (sleepTime + std::chrono::seconds(120))) {
                resume();
            }
        } else {
            // application is sleep, save the current state
            playing   = isPlaying();
            sleepTime = std::chrono::system_clock::now();
            pause();
            // do not automatically play video
            AUTO_PLAY = false;
        }
    });

    brls::Application::getExitEvent()->subscribe([]() { disableDimming(false); });

    this->initializeVideo();
}

#ifdef MPV_BUNDLE_DLL
MPVCore::~MPVCore() { MemoryFreeLibrary(dll); }
#else
MPVCore::~MPVCore() = default;
#endif

void MPVCore::clean() {
    check_error(mpvCommandString(this->mpv, "quit"));

    brls::Application::getWindowFocusChangedEvent()->unsubscribe(focusSubscription);

    brls::Logger::info("uninitialize Video");
    this->uninitializeVideo();

    brls::Logger::info("trying free mpv context");
    if (this->mpv_context) {
        mpvRenderContextFree(this->mpv_context);
        this->mpv_context = nullptr;
    }

    brls::Logger::info("trying terminate mpv");
    if (this->mpv) {
        mpvTerminateDestroy(this->mpv);
        this->mpv = nullptr;
    }
}

void MPVCore::restart() {
    this->clean();
    this->init();
    setMirror(MPVCore::VIDEO_MIRROR);
    setShader(currentShaderProfile, currentShader, currentSetting, false);
    mpvCoreEvent.fire(MpvEventEnum::RESTART);

    // 如果正在播放视频时重启mpv，重启前后存在软硬解切，那么视频尺寸会不正确
    // 手动设置一次尺寸可以解决这个问题 (同 MPVCore::reset())
    setFrameSize(rect);
}

void MPVCore::uninitializeVideo() {
#ifdef MPV_USE_FB
    if (media_framebuffer != 0) glDeleteFramebuffers(1, &media_framebuffer);
    if (media_texture != 0) glDeleteTextures(1, &media_texture);
    if (shader.vbo != 0) glDeleteBuffers(1, &shader.vbo);
    if (shader.ebo != 0) glDeleteBuffers(1, &shader.ebo);
    if (shader.prog != 0) glDeleteProgram(shader.prog);
#ifdef MPV_USE_VAO
    if (shader.vao != 0) glDeleteVertexArrays(1, &shader.vao);
    shader.vao = 0;
#endif
    media_framebuffer = 0;
    media_texture     = 0;
    shader.vbo        = 0;
    shader.ebo        = 0;
    shader.prog       = 0;
#endif
}

void MPVCore::initializeVideo() {
#if defined(BOREALIS_USE_OPENGL) && !defined(MPV_SW_RENDER)
    // Get default framebuffer
#if defined(IOS)
    // SDL: OpenGL ES on iOS doesn't use the traditional system-framebuffer setup provided in other operating systems.
    default_framebuffer = 1;
#else
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_framebuffer);
#endif
#endif

#if defined(MPV_NO_FB)
    mpv_fbo.fbo = default_framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
#elif defined(MPV_USE_FB)
    if (this->media_texture != 0) return;
    brls::Logger::debug("initializeGL");

    // create texture
    glGenTextures(1, &this->media_texture);
    glBindTexture(GL_TEXTURE_2D, this->media_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)brls::Application::windowWidth, (int)brls::Application::windowHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // create frame buffer
    glGenFramebuffers(1, &this->media_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, this->media_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->media_texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        brls::Logger::error("glCheckFramebufferStatus failed");
        return;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
    this->mpv_fbo.fbo = (int)this->media_framebuffer;
    brls::Logger::debug("create fbo and texture done\n");

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    // fragment shader
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // link shaders
    GLuint shaderProgram = linkProgram(vertexShader, fragmentShader);

    if (vertexShader) glDeleteShader(vertexShader);
    if (fragmentShader) glDeleteShader(fragmentShader);
    this->shader.prog = shaderProgram;

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    unsigned int indices[] = {0, 1, 3, 1, 2, 3};
    glGenBuffers(1, &this->shader.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->shader.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &this->shader.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->shader.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

#ifdef MPV_USE_VAO
    glGenVertexArrays(1, &this->shader.vao);
    glBindVertexArray(this->shader.vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->shader.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->shader.ebo);

    GLuint aPos = glGetAttribLocation(shaderProgram, "aPos");
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)nullptr);
    glEnableVertexAttribArray(aPos);

    GLuint aTexCoord = glGetAttribLocation(shaderProgram, "aTexCoord");
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(aTexCoord);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
#endif
    brls::Logger::debug("initializeGL done");
#endif
}

void MPVCore::setFrameSize(brls::Rect r) {
    rect = r;
    if (isnan(rect.getWidth()) || isnan(rect.getHeight())) return;

#ifdef MPV_SW_RENDER
#ifdef BOREALIS_USE_D3D11
    // 使用 dx11 的拷贝交换，否则视频渲染异常
    const static int mpvImageFlags = NVG_IMAGE_STREAMING | NVG_IMAGE_COPY_SWAP;
#else
    const static int mpvImageFlags = 0;
#endif
    int drawWidth  = rect.getWidth() * brls::Application::windowScale;
    int drawHeight = rect.getHeight() * brls::Application::windowScale;
    if (drawWidth == 0 || drawHeight == 0) return;
    int frameSize = drawWidth * drawHeight;

    if (pixels != nullptr && frameSize > sw_size[0] * sw_size[1]) {
        brls::Logger::debug("Enlarge video surface buffer");
        free(pixels);
        pixels = nullptr;
    }

    if (pixels == nullptr) {
        pixels             = malloc(frameSize * PIXCEL_SIZE);
        mpv_params[3].data = pixels;
    }

    if (nvg_image) nvgDeleteImage(brls::Application::getNVGContext(), nvg_image);
    nvg_image = nvgCreateImageRGBA(brls::Application::getNVGContext(), drawWidth, drawHeight, mpvImageFlags,
                                   (const unsigned char *)pixels);

    sw_size[0] = drawWidth;
    sw_size[1] = drawHeight;
    pitch      = PIXCEL_SIZE * drawWidth;

    // 在视频暂停时调整纹理尺寸，视频画面会被清空为黑色，强制重新绘制一次，避免这个问题
    mpvRenderContextRender(mpv_context, mpv_params);
    mpvRenderContextReportSwap(mpv_context);
#elif !defined(MPV_USE_FB)
        // Using default framebuffer
#ifndef BOREALIS_USE_D3D11
    this->mpv_fbo.w = brls::Application::windowWidth;
    this->mpv_fbo.h = brls::Application::windowHeight;
#endif
    command_async("set", "video-margin-ratio-right",
                  (brls::Application::contentWidth - rect.getMaxX()) / brls::Application::contentWidth);
    command_async("set", "video-margin-ratio-bottom",
                  (brls::Application::contentHeight - rect.getMaxY()) / brls::Application::contentHeight);
    command_async("set", "video-margin-ratio-top", rect.getMinY() / brls::Application::contentHeight);
    command_async("set", "video-margin-ratio-left", rect.getMinX() / brls::Application::contentWidth);
#else
    if (this->media_texture == 0) return;
    int drawWidth  = rect.getWidth() * brls::Application::windowScale;
    int drawHeight = rect.getHeight() * brls::Application::windowScale;

    if (drawWidth == 0 || drawHeight == 0) return;
    brls::Logger::debug("MPVCore::setFrameSize: {}/{}", drawWidth, drawHeight);
    glBindTexture(GL_TEXTURE_2D, this->media_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, drawWidth, drawHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    this->mpv_fbo.w = drawWidth;
    this->mpv_fbo.h = drawHeight;

    float new_min_x = rect.getMinX() / brls::Application::contentWidth * 2 - 1;
    float new_min_y = 1 - rect.getMinY() / brls::Application::contentHeight * 2;
    float new_max_x = rect.getMaxX() / brls::Application::contentWidth * 2 - 1;
    float new_max_y = 1 - rect.getMaxY() / brls::Application::contentHeight * 2;

    vertices[0]  = new_max_x;
    vertices[1]  = new_min_y;
    vertices[5]  = new_max_x;
    vertices[6]  = new_max_y;
    vertices[10] = new_min_x;
    vertices[11] = new_max_y;
    vertices[15] = new_min_x;
    vertices[16] = new_min_y;

    glBindBuffer(GL_ARRAY_BUFFER, this->shader.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 在视频暂停时调整纹理尺寸，视频画面会被清空为黑色，强制重新绘制一次，避免这个问题
    mpvRenderContextRender(mpv_context, mpv_params);
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
    glViewport(0, 0, (GLsizei)brls::Application::windowWidth, (GLsizei)brls::Application::windowHeight);
    mpvRenderContextReportSwap(mpv_context);
#endif
}

bool MPVCore::isValid() { return mpv_context != nullptr; }

void MPVCore::draw(brls::Rect area, float alpha) {
    if (mpv_context == nullptr) return;
    if (!(this->rect == area)) setFrameSize(area);

#ifdef MPV_SW_RENDER
    if (!pixels) return;

    auto *vg = brls::Application::getNVGContext();
    nvgUpdateImage(vg, nvg_image, (const unsigned char *)pixels);

    // draw black background
    nvgBeginPath(vg);
    NVGcolor bg{};
    bg.a = alpha;
    nvgFillColor(vg, bg);
    nvgRect(vg, rect.getMinX(), rect.getMinY(), rect.getWidth(), rect.getHeight());
    nvgFill(vg);

    // draw video
    nvgBeginPath(vg);
    nvgRect(vg, rect.getMinX(), rect.getMinY(), rect.getWidth(), rect.getHeight());
    nvgFillPaint(vg, nvgImagePattern(vg, 0, 0, rect.getWidth(), rect.getHeight(), 0, nvg_image, alpha));
    nvgFill(vg);
#elif !defined(MPV_USE_FB)
    // 只在非透明时绘制视频，可以避免退出页面时视频画面残留
    if (alpha >= 1) {
#ifdef BOREALIS_USE_DEKO3D
        static auto videoContext = (brls::SwitchVideoContext *)brls::Application::getPlatform()->getVideoContext();
        mpv_fbo.tex              = videoContext->getFramebuffer();
        videoContext->queueSignalFence(&readyFence);
        videoContext->queueFlush();
#endif
        // 绘制视频
        mpvRenderContextRender(this->mpv_context, mpv_params);
#ifdef BOREALIS_USE_DEKO3D
        videoContext->queueWaitFence(&doneFence);
#elif defined(BOREALIS_USE_D3D11)
        D3D11_CONTEXT->beginFrame();
#else
        glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer);
        glViewport(0, 0, brls::Application::windowWidth, brls::Application::windowHeight);
#endif
        mpvRenderContextReportSwap(this->mpv_context);

        // 画背景来覆盖mpv的黑色边框
        if (rect.getWidth() < brls::Application::contentWidth) {
            auto *vg = brls::Application::getNVGContext();
            nvgBeginPath(vg);
            nvgFillColor(vg, brls::Application::getTheme().getColor("brls/background"));
            nvgRect(vg, 0, 0, rect.getMinX(), brls::Application::contentHeight);
            nvgRect(vg, rect.getMaxX(), 0, brls::Application::contentWidth - rect.getMaxX(),
                    brls::Application::contentHeight);
            nvgRect(vg, rect.getMinX() - 1, 0, rect.getWidth() + 2, rect.getMinY());
            nvgRect(vg, rect.getMinX() - 1, rect.getMaxY(), rect.getWidth() + 2,
                    brls::Application::contentHeight - rect.getMaxY());
            nvgFill(vg);
        }
    }
#else
    // shader draw
    glUseProgram(shader.prog);
    glBindTexture(GL_TEXTURE_2D, this->media_texture);
#ifdef MPV_USE_VAO
    glBindVertexArray(shader.vao);
#else
    glBindBuffer(GL_ARRAY_BUFFER, shader.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shader.ebo);
    static GLuint aPos = glGetAttribLocation(shader.prog, "aPos");
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)nullptr);
    glEnableVertexAttribArray(aPos);

    static GLuint aTexCoord = glGetAttribLocation(shader.prog, "aTexCoord");
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(aTexCoord);
#endif

    // Set alpha
    if (alpha < 1) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
    static GLint alphaID = glGetUniformLocation(shader.prog, "Alpha");
    glUniform1f(alphaID, alpha);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
#endif
}

mpv_render_context *MPVCore::getContext() { return this->mpv_context; }

mpv_handle *MPVCore::getHandle() { return this->mpv; }

MPVEvent *MPVCore::getEvent() { return &this->mpvCoreEvent; }

std::string MPVCore::getCacheSpeed() const {
    if (cache_speed >> 20 > 0) {
        return fmt::format("{:.2f} MB/s", (cache_speed >> 10) / 1024.0f);
    } else if (cache_speed >> 10 > 0) {
        return fmt::format("{:.2f} KB/s", cache_speed / 1024.0f);
    } else {
        return fmt::format("{} B/s", cache_speed);
    }
}

void MPVCore::eventMainLoop() {
    while (true) {
        auto event = mpvWaitEvent(this->mpv, 0);
        switch (event->event_id) {
            case MPV_EVENT_NONE:
                return;
            case MPV_EVENT_LOG_MESSAGE: {
                auto log = (mpv_event_log_message *)event->data;
                if (log->log_level <= MPV_LOG_LEVEL_ERROR) {
                    brls::Logger::error("{}: {}", log->prefix, log->text);
                } else if (log->log_level <= MPV_LOG_LEVEL_WARN) {
                    brls::Logger::warning("{}: {}", log->prefix, log->text);
                } else if (log->log_level <= MPV_LOG_LEVEL_INFO) {
                    brls::Logger::info("{}: {}", log->prefix, log->text);
                } else if (log->log_level <= MPV_LOG_LEVEL_V) {
                    brls::Logger::debug("{}: {}", log->prefix, log->text);
                } else {
                    brls::Logger::verbose("{}: {}", log->prefix, log->text);
                }
            } break;
            case MPV_EVENT_FILE_LOADED:
                brls::Logger::info("========> MPV_EVENT_FILE_LOADED");
                // event 8: 文件预加载结束，准备解码
                mpvCoreEvent.fire(MpvEventEnum::MPV_LOADED);
                // 发布一次进度更新事件，避免进度条在0秒时没有进度更新
                video_progress = 0;
                mpvCoreEvent.fire(MpvEventEnum::UPDATE_PROGRESS);
                // 移除其他备用链接
                command_async("playlist-clear");

                if (AUTO_PLAY) {
                    mpvCoreEvent.fire(MpvEventEnum::MPV_RESUME);
                    this->resume();
                } else {
                    mpvCoreEvent.fire(MpvEventEnum::MPV_PAUSE);
                    this->pause();
                }
                break;
            case MPV_EVENT_START_FILE:
                // event 6: 开始加载文件
                brls::Logger::info("========> MPV_EVENT_START_FILE");

                // show osd for a really long time
                mpvCoreEvent.fire(MpvEventEnum::START_FILE);

                mpvCoreEvent.fire(MpvEventEnum::LOADING_START);
                break;
            case MPV_EVENT_PLAYBACK_RESTART:
                // event 21: 开始播放文件（一般是播放或调整进度结束之后触发）
                brls::Logger::info("========> MPV_EVENT_PLAYBACK_RESTART");
                video_stopped = false;
                mpvCoreEvent.fire(MpvEventEnum::LOADING_END);
                break;
            case MPV_EVENT_END_FILE: {
                // event 7: 文件播放结束
                brls::Logger::info("========> MPV_STOP");
                mpvCoreEvent.fire(MpvEventEnum::MPV_STOP);
                video_stopped = true;
                auto node     = (mpv_event_end_file *)event->data;
                if (node->reason == MPV_END_FILE_REASON_ERROR) {
                    mpv_error_code = node->error;
                    brls::Logger::error("========> MPV ERROR: {}", mpvErrorString(node->error));
                    mpvCoreEvent.fire(MpvEventEnum::MPV_FILE_ERROR);
                }

                break;
            }
            case MPV_EVENT_PROPERTY_CHANGE: {
                auto *data = ((mpv_event_property *)event->data)->data;
                switch (event->reply_userdata) {
                    case 1:
                        if (data) {
                            bool playing = *(int *)data == 0;
                            if (playing != video_playing) {
                                video_playing = playing;
                                mpvCoreEvent.fire(MpvEventEnum::MPV_IDLE);
                            }
                            video_playing = playing;
                            disableDimming(video_playing);
                        }
                        break;
                    case 2:
                        if (data) video_eof = *(int *)data;
                        // 当视频播放自然结束时会先触发一次 EOF，如果这时 stop 或者设置了新的播放文件，会触发 STOP 然后再触发一次 EOF
                        // 避免多次触发 EOF，将第二种情况排除在外
                        if (video_eof && !video_stopped) {
                            brls::Logger::info("========> END OF FILE");
                            mpvCoreEvent.fire(MpvEventEnum::END_OF_FILE);
                        }
                        break;
                    case 3:
                        // 视频总时长更新
                        if (((mpv_event_property *)event->data)->data)
                            duration = *(int64_t *)((mpv_event_property *)event->data)->data;
                        if (duration != 0) {
                            brls::Logger::debug("========> duration: {}", duration);
                            mpvCoreEvent.fire(MpvEventEnum::UPDATE_DURATION);
                        }
                        break;
                    case 4:
                        // 播放进度更新
                        if (((mpv_event_property *)event->data)->data) {
                            playback_time = *(double *)((mpv_event_property *)event->data)->data;
                            if (video_progress != (int64_t)playback_time) {
                                video_progress = (int64_t)playback_time;
                                mpvCoreEvent.fire(MpvEventEnum::UPDATE_PROGRESS);
                                // 判断是否需要暂停播放
                                if (CLOSE_TIME > 0 && wiliwili::getUnixTime() > CLOSE_TIME) {
                                    CLOSE_TIME = 0;
                                    this->pause();
                                }
                            }
                        }

                        break;
                    case 5:
                        // 视频 cache speed
                        if (((mpv_event_property *)event->data)->data) {
                            cache_speed = *(int64_t *)((mpv_event_property *)event->data)->data;
                            mpvCoreEvent.fire(MpvEventEnum::CACHE_SPEED_CHANGE);
                        }
                        break;
                    case 6:
                        // 视频进度更新（百分比）
                        if (((mpv_event_property *)event->data)->data) {
                            percent_pos = *(double *)((mpv_event_property *)event->data)->data;
                        }
                        break;
                    case 7:
                        // 发生了缓存等待
                        if (!data) break;

                        if (*(int *)data) {
                            brls::Logger::info("========> VIDEO PAUSED FOR CACHE");
                            mpvCoreEvent.fire(MpvEventEnum::LOADING_START);
                        } else {
                            brls::Logger::info("========> VIDEO RESUME FROM CACHE");
                            mpvCoreEvent.fire(MpvEventEnum::LOADING_END);
                        }
                        break;
                    case 8:
                        // 缓存时间
                        if (((mpv_event_property *)event->data)->data) {
                            brls::Logger::verbose("demuxer-cache-time: {}",
                                                  *(double *)((mpv_event_property *)event->data)->data);
                        }
                        break;
                    case 9:
                        // 缓存信息
                        if (((mpv_event_property *)event->data)->data) {
                            auto *node = (mpv_node *)((mpv_event_property *)event->data)->data;
                            std::unordered_map<std::string, mpv_node> node_map;
                            for (int i = 0; i < node->u.list->num; i++) {
                                node_map.insert(
                                    std::make_pair(std::string(node->u.list->keys[i]), node->u.list->values[i]));
                            }
                            brls::Logger::debug(
                                "total-bytes: {:.2f}MB; cache-duration: "
                                "{:.2f}; "
                                "underrun: {}; fw-bytes: {:.2f}MB; bof-cached: "
                                "{}; eof-cached: {}; file-cache-bytes: {}; "
                                "raw-input-rate: {:.2f};",
                                node_map["total-bytes"].u.int64 / 1048576.0, node_map["cache-duration"].u.double_,
                                node_map["underrun"].u.flag, node_map["fw-bytes"].u.int64 / 1048576.0,
                                node_map["bof-cached"].u.flag, node_map["eof-cached"].u.flag,
                                node_map["file-cache-bytes"].u.int64 / 1048576.0,
                                node_map["raw-input-rate"].u.int64 / 1048576.0);
                        }
                        break;
                    case 10:
                        // 倍速信息
                        if (data) {
                            video_speed = *(double *)data;
                            mpvCoreEvent.fire(VIDEO_SPEED_CHANGE);
                        }
                        break;
                    case 11:
                        // 音量信息
                        if (data) {
                            if (*(int64_t *)data > 0 && volume == 0) {
                                mpvCoreEvent.fire(VIDEO_UNMUTE);
                            } else if (*(int64_t *)data == 0 && volume > 0) {
                                mpvCoreEvent.fire(VIDEO_MUTE);
                            }
                            volume = *(int64_t *)data;
                            mpvCoreEvent.fire(VIDEO_VOLUME_CHANGE);
                        }
                        break;
                    case 12:
                        if (data) video_paused = *(int *)data;
                        if (video_paused) {
                            brls::Logger::info("========> PAUSE");
                            mpvCoreEvent.fire(MpvEventEnum::MPV_PAUSE);
                        } else if (!video_stopped) {
                            brls::Logger::info("========> RESUME");
                            mpvCoreEvent.fire(MpvEventEnum::MPV_RESUME);
                        }
                        break;
                    case 13:
                        if (data) video_stopped = *(int *)data;

                        break;
                    case 14:
                        if (data) video_seeking = *(int *)data;
                        if (video_seeking) {
                            brls::Logger::info("========> VIDEO SEEKING");
                            mpvCoreEvent.fire(MpvEventEnum::LOADING_START);
                        }
                        break;
                    case 15:
                        if (data) {
                            hwCurrent = *(char **)data;
                            brls::Logger::info("========> HW: {}", hwCurrent);
                            GA("hwdec", {{"hwdec", hwCurrent}})
                        }
                        break;
                    case 16:
                        if (data) filepath = *(char **)data;
                        break;
                    case 17:
                        if (data) video_brightness = *(double *)data;
                        break;
                    case 18:
                        if (data) video_contrast = *(double *)data;
                        break;
                    case 19:
                        if (data) video_saturation = *(double *)data;
                        break;
                    case 20:
                        if (data) video_gamma = *(double *)data;
                        break;
                    case 21:
                        if (data) video_hue = *(double *)data;
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }
}

void MPVCore::reset() {
    brls::Logger::debug("MPVCore::reset");
    mpvCoreEvent.fire(MpvEventEnum::RESET);
    this->percent_pos    = 0;
    this->duration       = 0;  // second
    this->cache_speed    = 0;  // Bps
    this->playback_time  = 0;
    this->video_progress = 0;
    this->mpv_error_code = 0;
    this->video_aspect   = aspectConverter(MPVCore::VIDEO_ASPECT);

    // 软硬解切换后应该手动设置一次渲染尺寸
    // 切换视频前设置渲染尺寸可以顺便将上一条视频的最后一帧画面清空
    setFrameSize(rect);
}

void MPVCore::setUrl(const std::string &url, const std::string &extra, const std::string &method) {
    brls::Logger::debug("{} Url: {}, extra: {}", method, url, extra);
    if (extra.empty()) {
        command_async("loadfile", url, method);
    } else {
        if (mpvClientApiVersion() >= MPV_MAKE_VERSION(2, 3))
            command_async("loadfile", url, method, "0", extra);
        else
            command_async("loadfile", url, method, extra);
    }
}

void MPVCore::setBackupUrl(const std::string &url, const std::string &extra) { this->setUrl(url, extra, "append"); }

void MPVCore::setVolume(int64_t value) {
    if (value < 0 || value > 100) return;
    command_async("set", "volume", value);
    MPVCore::VIDEO_VOLUME = (int)value;
}

void MPVCore::setVolume(const std::string &value) {
    command_async("set", "volume", value);
    MPVCore::VIDEO_VOLUME = std::stoi(value);
}

int64_t MPVCore::getVolume() const { return this->volume; }

void MPVCore::resume() { command_async("set", "pause", "no"); }

void MPVCore::pause() { command_async("set", "pause", "yes"); }

void MPVCore::stop() { command_async("stop"); }

void MPVCore::seek(int64_t p) { command_async("seek", p, "absolute"); }

void MPVCore::seek(const std::string &p) { command_async("seek", p, "absolute"); }

void MPVCore::seekRelative(int64_t p) { command_async("seek", p, "relative"); }

void MPVCore::seekPercent(double p) { command_async("seek", p * 100, "absolute-percent"); }

bool MPVCore::isStopped() const { return video_stopped; }

bool MPVCore::isPlaying() const { return video_playing; }

bool MPVCore::isPaused() const { return video_paused; }

double MPVCore::getSpeed() const { return video_speed; }

void MPVCore::setSpeed(double value) {
    if (video_speed != value) command_async("set", "speed", value);
}

void MPVCore::setAspect(const std::string &value) {
    MPVCore::VIDEO_ASPECT = value;
    video_aspect          = aspectConverter(MPVCore::VIDEO_ASPECT);
    if (value == "-2") {
        // 拉伸全屏
        command_async("set", "keepaspect", "no");
        command_async("set", "video-aspect-override", "-1");
        command_async("set", "panscan", "0.0");
    } else if (value == "-3") {
        // 裁剪填充
        command_async("set", "keepaspect", "yes");
        command_async("set", "video-aspect-override", "-1");
        command_async("set", "panscan", "1.0");
    } else {
        // 指定比例
        command_async("set", "keepaspect", "yes");
        command_async("set", "video-aspect-override", MPVCore::VIDEO_ASPECT);
        command_async("set", "panscan", "0.0");
    }
}

void MPVCore::setMirror(bool value) {
    MPVCore::VIDEO_MIRROR = value;
    command_async("set", "vf", value ? "hflip": "");
}

void MPVCore::setBrightness(int value) {
    if (value < -100) value = -100;
    if (value > 100) value = 100;
    MPVCore::VIDEO_BRIGHTNESS = value;
    if (video_brightness != value) command_async("set", "brightness", value);
}

void MPVCore::setContrast(int value) {
    if (value < -100) value = -100;
    if (value > 100) value = 100;
    MPVCore::VIDEO_CONTRAST = value;
    if (video_contrast != value) command_async("set", "contrast", value);
}

void MPVCore::setSaturation(int value) {
    if (value < -100) value = -100;
    if (value > 100) value = 100;
    MPVCore::VIDEO_SATURATION = value;
    if (video_saturation != value) command_async("set", "saturation", value);
}

void MPVCore::setGamma(int value) {
    if (value < -100) value = -100;
    if (value > 100) value = 100;
    MPVCore::VIDEO_GAMMA = value;
    if (video_gamma != value) command_async("set", "gamma", value);
}

void MPVCore::setHue(int value) {
    if (value < -100) value = -100;
    if (value > 100) value = 100;
    MPVCore::VIDEO_HUE = value;
    if (video_hue != value) command_async("set", "hue", value);
}

int MPVCore::getBrightness() const { return video_brightness; }

int MPVCore::getContrast() const { return video_contrast; }

int MPVCore::getSaturation() const { return video_saturation; }

int MPVCore::getGamma() const { return video_gamma; }

int MPVCore::getHue() const { return video_hue; }

// todo: remove these sync function
std::string MPVCore::getString(const std::string &key) {
    char *value = nullptr;
    mpvGetProperty(mpv, key.c_str(), MPV_FORMAT_STRING, &value);
    if (!value) return "";
    std::string result = std::string{value};
    mpvFree(value);
    return result;
}

double MPVCore::getDouble(const std::string &key) {
    double value = 0;
    mpvGetProperty(mpv, key.c_str(), MPV_FORMAT_DOUBLE, &value);
    return value;
}

int64_t MPVCore::getInt(const std::string &key) {
    int64_t value = 0;
    mpvGetProperty(mpv, key.c_str(), MPV_FORMAT_INT64, &value);
    return value;
}

std::unordered_map<std::string, mpv_node> MPVCore::getNodeMap(const std::string &key) {
    mpv_node node;
    std::unordered_map<std::string, mpv_node> nodeMap;
    if (mpvGetProperty(mpv, key.c_str(), MPV_FORMAT_NODE, &node) < 0) return nodeMap;
    if (node.format != MPV_FORMAT_NODE_MAP) return nodeMap;
    // todo: 目前不要使用 mpv_node中有指针的部分，因为这些内容指向的内存会在这个函数结束的时候删除
    for (int i = 0; i < node.u.list->num; i++) {
        char *nodeKey = node.u.list->keys[i];
        if (nodeKey == nullptr) continue;
        nodeMap.insert(std::make_pair(std::string{nodeKey}, node.u.list->values[i]));
    }
    mpvFreeNodeContents(&node);
    return nodeMap;
}

double MPVCore::getPlaybackTime() const { return playback_time; }

void MPVCore::disableDimming(bool disable) {
    brls::Logger::info("disableDimming: {}", disable);
    brls::Application::getPlatform()->disableScreenDimming(disable, "Playing video", APPVersion::getPackageName());
    static bool deactivationAvailable = ProgramConfig::instance().getSettingItem(SettingItem::DEACTIVATED_TIME, 0) > 0;
    if (deactivationAvailable) {
        brls::Application::setAutomaticDeactivation(!disable);
    }
}

void MPVCore::setShader(const std::string &profile, const std::string &shaders,
                        const std::vector<std::vector<std::string>> &settings, bool reset) {
    brls::Logger::info("Set shader [{}]: {}", profile, shaders);

    // 如果之前设置的shader包含mpv配置，就需要重置一下
    if (!currentSetting.empty() && reset) clearShader(false);

    currentShaderProfile = profile;
    currentShader        = shaders;
    currentSetting       = settings;

    // 设置着色器
    if (!shaders.empty()) command_async("no-osd", "change-list", "glsl-shaders", "set", shaders);

    // 设置mpv配置
    for (auto &setting : settings) {
        _command_async(setting);
    }

    // 显示通知
    if (reset) brls::Application::notify(profile);
}

void MPVCore::clearShader(bool showHint) {
    brls::Logger::info("Clear shader");

    // 如果当前不涉及mpv配置修改，就无需重置
    bool reset = !currentSetting.empty();

    currentShader.clear();
    currentShaderProfile.clear();
    currentSetting.clear();

    // 清空着色器
    command_async("no-osd", "change-list", "glsl-shaders", "clr", "");

    // 重置mpv配置
    if (reset) MPVCore::instance().restart();

    // 显示通知
    if (showHint) brls::Application::notify("Clear profile");
}

void MPVCore::showOsdText(const std::string &value, int d) { command_async("show-text", value, d); }

void MPVCore::_command_async(const std::vector<std::string> &commands) {
    std::vector<const char *> res;
    res.reserve(commands.size() + 1);
    for (auto &i : commands) {
        res.emplace_back(i.c_str());
    }
    res.emplace_back(nullptr);
    mpvCommandAsync(mpv, 0, res.data());
}