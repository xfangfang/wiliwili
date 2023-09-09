//
// Created by fang on 2022/8/12.
//

#include <cstdlib>
#include <clocale>
#include "view/mpv_core.hpp"
#include "view/danmaku_core.hpp"
#include "view/subtitle_core.hpp"
#include <pystring.h>
#include "utils/config_helper.hpp"
#include "utils/number_helper.hpp"

#if !defined(MPV_NO_FB) && !defined(MPV_SW_RENDER) && !defined(BOREALIS_USE_DEKO3D)
const char *vertexShaderSource =
    "#version 150 core\n"
    "in vec3 aPos;\n"
    "in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   TexCoord = aTexCoord;\n"
    "}\0";
const char *fragmentShaderSource =
    "#version 150 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D ourTexture;\n"
    "uniform float Alpha = 1.0;\n"
    "void main()\n"
    "{\n"
    "   FragColor = texture(ourTexture, TexCoord);\n"
    "   FragColor.a = Alpha;\n"
    "}\n\0";
#endif

#ifdef BOREALIS_USE_DEKO3D
#include <borealis/platforms/switch/switch_video.hpp>
#endif


static inline void check_error(int status) {
    if (status < 0) {
        brls::Logger::error("MPV ERROR ====> {}", mpv_error_string(status));
    }
}

#if !defined(MPV_SW_RENDER) && !defined(BOREALIS_USE_DEKO3D)
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

void MPVCore::on_update(void *self) {
    brls::sync(
        []() { mpv_render_context_update(MPVCore::instance().getContext()); });
}

void MPVCore::on_wakeup(void *self) {
    brls::sync([]() { MPVCore::instance().eventMainLoop(); });
}

MPVCore::MPVCore() {
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
    this->mpv = mpv_create();
    if (!mpv) {
        brls::fatal("Error Create mpv Handle");
    }

    // misc
    mpv_set_option_string(mpv, "config", "yes");
    mpv_set_option_string(mpv, "config-dir",
                          ProgramConfig::instance().getConfigDir().c_str());
    mpv_set_option_string(mpv, "ytdl", "no");
    mpv_set_option_string(mpv, "terminal", "yes");
    mpv_set_option_string(mpv, "audio-channels", "stereo");
    mpv_set_option_string(mpv, "idle", "yes");
    mpv_set_option_string(mpv, "loop-file", "no");
    mpv_set_option_string(mpv, "osd-level", "0");
    mpv_set_option_string(mpv, "video-timing-offset", "0");  // 60fps
    mpv_set_option_string(mpv, "keep-open", "yes");
    mpv_set_option_string(mpv, "hr-seek", "yes");
    mpv_set_option_string(mpv, "reset-on-next-file", "speed,pause");

    if (MPVCore::LOW_QUALITY) {
        // Less cpu cost
        brls::Logger::info("lavc: skip loop filter and set fast decode");
        mpv_set_option_string(mpv, "vd-lavc-skiploopfilter", "all");
        mpv_set_option_string(mpv, "vd-lavc-fast", "yes");
    }

    if (MPVCore::INMEMORY_CACHE) {
        // cache
        brls::Logger::info("set memory cache: {}MB", MPVCore::INMEMORY_CACHE);
        mpv_set_option_string(
            mpv, "demuxer-max-bytes",
            fmt::format("{}MiB", MPVCore::INMEMORY_CACHE).c_str());
        mpv_set_option_string(
            mpv, "demuxer-max-back-bytes",
            fmt::format("{}MiB", MPVCore::INMEMORY_CACHE / 2).c_str());
    } else {
        mpv_set_option_string(mpv, "cache", "no");
    }

    // hardware decoding
    if (HARDWARE_DEC) {
#ifdef __SWITCH__
        mpv_set_option_string(mpv, "hwdec", "auto");
#elif defined(__PSV__)
        mpv_set_option_string(mpv, "hwdec", "vita-copy");
        brls::Logger::info("MPV hardware decode: vita-copy");
#else
        mpv_set_option_string(mpv, "hwdec", PLAYER_HWDEC_METHOD.c_str());
        brls::Logger::info("MPV hardware decode: {}", PLAYER_HWDEC_METHOD);
#endif
    } else {
        mpv_set_option_string(mpv, "hwdec", "no");
    }

    // Making the loading process faster
#if defined(__SWITCH__)
    mpv_set_option_string(mpv, "vd-lavc-dr", "no");
    mpv_set_option_string(mpv, "vd-lavc-threads", "4");
#elif defined(__PSV__)
    mpv_set_option_string(mpv, "vd-lavc-dr", "no");
    mpv_set_option_string(mpv, "vd-lavc-threads", "4");

    // Fix vo_wait_frame() cannot be wakeup
    mpv_set_option_string(mpv, "video-latency-hacks", "yes");
#endif
    mpv_set_option_string(mpv, "demuxer-lavf-analyzeduration", "0.1");
    mpv_set_option_string(mpv, "demuxer-lavf-probe-info", "nostreams");
    mpv_set_option_string(mpv, "demuxer-lavf-probescore", "24");

    // log
    // mpv_set_option_string(mpv, "msg-level", "ffmpeg=trace");
    // mpv_set_option_string(mpv, "msg-level", "all=no");

#ifdef _DEBUG
    mpv_set_option_string(mpv, "msg-level", "all=v");
#endif

    if (mpv_initialize(mpv) < 0) {
        mpv_terminate_destroy(mpv);
        brls::fatal("Could not initialize mpv context");
    }

    // set observe properties
    //    check_error(mpv_observe_property(mpv, 1, "core-idle", MPV_FORMAT_FLAG));
    check_error(mpv_observe_property(mpv, 2, "pause", MPV_FORMAT_FLAG));
    check_error(mpv_observe_property(mpv, 3, "duration", MPV_FORMAT_INT64));
    check_error(
        mpv_observe_property(mpv, 4, "playback-time", MPV_FORMAT_DOUBLE));
    check_error(mpv_observe_property(mpv, 5, "cache-speed", MPV_FORMAT_INT64));
    check_error(mpv_observe_property(mpv, 6, "percent-pos", MPV_FORMAT_DOUBLE));
    check_error(
        mpv_observe_property(mpv, 7, "paused-for-cache", MPV_FORMAT_FLAG));
    //    check_error(mpv_observe_property(mpv, 8, "demuxer-cache-time", MPV_FORMAT_DOUBLE));
    //    check_error(mpv_observe_property(mpv, 9, "demuxer-cache-state", MPV_FORMAT_NODE));
    check_error(mpv_observe_property(mpv, 10, "speed", MPV_FORMAT_DOUBLE));
    check_error(mpv_observe_property(mpv, 11, "volume", MPV_FORMAT_INT64));
    check_error(mpv_observe_property(mpv, 12, "pause", MPV_FORMAT_FLAG));
    check_error(
        mpv_observe_property(mpv, 13, "playback-abort", MPV_FORMAT_FLAG));
    check_error(mpv_observe_property(mpv, 14, "seeking", MPV_FORMAT_FLAG));

    // init renderer params
#ifdef MPV_SW_RENDER
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_SW)},
        {MPV_RENDER_PARAM_INVALID, nullptr}};
#elif defined(BOREALIS_USE_DEKO3D)
    auto switchPlatform = (brls::SwitchVideoContext *)brls::Application::getPlatform()->getVideoContext();
    mpv_deko3d_init_params deko_init_params{switchPlatform->getDeko3dDevice()};
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_DEKO3D)},
        {MPV_RENDER_PARAM_DEKO3D_INIT_PARAMS, &deko_init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr}};
#else
    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr};
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE,
         const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr}};
#endif

    if (mpv_render_context_create(&mpv_context, mpv, params) < 0) {
        mpv_terminate_destroy(mpv);
        brls::fatal("failed to initialize mpv GL context");
    }

    brls::Logger::info("MPV Version: {}",
                       mpv_get_property_string(mpv, "mpv-version"));
    brls::Logger::info("FFMPEG Version: {}",
                       mpv_get_property_string(mpv, "ffmpeg-version"));
    command_async("set", "audio-client-name", APPVersion::getPackageName());
    setVolume(MPVCore::VIDEO_VOLUME);

    // set event callback
    mpv_set_wakeup_callback(mpv, on_wakeup, this);
    // set render callback
    mpv_render_context_set_update_callback(mpv_context, on_update, this);

    this->initializeGL();

    focusSubscription =
        brls::Application::getWindowFocusChangedEvent()->subscribe(
            [this](bool focus) {
                static bool playing = false;
                // save current AUTO_PLAY value to autoPlay
                static bool autoPlay = AUTO_PLAY;
                if (focus) {
                    // restore AUTO_PLAY
                    AUTO_PLAY = autoPlay;
                    // application is on top
                    if (playing) {
                        resume();
                    }
                } else {
                    // application is sleep, save the current state
                    playing = !isPaused();
                    pause();
                    // do not automatically play video
                    AUTO_PLAY = false;
                }
            });

#ifdef IOS
    glBindFramebuffer(GL_FRAMEBUFFER, 1);
#endif
}

MPVCore::~MPVCore() = default;

void MPVCore::clean() {
    check_error(mpv_command_string(this->mpv, "quit"));

    brls::Application::getWindowFocusChangedEvent()->unsubscribe(
        focusSubscription);

    brls::Logger::info("trying delete fbo");
    this->deleteFrameBuffer();

    brls::Logger::info("trying delete shader");
    this->deleteShader();

    brls::Logger::info("trying free mpv context");
    if (this->mpv_context) {
        mpv_render_context_free(this->mpv_context);
        this->mpv_context = nullptr;
    }

    brls::Logger::info("trying terminate mpv");
    if (this->mpv) {
        mpv_terminate_destroy(this->mpv);
        this->mpv = nullptr;
    }
}

void MPVCore::restart() {
    this->clean();
    this->init();
}

void MPVCore::deleteFrameBuffer() {
#if defined(MPV_NO_FB) || defined(MPV_SW_RENDER)
#elif defined(BOREALIS_USE_DEKO3D)
#else
    if (this->media_framebuffer != 0) {
        glDeleteFramebuffers(1, &this->media_framebuffer);
        this->media_framebuffer = 0;
    }
    if (this->media_texture != 0) {
        glDeleteTextures(1, &this->media_texture);
        this->media_texture = 0;
    }
#endif
}

void MPVCore::deleteShader() {
#if defined(MPV_NO_FB) || defined(MPV_SW_RENDER)
#elif defined(BOREALIS_USE_DEKO3D)
#else
    if (shader.vao != 0) glDeleteVertexArrays(1, &shader.vao);
    if (shader.vbo != 0) glDeleteBuffers(1, &shader.vbo);
    if (shader.ebo != 0) glDeleteBuffers(1, &shader.ebo);
    if (shader.prog != 0) glDeleteProgram(shader.prog);
#endif
}

void MPVCore::initializeGL() {
#if defined(MPV_NO_FB)
#if defined(IOS)
    mpv_fbo.fbo = 1;
#endif
#elif defined(MPV_SW_RENDER)
#elif defined(BOREALIS_USE_DEKO3D)
#else
    if (media_framebuffer != 0) return;
    brls::Logger::debug("initializeGL");

    // create frame buffer
    glGenFramebuffers(1, &this->media_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, this->media_framebuffer);
    glGenTextures(1, &this->media_texture);
    glBindTexture(GL_TEXTURE_2D, this->media_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, brls::Application::windowWidth,
                 brls::Application::windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           this->media_texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        brls::Logger::error("glCheckFramebufferStatus failed");
        this->deleteFrameBuffer();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->mpv_fbo.fbo = (int)this->media_framebuffer;
    brls::Logger::debug("create fbo and texture done\n");

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        brls::Logger::error("vertex shader compile error:", infoLog);
    }

    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        brls::Logger::error("fragment shader compile error:", infoLog);
    }

    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        brls::Logger::error("shaders linking error: {}", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    this->shader.prog = shaderProgram;

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    unsigned int indices[] = {0, 1, 3, 1, 2, 3};
    glGenVertexArrays(1, &this->shader.vao);
    glGenBuffers(1, &this->shader.vbo);
    glGenBuffers(1, &this->shader.ebo);
    glBindVertexArray(this->shader.vao);

    glBindBuffer(GL_ARRAY_BUFFER, this->shader.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->shader.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    GLuint aPos = glGetAttribLocation(shaderProgram, "aPos");
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(aPos);

    GLuint aTexCoord = glGetAttribLocation(shaderProgram, "aTexCoord");
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(aTexCoord);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    brls::Logger::debug("initializeGL done");
#endif
}

void MPVCore::setFrameSize(brls::Rect rect) {
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

    if (nvg_image)
        nvgDeleteImage(brls::Application::getNVGContext(), nvg_image);
    nvg_image = nvgCreateImageRGBA(brls::Application::getNVGContext(),
                                   drawWidth, drawHeight, mpvImageFlags,
                                   (const unsigned char *)pixels);

    sw_size[0] = drawWidth;
    sw_size[1] = drawHeight;
    pitch      = PIXCEL_SIZE * drawWidth;
#elif defined(MPV_NO_FB) || defined(BOREALIS_USE_DEKO3D)
    // Using default framebuffer
    this->mpv_fbo.w = brls::Application::windowWidth;
    this->mpv_fbo.h = brls::Application::windowHeight;
    command_async("set", "video-margin-ratio-left",
                  rect.getMinX() / brls::Application::contentWidth);
    command_async("set", "video-margin-ratio-right",
                  (brls::Application::contentWidth - rect.getMaxX()) /
                      brls::Application::contentWidth);
    command_async("set", "video-margin-ratio-top",
                  rect.getMinY() / brls::Application::contentHeight);
    command_async("set", "video-margin-ratio-bottom",
                  (brls::Application::contentHeight - rect.getMaxY()) /
                      brls::Application::contentHeight);
#else
    if (this->media_texture == 0) return;
    int drawWidth  = rect.getWidth() * brls::Application::windowScale;
    int drawHeight = rect.getHeight() * brls::Application::windowScale;

    if (drawWidth == 0 || drawHeight == 0) return;
    brls::Logger::debug("MPVCore::setFrameSize: {}/{}", drawWidth, drawHeight);
    glBindTexture(GL_TEXTURE_2D, this->media_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, drawWidth, drawHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
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
#endif
}

bool MPVCore::isValid() { return mpv_context != nullptr; }

void MPVCore::openglDraw(brls::Rect rect, float alpha) {
    if (mpv_context == nullptr) return;

#ifdef MPV_SW_RENDER
    if (!pixels) return;
    mpv_render_context_render(this->mpv_context, mpv_params);
    mpv_render_context_report_swap(this->mpv_context);

    auto *vg = brls::Application::getNVGContext();
    nvgUpdateImage(vg, nvg_image, (const unsigned char *)pixels);

    // draw black background
    nvgBeginPath(vg);
    nvgFillColor(vg, NVGcolor{0, 0, 0, alpha});
    nvgRect(vg, rect.getMinX(), rect.getMinY(), rect.getWidth(),
            rect.getHeight());
    nvgFill(vg);

    // draw video
    nvgBeginPath(vg);
    nvgRect(vg, rect.getMinX(), rect.getMinY(), rect.getWidth(),
            rect.getHeight());
    nvgFillPaint(vg, nvgImagePattern(vg, 0, 0, rect.getWidth(),
                                     rect.getHeight(), 0, nvg_image, alpha));
    nvgFill(vg);
#elif defined(MPV_NO_FB) || defined(BOREALIS_USE_DEKO3D)
    // 只在非透明时绘制视频，可以避免退出页面时视频画面残留
    if (alpha >= 1) {
#ifdef BOREALIS_USE_DEKO3D
        static auto videoContext = (brls::SwitchVideoContext *)brls::Application::getPlatform()->getVideoContext();
        mpv_fbo.tex = videoContext->getFramebuffer();
        videoContext->queueSignalFence(&readyFence);
        videoContext->queueFlush();
#endif
        // 绘制视频
        mpv_render_context_render(this->mpv_context, mpv_params);
#ifdef IOS
        glBindFramebuffer(GL_FRAMEBUFFER, 1);
#endif
#ifdef BOREALIS_USE_DEKO3D
        videoContext->queueWaitFence(&doneFence);
#else
        glViewport(0, 0, brls::Application::windowWidth,
                   brls::Application::windowHeight);
#endif
        mpv_render_context_report_swap(this->mpv_context);

        // 画背景来覆盖mpv的黑色边框
        if (rect.getWidth() < brls::Application::contentWidth) {
            auto *vg = brls::Application::getNVGContext();
            nvgBeginPath(vg);
            nvgFillColor(
                vg, brls::Application::getTheme().getColor("brls/background"));
            nvgRect(vg, 0, 0, rect.getMinX(), brls::Application::contentHeight);
            nvgRect(vg, rect.getMaxX(), 0,
                    brls::Application::contentWidth - rect.getMaxX(),
                    brls::Application::contentHeight);
            nvgRect(vg, rect.getMinX() - 1, 0, rect.getWidth() + 2,
                    rect.getMinY());
            nvgRect(vg, rect.getMinX() - 1, rect.getMaxY(), rect.getWidth() + 2,
                    brls::Application::contentHeight - rect.getMaxY());
            nvgFill(vg);
        }
    }
#else
    mpv_render_context_render(this->mpv_context, mpv_params);
    glViewport(0, 0, brls::Application::windowWidth,
               brls::Application::windowHeight);
    mpv_render_context_report_swap(this->mpv_context);

    // shader draw
    glUseProgram(shader.prog);
    glBindTexture(GL_TEXTURE_2D, this->media_texture);
    glBindVertexArray(shader.vao);

    // Set alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    static GLuint alphaID = glGetUniformLocation(shader.prog, "Alpha");
    glUniform1f(alphaID, alpha);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
#endif
}

mpv_render_context *MPVCore::getContext() { return this->mpv_context; }

mpv_handle *MPVCore::getHandle() { return this->mpv; }

MPVEvent *MPVCore::getEvent() { return &this->mpvCoreEvent; }

MPVCustomEvent *MPVCore::getCustomEvent() { return &this->mpvCoreCustomEvent; }

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
        auto event = mpv_wait_event(this->mpv, 0);
        switch (event->event_id) {
            case MPV_EVENT_NONE:
                return;
            case MPV_EVENT_SHUTDOWN:
                brls::Logger::debug("========> MPV_EVENT_SHUTDOWN");
                disableDimming(false);
                return;
            case MPV_EVENT_FILE_LOADED:
                brls::Logger::info("========> MPV_EVENT_FILE_LOADED");
                // event 8: 文件预加载结束，准备解码
                mpvCoreEvent.fire(MpvEventEnum::MPV_LOADED);
                // 移除其他备用链接
                command_async("playlist-clear");
                break;
            case MPV_EVENT_START_FILE:
                // event 6: 开始加载文件
                brls::Logger::info("========> MPV_EVENT_START_FILE");
                disableDimming(true);

                // show osd for a really long time
                mpvCoreEvent.fire(MpvEventEnum::START_FILE);

                mpvCoreEvent.fire(MpvEventEnum::LOADING_START);
                break;
            case MPV_EVENT_PLAYBACK_RESTART:
                // event 21: 开始播放文件（一般是播放或调整进度结束之后触发）
                brls::Logger::info("========> MPV_EVENT_PLAYBACK_RESTART");
                mpvCoreEvent.fire(MpvEventEnum::LOADING_END);
                DanmakuCore::instance().refresh();
                if (AUTO_PLAY){
                    mpvCoreEvent.fire(MpvEventEnum::MPV_RESUME);
                    this->resume();
                    disableDimming(true);
                }
                else{
                    mpvCoreEvent.fire(MpvEventEnum::MPV_PAUSE);
                    this->pause();
                    disableDimming(false);
                }
                break;
            case MPV_EVENT_END_FILE: {
                // event 7: 文件播放结束
                disableDimming(false);
                brls::Logger::info("========> MPV_STOP");
                mpvCoreEvent.fire(MpvEventEnum::MPV_STOP);
                disableDimming(false);
                auto node = (mpv_event_end_file *)event->data;
                if (node->reason == MPV_END_FILE_REASON_ERROR) {
                    brls::Logger::error("========> MPV ERROR: {}",
                                        mpv_error_string(node->error));
                    mpvCoreEvent.fire(MpvEventEnum::MPV_FILE_ERROR);
                }

                break;
            }
            case MPV_EVENT_PROPERTY_CHANGE: {
                auto *data = ((mpv_event_property *)event->data)->data;
                switch (event->reply_userdata) {
                    case 2:
                        // 播放器播放状态改变（暂停或播放）
                        break;
                    case 3:
                        // 视频总时长更新
                        if (((mpv_event_property *)event->data)->data)
                            duration =
                                *(int64_t *)((mpv_event_property *)event->data)
                                     ->data;
                        if (duration != 0) {
                            brls::Logger::debug("========> duration: {}",
                                                duration);
                            mpvCoreEvent.fire(MpvEventEnum::UPDATE_DURATION);
                        }
                        break;
                    case 4:
                        // 播放进度更新
                        if (((mpv_event_property *)event->data)->data) {
                            playback_time =
                                *(double *)((mpv_event_property *)event->data)
                                     ->data;
                            if (video_progress != (int64_t)playback_time) {
                                video_progress = (int64_t)playback_time;
                                mpvCoreEvent.fire(
                                    MpvEventEnum::UPDATE_PROGRESS);
                                // 判断是否需要暂停播放
                                if (CLOSE_TIME > 0 &&
                                    wiliwili::getUnixTime() > CLOSE_TIME) {
                                    CLOSE_TIME = 0;
                                    this->pause();
                                }
                            }
                        }

                        break;
                    case 5:
                        // 视频 cache speed
                        if (((mpv_event_property *)event->data)->data) {
                            cache_speed =
                                *(int64_t *)((mpv_event_property *)event->data)
                                     ->data;
                            mpvCoreEvent.fire(MpvEventEnum::CACHE_SPEED_CHANGE);
                        }
                        break;
                    case 6:
                        // 视频进度更新（百分比）
                        if (((mpv_event_property *)event->data)->data) {
                            percent_pos =
                                *(double *)((mpv_event_property *)event->data)
                                     ->data;
                        }
                        break;
                    case 7:
                        // 发生了缓存等待
                        if (!data) break;

                        if (*(int *)data) {
                            brls::Logger::info("========> VIDEO PAUSED FOR CACHE");
                            mpvCoreEvent.fire(MpvEventEnum::LOADING_START);
                            disableDimming(false);
                        } else {
                            brls::Logger::info("========> VIDEO RESUME FROM CACHE");
                            mpvCoreEvent.fire(MpvEventEnum::LOADING_END);
                            disableDimming(true);
                        }
                        break;
                    case 8:
                        // 缓存时间
                        if (((mpv_event_property *)event->data)->data) {
                            brls::Logger::verbose(
                                "demuxer-cache-time: {}",
                                *(double *)((mpv_event_property *)event->data)
                                     ->data);
                        }
                        break;
                    case 9:
                        // 缓存信息
                        if (((mpv_event_property *)event->data)->data) {
                            auto *node =
                                (mpv_node *)((mpv_event_property *)event->data)
                                    ->data;
                            std::unordered_map<std::string, mpv_node> node_map;
                            for (int i = 0; i < node->u.list->num; i++) {
                                node_map.insert(std::make_pair(
                                    std::string(node->u.list->keys[i]),
                                    node->u.list->values[i]));
                            }
                            brls::Logger::debug(
                                "total-bytes: {:.2f}MB; cache-duration: "
                                "{:.2f}; "
                                "underrun: {}; fw-bytes: {:.2f}MB; bof-cached: "
                                "{}; eof-cached: {}; file-cache-bytes: {}; "
                                "raw-input-rate: {:.2f};",
                                node_map["total-bytes"].u.int64 / 1048576.0,
                                node_map["cache-duration"].u.double_,
                                node_map["underrun"].u.flag,
                                node_map["fw-bytes"].u.int64 / 1048576.0,
                                node_map["bof-cached"].u.flag,
                                node_map["eof-cached"].u.flag,
                                node_map["file-cache-bytes"].u.int64 /
                                    1048576.0,
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
                            if (duration > 0 &&
                                (double)duration - playback_time < 1) {
                                video_progress = duration;
                                brls::Logger::info(
                                    "========> END OF FILE (paused)");
                                mpvCoreEvent.fire(MpvEventEnum::END_OF_FILE);
                            } else {
                                brls::Logger::info("========> PAUSE");
                                mpvCoreEvent.fire(MpvEventEnum::MPV_PAUSE);
                            }
                            disableDimming(false);
                        } else {
                            brls::Logger::info("========> RESUME");
                            mpvCoreEvent.fire(MpvEventEnum::MPV_RESUME);
                            disableDimming(true);
                        }
                        break;
                    case 13:
                        if (data) video_stopped = *(int *)data;

                        break;
                    case 14:
                        if (data) video_seeking = *(int *)data;
                        if (video_seeking){
                            brls::Logger::info("========> VIDEO SEEKING");
                            mpvCoreEvent.fire(MpvEventEnum::LOADING_START);
                            disableDimming(false);
                        }
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
    DanmakuCore::instance().reset();
    SubtitleCore::instance().reset();
    this->percent_pos    = 0;
    this->duration       = 0;  // second
    this->cache_speed    = 0;  // Bps
    this->playback_time  = 0;
    this->video_progress = 0;
}

void MPVCore::setUrl(const std::string &url, const std::string &extra,
                     const std::string &method) {
    brls::Logger::debug("{} Url: {}, extra: {}", method, url, extra);
    if (extra.empty()) {
        command_async("loadfile", url, method);
    } else {
        command_async("loadfile", url, method, extra);
    }
}

void MPVCore::setBackupUrl(const std::string &url, const std::string &extra) {
    this->setUrl(url, extra, "append");
}

void MPVCore::setVolume(int64_t value) {
    if (value < 0 || value > 100) return;
    command_async("set", "volume", value);
    MPVCore::VIDEO_VOLUME = (int)value;
}

void MPVCore::setVolume(const std::string& value) {
    command_async("set", "volume", value);
    MPVCore::VIDEO_VOLUME = std::stoi(value);
}

int64_t MPVCore::getVolume() const { return this->volume; }

void MPVCore::resume() { command_async("set", "pause", "no"); }

void MPVCore::pause() { command_async("set", "pause", "yes"); }

void MPVCore::stop() { command_async("stop"); }

void MPVCore::seek(int64_t p) { command_async("seek", p, "absolute"); }

void MPVCore::seek(const std::string& p) { command_async("seek", p, "absolute"); }

void MPVCore::seekRelative(int64_t p) { command_async("seek", p, "relative"); }

void MPVCore::seekPercent(double p) {
    command_async("seek", p * 100, "absolute-percent");
}

bool MPVCore::isStopped() const { return video_stopped; }

bool MPVCore::isPaused() const { return video_paused; }

double MPVCore::getSpeed() const { return video_speed; }

void MPVCore::setSpeed(double value) { command_async("set", "speed", value); }

// todo: remove these sync function
std::string MPVCore::getString(const std::string &key) {
    char *value = nullptr;
    mpv_get_property(mpv, key.c_str(), MPV_FORMAT_STRING, &value);
    if (!value) return "";
    std::string result = std::string{value};
    mpv_free(value);
    return result;
}

double MPVCore::getDouble(const std::string &key) {
    double value = 0;
    mpv_get_property(mpv, key.c_str(), MPV_FORMAT_DOUBLE, &value);
    return value;
}

int64_t MPVCore::getInt(const std::string &key) {
    int64_t value = 0;
    mpv_get_property(mpv, key.c_str(), MPV_FORMAT_INT64, &value);
    return value;
}

std::unordered_map<std::string, mpv_node> MPVCore::getNodeMap(
    const std::string &key) {
    mpv_node node;
    std::unordered_map<std::string, mpv_node> nodeMap;
    if (mpv_get_property(mpv, key.c_str(), MPV_FORMAT_NODE, &node) < 0)
        return nodeMap;
    if (node.format != MPV_FORMAT_NODE_MAP) return nodeMap;
    // todo: 目前不要使用 mpv_node中有指针的部分，因为这些内容指向的内存会在这个函数结束的时候删除
    for (int i = 0; i < node.u.list->num; i++) {
        char *nodeKey = node.u.list->keys[i];
        if (nodeKey == nullptr) continue;
        nodeMap.insert(
            std::make_pair(std::string{nodeKey}, node.u.list->values[i]));
    }
    mpv_free_node_contents(&node);
    return nodeMap;
}

double MPVCore::getPlaybackTime() const { return playback_time; }

void MPVCore::disableDimming(bool disable) {
    brls::Application::getPlatform()->disableScreenDimming(
        disable, "Playing video", APPVersion::getPackageName());
    static bool deactivationAvailable =
        ProgramConfig::instance().getSettingItem(SettingItem::DEACTIVATED_TIME,
                                                 0) > 0;
    if (deactivationAvailable) {
        brls::Application::setAutomaticDeactivation(!disable);
    }
}

void MPVCore::setShader(const std::string &profile, const std::string &shaders,
                        bool showHint) {
    brls::Logger::info("Set shader [{}]: {}", profile, shaders);
    if (shaders.empty()) return;
    command_async("no-osd", "change-list", "glsl-shaders", "set",
                  "\"" + shaders + "\"");
    if (showHint) showOsdText(profile);
}

void MPVCore::clearShader(bool showHint) {
    brls::Logger::info("Clear shader");
    command_async("no-osd", "change-list", "glsl-shaders", "clr", "\"\"");
    if (showHint) showOsdText("Clear shader");
}

void MPVCore::showOsdText(const std::string &value, int d) {
    command_async("show-text", value, d);
}