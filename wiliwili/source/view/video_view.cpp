//
// Created by fang on 2022/4/23.
//

#include "view/video_view.hpp"

#include <future>
#include <thread>
#include <iostream>

#ifdef __SWITCH__
#include <switch.h>
#endif

using namespace brls;

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec2 aTexCoord;\n"
                                 "out vec2 TexCoord;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos, 1.0);\n"
                                 "   TexCoord = aTexCoord;\n"
                                 "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
                                   "in vec2 TexCoord;\n"
                                   "out vec4 FragColor;\n"
                                   "uniform sampler2D ourTexture;\n"
                                   "uniform float Alpha = 1.0;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = texture(ourTexture, TexCoord);\n"
                                   "   FragColor.a = Alpha;\n"
                                   "}\n\0";

static void *get_proc_address(void *unused, const char *name) {
    glfwGetCurrentContext();
    return (void *)glfwGetProcAddress(name);
}


class MPVCore: public Singleton<MPVCore>{
public:

    MPVCore(){
        this->mpv = mpv_create();
        if (!mpv){
            fatal("Error Create mpv Handle");
        }

        mpv_set_option_string(mpv, "terminal", "yes");
        mpv_set_option_string(mpv, "msg-level", "all=v");
        mpv_set_option_string(mpv, "vd-lavc-threads", "4");
//        mpv_set_option_string(mpv, "vo", "libmpv");
        mpv_set_option_string(mpv, "audio-channels", "stereo");
        mpv_set_option_string(mpv, "user-agent","wiliwili/0.1(NintendoSwitch)");
        mpv_set_option_string(mpv, "referrer","https://www.bilibili.com/");
        mpv_set_option_string(mpv, "network-timeout","16");
        mpv_set_option_string(mpv, "idle","yes");
        mpv_set_option_string(mpv, "opengl-pbo", "yes");
        mpv_set_option_string(mpv, "fbo-format", "rgba8");

        // Less cpu cost
        mpv_set_option_string(mpv, "vd-lavc-skiploopfilter", "all");
        mpv_set_option_string(mpv, "vd-lavc-fast", "yes");

        // Making the loading process faster
        mpv_set_option_string(mpv, "demuxer-lavf-analyzeduration", "1");
        mpv_set_option_string(mpv, "demuxer-lavf-probescore", "24");
        mpv_set_option_string(mpv, "cache-secs", "60");
//        mpv_set_option_string(mpv, "msg-level", "ffmpeg=trace");

        if(mpv_initialize(mpv) < 0){
            fatal("Could not initialize mpv context");
        }

        check_error(mpv_observe_property(mpv, 1, "core-idle", MPV_FORMAT_FLAG));
        check_error(mpv_observe_property(mpv, 2, "pause", MPV_FORMAT_FLAG));
        check_error(mpv_observe_property(mpv, 3, "duration", MPV_FORMAT_DOUBLE));
        check_error(mpv_observe_property(mpv, 4, "playback-time", MPV_FORMAT_DOUBLE));
//        check_error(mpv_observe_property(mpv, 5, "eof-reached", MPV_FORMAT_FLAG));
//        check_error(mpv_observe_property(mpv, 6, "track-list", MPV_FORMAT_NODE));
//        check_error(mpv_observe_property(mpv, 7, "chapter-list", MPV_FORMAT_NODE));

//cache-speed

        mpv_set_wakeup_callback(mpv, on_wakeup, this);

        Logger::debug("initializeGL");
        this->initializeGL();
    }

    ~MPVCore(){
        check_error(mpv_command_string(this->mpv,"quit"));

        Logger::debug("trying delete fbo and shader");

        this->deleteFrameBuffer();
        this->deleteShader();

        Logger::debug("trying free mpv context");
        if (this->mpv_context) {
            mpv_render_context_free(this->mpv_context);
            this->mpv_context = nullptr;
        }

        Logger::debug("trying terminate mpv");
        if (this->mpv) {
            mpv_destroy(this->mpv);
            this->mpv = nullptr;
        }
    }

    static void on_update(void *self){
//        brls::sync([](){
//            MPVCore::instance().event_mainloop();
//        });
    }

    static void on_wakeup(void *self){
//        Logger::debug("on mpv events");
        brls::sync([](){
            MPVCore::instance().eventMainLoop();
        });
    }

    void deleteFrameBuffer() {
        if (this->media_framebuffer != 0){
            glDeleteFramebuffers(1, &this->media_framebuffer);
            this->media_framebuffer = 0;
        }
        if (this->media_texture != 0){
            glDeleteTextures(1, &this->media_texture);
            this->media_texture = 0;
        }
    }

    void deleteShader() {
        if (shader.vao !=  0)
            glDeleteVertexArrays(1, &shader.vao);
        if (shader.vbo != 0)
            glDeleteBuffers(1, &shader.vbo);
        if (shader.ebo != 0)
            glDeleteBuffers(1, &shader.ebo);
        if (shader.prog != 0)
            glDeleteProgram(shader.prog);
    }

    void initializeGL(){
        if (media_framebuffer != 0)
            return;

        Logger::debug("initializeGL1");
        mpv_opengl_init_params gl_init_params{ get_proc_address, nullptr};
        // int mpv_advanced_control = 1;
        Logger::debug("initializeGL2");
        mpv_render_param params[]{
                {MPV_RENDER_PARAM_API_TYPE,           const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
                {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
                // {MPV_RENDER_PARAM_ADVANCED_CONTROL,   &mpv_advanced_control},
                {MPV_RENDER_PARAM_INVALID,            nullptr}
        };
        if(mpv_render_context_create(&mpv_context, mpv, params)<0)
            fatal("failed to initialize mpv GL context");
        mpv_render_context_set_update_callback(mpv_context, on_update, this);
        Logger::debug("initializeGL3");
        // create frame buffer
        glGenFramebuffers(1, &this->media_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, this->media_framebuffer);
        glGenTextures(1, &this->media_texture);
        glBindTexture(GL_TEXTURE_2D, this->media_texture);
        Logger::debug("initializeGL4");

        //todo remove hardcode size
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     Application::contentWidth * Application::windowScale,
                     Application::contentHeight * Application::windowScale,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->media_texture, 0);

        Logger::debug("initializeGL5");
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Logger::error("glCheckFramebufferStatus failed");
            this->deleteFrameBuffer();
        }

        Logger::debug("initializeGL6");

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
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        }
        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
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
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        this->shader.prog = shaderProgram;

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
                0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
                0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
                -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
                -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
        };
        unsigned int indices[] = {
                0, 1, 3,
                1, 2, 3
        };
        glGenVertexArrays(1, &this->shader.vao);
        glGenBuffers(1, &this->shader.vbo);
        glGenBuffers(1, &this->shader.ebo);
        glBindVertexArray(this->shader.vao);

        glBindBuffer(GL_ARRAY_BUFFER, this->shader.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->shader.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

    }

    void command_str(const char *args){
        check_error(mpv_command_string(mpv, args));
    }

    void command(const char **args){
        check_error(mpv_command(mpv, args));
    }

    void setFrameSize(int drawWidth, int drawHeight){
        if(this->media_texture == 0)
            return;
        glBindTexture(GL_TEXTURE_2D, this->media_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, drawWidth, drawHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        this->mpv_fbo.w = drawWidth;
        this->mpv_fbo.h = drawHeight;
    }

    bool isValid(){
        return mpv_context != nullptr;
    }

    void openglDraw(Rect rect, float alpha=1.0){
        if(!mpv_context)
            return;

        int realWindowWidth = (int)(Application::windowScale * Application::contentWidth);
        int realWindowHeight = (int)(Application::windowScale * Application::contentHeight);

        mpv_render_context_render(this->mpv_context, mpv_params);
        glViewport(0, 0, realWindowWidth, realWindowHeight); // restore viewport


        float new_min_x = rect.getMinX() / Application::contentWidth * 2 - 1;
        float new_min_y = 1 - rect.getMinY() / Application::contentHeight * 2;
        float new_max_x = rect.getMaxX() / Application::contentWidth * 2 - 1;
        float new_max_y = 1 - rect.getMaxY() / Application::contentHeight * 2;

        float vertices[] = {
                new_max_x, new_min_y, 0.0f, 1.0f, 1.0f, //右上
                new_max_x, new_max_y, 0.0f, 1.0f, 0.0f, //右下
                new_min_x, new_max_y, 0.0f, 0.0f, 0.0f, //左下
                new_min_x, new_min_y, 0.0f, 0.0f, 1.0f //左上
        };
        glBindBuffer(GL_ARRAY_BUFFER, this->shader.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // shader draw
        glUseProgram(shader.prog);
        glBindTexture(GL_TEXTURE_2D, this->media_texture);
        glBindVertexArray(shader.vao);
        if(alpha < 1.0){
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            GLuint alphaLayout = glGetUniformLocation(shader.prog, "Alpha");
            brls::Logger::error("========> {}", alphaLayout);
            if(alphaLayout != -1){
                glUniform1f(alphaLayout, alpha);
            }
        }
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

private:
    mpv_handle* mpv = nullptr;
    mpv_render_context* mpv_context = nullptr;

    GLuint media_framebuffer = 0;
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


    /// Will be called in main thread to get events from mpv core
    void eventMainLoop() {
        while(true){
            auto event = mpv_wait_event(this->mpv, 0);
//            brls::Logger::error("event: {} / {}", event->event_id, event->reply_userdata);
            switch (event->event_id) {
                case MPV_EVENT_SHUTDOWN:
                        brls::Logger::debug("========> MPV_EVENT_SHUTDOWN");
                    return;
                case MPV_EVENT_NONE:
                    return;
                case MPV_EVENT_FILE_LOADED:
                    // event 8: 文件预加载结束，准备解码
                    break;
                case MPV_EVENT_START_FILE:
                    // event 6: 开始加载文件
                    break;
                case MPV_EVENT_PLAYBACK_RESTART:
                    // event 21: 开始播放文件（一般是播放或调整进度结束之后触发）
                    break;
                case MPV_EVENT_END_FILE:
                    // event 7: 文件播放结束
                    break;
                case MPV_EVENT_PROPERTY_CHANGE:
//                    check_error(mpv_observe_property(mpv, 1, "core-idle", MPV_FORMAT_FLAG));
//                    check_error(mpv_observe_property(mpv, 2, "pause", MPV_FORMAT_FLAG));
//                    check_error(mpv_observe_property(mpv, 3, "duration", MPV_FORMAT_DOUBLE));
//                    check_error(mpv_observe_property(mpv, 4, "playback-time", MPV_FORMAT_DOUBLE));
                    switch(event->reply_userdata){
                        case 1:
                            // 播放器放空了自己，啥也不干的状态
                            // 刚刚启动时、网络不好加载中、seek等都会进入这个状态
                            // 搭配其他值来判定播放器的真实状态
                            // idle = blank
                            // idle && pause = pause
                            // idle && seeking = seeking
                            // idle && unpause = loading
                            break;
                        case 2:
                            // 播放器播放状态改变（暂停或播放）
                            break;
                        case 3:
                            // 视频总时长更新
                            break;
                        case 4:
                            // 播放进度更新
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
};


VideoView::VideoView() {
    mpvCore = &MPVCore::instance();
    this->inflateFromXMLRes("xml/views/video_view.xml");
    this->setHideHighlightBackground(true);
}

VideoView::~VideoView(){
    Logger::debug("trying delete VideoView...");

    Logger::debug("appletSetMediaPlaybackState");

#ifdef __SWITCH__
    appletSetMediaPlaybackState(false);
#endif
    brls::Logger::error("Delete VideoView done");
}

void VideoView::draw(NVGcontext *vg, float x, float y, float width, float height, Style style, FrameContext *ctx) {

//    nvgFillColor(vg, nvgRGBf(0,0,0));
//    nvgBeginPath(vg);
//    nvgRect(vg, x, y, width, height);
//    nvgFill(vg);

//    nvgBeginPath(vg);
//    nvgStrokeColor(vg, nvgRGBAf(1,0,0,0.5));
//    nvgStrokeWidth(vg, 10);
//    nvgRoundedRect(vg, x, y, width, height, 2);
//    nvgStroke(vg);

//    if(mpv_context== nullptr || !(mpv_render_context_update(mpv_context) & MPV_RENDER_UPDATE_FRAME))
//        return;

    if(!mpvCore->isValid())
        return;

    int realWindowWidth = (int)(Application::windowScale * Application::contentWidth);
    int realWindowHeight = (int)(Application::windowScale * Application::contentHeight);

    // nvg draw
    nvgResetTransform(vg);
    nvgEndFrame(vg);

    mpvCore->openglDraw(this->getFrame(), this->getAlpha());

    // restore nvg
    nvgBeginFrame(vg,
                  (float)realWindowWidth,
                  (float)realWindowHeight,
                  Application::contentWidth / Application::contentHeight);
    nvgScale(vg, Application::windowScale, Application::windowScale);


    // draw osd
    if(unix_time() < this->osdLastShowTime || this->videoState != VideoState::PLAYING){
        Box::draw(vg, x, y, width, height, style, ctx);
    }
}

void VideoView::invalidate() {
    View::invalidate();

    Rect rect = getFrame();
    Logger::error("VideoView::invalidate: {}", rect.describe());
    if(int(rect.getWidth()) == 0 || int(rect.getHeight()) == 0)
        return;
    //todo: 检查是否和之前的长度宽度一致

    //    change texture size
    int drawWidth = (int) (Application::windowScale * rect.getWidth());
    int drawHeight = (int) (Application::windowScale * rect.getHeight());

    brls::Logger::debug("Video view size: {} / {}", drawWidth, drawHeight);
    this->mpvCore->setFrameSize(drawWidth, drawHeight);

}

void VideoView::setUrl(std::string url){
    const char *cmd[] = {"loadfile", url.c_str(), "replace", NULL};
    mpvCore->command(cmd);
}

void VideoView::resume(){
    //todo: 此处设置为 loading
    this->videoState = VideoState::PLAYING;
    mpvCore->command_str("set pause no");
#ifdef __SWITCH__
    appletSetMediaPlaybackState(true);
#endif
    brls::Logger::error("VideoView::resume");
}

void VideoView::pause(){
    this->videoState = VideoState::PAUSED;
    mpvCore->command_str("set pause yes");
#ifdef __SWITCH__
    appletSetMediaPlaybackState(false);
#endif
    brls::Logger::error("VideoView::pause");
}

void VideoView::stop(){
    mpvCore->command_str("stop");
#ifdef __SWITCH__
    appletSetMediaPlaybackState(false);
#endif
    brls::Logger::error("VideoView::stop");
}