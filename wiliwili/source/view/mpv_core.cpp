//
// Created by fang on 2022/8/12.
//

#include <stdlib.h>
#include <clocale>
#include "view/mpv_core.hpp"
#include "pystring.h"

#ifdef __SWITCH__
#include <switch.h>
#endif

const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   TexCoord = aTexCoord;\n"
    "}\0";
const char *fragmentShaderSource =
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D ourTexture;\n"
    "uniform float Alpha = 1.0;\n"
    "void main()\n"
    "{\n"
    "   FragColor = texture(ourTexture, TexCoord);\n"
    "   FragColor.a = Alpha;\n"
    "}\n\0";

static inline void check_error(int status) {
    if (status < 0) {
        brls::Logger::error("MPV ERROR ====> {}", mpv_error_string(status));
    }
}

static void *get_proc_address(void *unused, const char *name) {
    glfwGetCurrentContext();
    return (void *)glfwGetProcAddress(name);
}

void MPVCore::on_update(void *self) {
    brls::sync(
        []() { mpv_render_context_update(MPVCore::instance().getContext()); });
}

void MPVCore::on_wakeup(void *self) {
    brls::sync([]() { MPVCore::instance().eventMainLoop(); });
}

MPVCore::MPVCore() {
    setlocale(LC_NUMERIC, "C");
    this->mpv = mpv_create();
    if (!mpv) {
        brls::fatal("Error Create mpv Handle");
    }

    // misc
    mpv_set_option_string(mpv, "no-config", "yes");
    mpv_set_option_string(mpv, "no-ytdl", "yes");
    mpv_set_option_string(mpv, "terminal", "yes");
    mpv_set_option_string(mpv, "audio-channels", "stereo");
    mpv_set_option_string(mpv, "user-agent", "wiliwili/0.1(NintendoSwitch)");
    mpv_set_option_string(mpv, "referrer", "https://www.bilibili.com/");
    mpv_set_option_string(mpv, "idle", "yes");
    mpv_set_option_string(mpv, "opengl-pbo", "yes");
    mpv_set_option_string(mpv, "fbo-format", "rgba8");
    mpv_set_option_string(mpv, "reset-on-next-file", "all");
    mpv_set_option_string(mpv, "loop-file", "no");
    //    mpv_set_option_string(mpv, "osd-level", "3");
    mpv_set_option_string(mpv, "video-timing-offset", "0");  // 60fps
    mpv_set_option_string(mpv, "keep-open", "yes");

    // Less cpu cost
    mpv_set_option_string(mpv, "vd-lavc-skiploopfilter", "all");
    mpv_set_option_string(mpv, "vd-lavc-fast", "yes");

    // cache
    mpv_set_option_string(mpv, "demuxer-max-bytes", "100MiB");
    mpv_set_option_string(mpv, "demuxer-max-back-bytes", "50MiB");
    mpv_set_option_string(mpv, "cache-on-disk", "no");
    mpv_set_option_string(mpv, "cache-secs", "120");

    // hardware decoding
#ifndef __SWITCH__
    mpv_set_option_string(mpv, "hwdec", "auto-safe");
#endif

    // rebase-start-time no
    // demuxer-lavf-hacks no

    // Making the loading process faster
    mpv_set_option_string(mpv, "vd-lavc-threads", "4");
    // mpv_set_option_string(mpv, "demuxer-lavf-analyzeduration", "2");
    // mpv_set_option_string(mpv, "demuxer-lavf-probescore", "24");

    // log
    // mpv_set_option_string(mpv, "msg-level", "ffmpeg=trace");
    // mpv_set_option_string(mpv, "msg-level", "all=v");
    mpv_set_option_string(mpv, "msg-level", "all=no");

    if (mpv_initialize(mpv) < 0) {
        mpv_terminate_destroy(mpv);
        brls::fatal("Could not initialize mpv context");
    }

    check_error(mpv_observe_property(mpv, 1, "core-idle", MPV_FORMAT_FLAG));
    check_error(mpv_observe_property(mpv, 2, "pause", MPV_FORMAT_FLAG));
    check_error(mpv_observe_property(mpv, 3, "duration", MPV_FORMAT_INT64));
    check_error(
        mpv_observe_property(mpv, 4, "playback-time", MPV_FORMAT_DOUBLE));
    check_error(mpv_observe_property(mpv, 5, "cache-speed", MPV_FORMAT_INT64));

    brls::Logger::debug("initializeGL");
    this->initializeGL();
}

MPVCore::~MPVCore() {
    check_error(mpv_command_string(this->mpv, "quit"));

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
        // mpv_destroy(this->mpv);
        this->mpv = nullptr;
    }
}

void MPVCore::deleteFrameBuffer() {
    if (this->media_framebuffer != 0) {
        glDeleteFramebuffers(1, &this->media_framebuffer);
        this->media_framebuffer = 0;
    }
    if (this->media_texture != 0) {
        glDeleteTextures(1, &this->media_texture);
        this->media_texture = 0;
    }
}

void MPVCore::deleteShader() {
    if (shader.vao != 0) glDeleteVertexArrays(1, &shader.vao);
    if (shader.vbo != 0) glDeleteBuffers(1, &shader.vbo);
    if (shader.ebo != 0) glDeleteBuffers(1, &shader.ebo);
    if (shader.prog != 0) glDeleteProgram(shader.prog);
}

void MPVCore::initializeGL() {
    if (media_framebuffer != 0) return;

    brls::Logger::debug("initializeGL1");
    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr};
    int mpv_advanced_control = 1;
    brls::Logger::debug("initializeGL2");
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE,
         const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &mpv_advanced_control},
        {MPV_RENDER_PARAM_INVALID, nullptr}};
    if (mpv_render_context_create(&mpv_context, mpv, params) < 0) {
        mpv_terminate_destroy(mpv);
        brls::fatal("failed to initialize mpv GL context");
    }
    brls::Logger::debug("initializeGL3");

    // set event callback
    mpv_set_wakeup_callback(mpv, on_wakeup, this);
    // set render callback
    mpv_render_context_set_update_callback(mpv_context, on_update, this);

    // create frame buffer
    glGenFramebuffers(1, &this->media_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, this->media_framebuffer);
    glGenTextures(1, &this->media_texture);
    glBindTexture(GL_TEXTURE_2D, this->media_texture);
    brls::Logger::debug("initializeGL4");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, brls::Application::windowWidth,
                 brls::Application::windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           this->media_texture, 0);

    brls::Logger::debug("initializeGL5");
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        brls::Logger::error("glCheckFramebufferStatus failed");
        this->deleteFrameBuffer();
    }

    brls::Logger::debug("initializeGL6");

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
    brls::Logger::debug("initializeGL6.1 start");
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        brls::Logger::debug("initializeGL6.1 {}", infoLog);
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    brls::Logger::debug("initializeGL6.2 start");
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        brls::Logger::debug("initializeGL6.2 {}", infoLog);
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    brls::Logger::debug("initializeGL6.3 start");
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        brls::Logger::debug("initializeGL6.3 {}", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    this->shader.prog = shaderProgram;

    brls::Logger::debug("initializeGL6.4");

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    brls::Logger::debug("initializeGL7");
}

void MPVCore::command_str(const char *args) {
    check_error(mpv_command_string(mpv, args));
}

void MPVCore::command(const char **args) {
    check_error(mpv_command(mpv, args));
}

void MPVCore::command_async(const char **args) {
    check_error(mpv_command_async(mpv, 0, args));
}

void MPVCore::setFrameSize(brls::Rect rect) {
    if (this->media_texture == 0) return;
    int drawWidth  = rect.getWidth() * brls::Application::windowScale;
    int drawHeight = rect.getHeight() * brls::Application::windowScale;

    if (drawWidth == 0 || drawHeight == 0) return;
        // 在没有用到更小的视频时减少对texture的申请
#ifdef __SWITCH__
    if (rect.getWidth() < 400 || rect.getHeight() < 400) return;
#endif
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
}

bool MPVCore::isValid() { return mpv_context != nullptr; }

void MPVCore::openglDraw(brls::Rect rect, float alpha) {
    // if(mpv_context== nullptr || !(mpv_render_context_update(mpv_context) & MPV_RENDER_UPDATE_FRAME))
    // return;

    if (mpv_context == nullptr) return;

    mpv_render_context_render(this->mpv_context, mpv_params);

#ifdef __SWITCH__
    glViewport(0, 0, brls::Application::windowWidth,
               brls::Application::windowHeight);  // restore viewport
#else
    // PC运行可能因为拖拽窗口导致画面比例不是默认的，所以需要重新计算一下宽高
    int realWindowWidth =
        (int)(brls::Application::windowScale * brls::Application::contentWidth);
    int realWindowHeight = (int)(brls::Application::windowScale *
                                 brls::Application::contentHeight);
    glViewport(0, 0, realWindowWidth, realWindowHeight);
#endif

    // shader draw
    glUseProgram(shader.prog);
    glBindTexture(GL_TEXTURE_2D, this->media_texture);
    glBindVertexArray(shader.vao);

    // Set alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform1f(0, alpha);
    //        glUniform1f(glGetUniformLocation(shader.prog, "Alpha"), alpha);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

mpv_render_context *MPVCore::getContext() { return this->mpv_context; }

mpv_handle *MPVCore::getHandle() { return this->mpv; }

MPVEvent *MPVCore::getEvent() { return &this->mpvCoreEvent; }

void MPVCore::eventMainLoop() {
    while (true) {
        auto event = mpv_wait_event(this->mpv, 0);
        //            brls::Logger::error("event: {} / {}", event->event_id, event->reply_userdata);
        switch (event->event_id) {
            case MPV_EVENT_NONE:
                return;
            case MPV_EVENT_SHUTDOWN:
                brls::Logger::debug("========> MPV_EVENT_SHUTDOWN");
#ifdef __SWITCH__
                appletSetMediaPlaybackState(false);
#endif
                return;
            case MPV_EVENT_FILE_LOADED:
                brls::Logger::info("========> MPV_EVENT_FILE_LOADED");
                // event 8: 文件预加载结束，准备解码
                mpvCoreEvent.fire(MpvEventEnum::MPV_LOADED);
                // this->resume();
                break;
            case MPV_EVENT_START_FILE:
                // event 6: 开始加载文件
                brls::Logger::info("========> MPV_EVENT_START_FILE");
#ifdef __SWITCH__
                appletSetMediaPlaybackState(true);
#endif

                // show osd for a really long time
                mpvCoreEvent.fire(MpvEventEnum::START_FILE);

                mpvCoreEvent.fire(MpvEventEnum::LOADING_START);
                break;
            case MPV_EVENT_PLAYBACK_RESTART:
                // event 21: 开始播放文件（一般是播放或调整进度结束之后触发）
                brls::Logger::info("========> MPV_EVENT_PLAYBACK_RESTART");
                mpvCoreEvent.fire(MpvEventEnum::LOADING_END);
                break;
            case MPV_EVENT_END_FILE:
// event 7: 文件播放结束
#ifdef __SWITCH__
                appletSetMediaPlaybackState(false);
#endif
                brls::Logger::info("========> MPV_STOP");
                mpvCoreEvent.fire(MpvEventEnum::MPV_STOP);
                break;
            case MPV_EVENT_PROPERTY_CHANGE:
                switch (event->reply_userdata) {
                    case 1:
                        // 播放器放空了自己，啥也不干的状态
                        // 刚刚启动时、网络不好加载中、seek等都会进入这个状态
                        // 搭配其他值来判定播放器的真实状态
                        // idle = blank
                        // idle && pause = pause
                        // idle && seeking = seeking
                        // idle && unpause = loading
                        core_idle =
                            *(int *)((mpv_event_property *)event->data)->data;
                        brls::Logger::info("========> core-idle: {}",
                                           core_idle);
                        if (isPaused()) {
                            if (fabs(playback_time - duration) < 1 &&
                                duration > 0) {
                                brls::Logger::info(
                                    "========> END OF FILE (paused)");
                                mpvCoreEvent.fire(MpvEventEnum::END_OF_FILE);
                            } else {
                                brls::Logger::info("========> PAUSE");
                                mpvCoreEvent.fire(MpvEventEnum::MPV_PAUSE);
                            }
#ifdef __SWITCH__
                            appletSetMediaPlaybackState(false);
#endif
                        } else {
                            if (core_idle) {
                                if (fabs(playback_time - duration) < 1 &&
                                    duration > 0) {
                                    brls::Logger::info("========> END OF FILE");
                                    this->pause();
                                    mpvCoreEvent.fire(
                                        MpvEventEnum::END_OF_FILE);
                                } else {
                                    brls::Logger::info("========> LOADING");
                                    mpvCoreEvent.fire(
                                        MpvEventEnum::LOADING_START);
                                }
#ifdef __SWITCH__
                                appletSetMediaPlaybackState(false);
#endif
                            } else {
                                // video is playing
                                brls::Logger::info("========> RESUME");
                                mpvCoreEvent.fire(MpvEventEnum::MPV_RESUME);
#ifdef __SWITCH__
                                appletSetMediaPlaybackState(true);
#endif
                            }
                        }
                        break;
                    case 2:
                        // 播放器播放状态改变（暂停或播放）
                        break;
                    case 3:
                        // 视频总时长更新
                        if (((mpv_event_property *)event->data)->data)
                            duration =
                                *(int64_t *)((mpv_event_property *)event->data)
                                     ->data;
                        brls::Logger::debug("========> duration: {}", duration);
                        mpvCoreEvent.fire(MpvEventEnum::UPDATE_DURATION);
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
                            }
                        }

                        break;
                    case 5:
                        // 视频 cache speed
                        if (((mpv_event_property *)event->data)->data) {
                            cache_speed =
                                *(int64_t *)((mpv_event_property *)event->data)
                                     ->data;
                        }

                        brls::Logger::debug("========> cache_speed: {}KB/s ",
                                            cache_speed / 1024);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

void MPVCore::loadDanmakuData(const std::vector<DanmakuItem> &data) {
    this->danmakuData.clear();
    danmakuLoaded = true;
    //todo: 可能出现异步问题
    this->danmakuData = std::move(data);
    mpvCoreEvent.fire(MpvEventEnum::DANMAKU_LOADED);
}

void MPVCore::reset() {
    DanmakuItem::lines = std::vector<std::pair<float, float>>(
        20, std::make_pair<float, float>(0, 0));
    this->danmakuData.clear();
    this->core_idle      = 0;
    this->duration       = 0;  // second
    this->cache_speed    = 0;  // Bps
    this->playback_time  = 0;
    this->video_progress = 0;
    //    this->showDanmaku = true; // todo: 根据配置文件修改
    this->danmakuLoaded = false;
}

/// Danmaku

DanmakuItem::DanmakuItem(const std::string &content, const char *attributes)
    : msg(std::move(content)) {
    auto attrs = pystring::split(attributes, ",", 3);
    time       = atof(attrs[0].c_str());
    type       = atoi(attrs[1].c_str());
    fontSize   = atoi(attrs[2].c_str());
    fontColor  = atoi(attrs[3].c_str());
    int r = (fontColor >> 16) & 0xff, g = (fontColor >> 8) & 0xff,
        b = fontColor & 0xff;
    color = nvgRGBA(r, g, b, 220);
    if ((r * 299 + g * 587 + b * 114) < 60000) {
        borderColor = nvgRGBA(255, 255, 255, 160);
    }
}