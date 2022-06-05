//
// Created by fang on 2022/4/23.
//

#include "view/video_view.hpp"
#ifdef __SWITCH__
#include <switch.h>
#endif

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
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = texture(ourTexture, TexCoord);\n"
                                   "}\n\0";

static void *get_proc_address(void *unused, const char *name) {
    glfwGetCurrentContext();
    return (void *)glfwGetProcAddress(name);
}
VideoView::VideoView() {
    this->inflateFromXMLRes("xml/views/video_view.xml");
    this->setHideHighlightBackground(true);
    this->mpv = mpv_create();
    if (!mpv)
        fatal("could not create mpv context");
    mpv_set_option_string(mpv, "terminal", "yes");
    mpv_set_option_string(mpv, "msg-level", "all=v");
    mpv_set_option_string(mpv, "vd-lavc-threads", "4");
    mpv_set_option_string(mpv, "vd-lavc-fast", "yes");
    mpv_set_option_string(mpv, "vo", "libmpv");
    mpv_set_option_string(mpv, "vd-lavc-skiploopfilter", "all");
    mpv_set_option_string(mpv, "audio-channels", "stereo");
    mpv_set_option_string(mpv, "user-agent","wiliwili/0.1(NintendoSwitch)");
    mpv_set_option_string(mpv, "referrer","http://www.bilibili.com/");
    mpv_set_option_string(mpv, "network-timeout","16");
    mpv_set_option_string(mpv,"idle","yes");

//        mpv_set_option_string(mpv, "vid", "no");
//        mpv_set_option_string(mpv, "aid", "no");
//        mpv_set_option_string(mpv, "sid", "no");
//        mpv_set_option_string(mpv, "vo", "null");
//        mpv_set_option_string(mpv, "ao", "null");


    if(mpv_initialize(mpv) < 0)
        fatal("could not initialize mpv context");

    check_error(mpv_observe_property(mpv, 0, "core-idle", MPV_FORMAT_FLAG));
    check_error(mpv_observe_property(mpv, 0, "pause", MPV_FORMAT_FLAG));
    check_error(mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE));
    check_error(mpv_observe_property(mpv, 0, "playback-time", MPV_FORMAT_DOUBLE));
    check_error(mpv_observe_property(mpv, 0, "eof-reached", MPV_FORMAT_FLAG));
    check_error(mpv_observe_property(mpv, 0, "track-list", MPV_FORMAT_NODE));
    check_error(mpv_observe_property(mpv, 0, "chapter-list", MPV_FORMAT_NODE));

    mpv_set_wakeup_callback(mpv, [](void *)->void{
        Logger::debug("on mpv events");
    }, this);

    Logger::debug("initializeGL");
    this->initializeGL();
}

void VideoView::initializeGL(){
    if (media_framebuffer != 0)
        return;

    mpv_opengl_init_params gl_init_params{ get_proc_address, nullptr};
    // int mpv_advanced_control = 1;
    mpv_render_param params[]{
            {MPV_RENDER_PARAM_API_TYPE,           const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
            {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
            // {MPV_RENDER_PARAM_ADVANCED_CONTROL,   &mpv_advanced_control},
            {MPV_RENDER_PARAM_INVALID,            nullptr}
    };
    if(mpv_render_context_create(&mpv_context, mpv, params)<0)
        fatal("failed to initialize mpv GL context");
    mpv_render_context_set_update_callback(mpv_context, on_update, this);

    // create frame buffer
    glGenFramebuffers(1, &this->media_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, this->media_framebuffer);
    glGenTextures(1, &this->media_texture);
    glBindTexture(GL_TEXTURE_2D, this->media_texture);

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

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Logger::error("glCheckFramebufferStatus failed");
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

VideoView::~VideoView(){
    Logger::debug("trying delete VideoView...");

    check_error(mpv_unobserve_property(mpv, 0));
    check_error(mpv_command_string(mpv,"quit"));

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
        mpv_terminate_destroy(this->mpv);
        this->mpv = nullptr;
    }

#ifdef __SWITCH__
    appletSetMediaPlaybackState(false);
#endif
    brls::Logger::error("Delete VideoView done");
}

void VideoView::deleteFrameBuffer() {
    if (this->media_framebuffer != 0){
        glDeleteFramebuffers(1, &this->media_framebuffer);
        this->media_framebuffer = 0;
    }
    if (this->media_renderbuffer != 0){
        glDeleteRenderbuffers(1, &this->media_renderbuffer);
        this->media_renderbuffer = 0;
    }
    if (this->media_texture != 0){
        glDeleteTextures(1, &this->media_texture);
        this->media_texture = 0;
    }
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
    if(mpv_context== nullptr)
        return;

//    int drawWidth = (int) (Application::windowScale * width);
//    int drawHeight = (int) (Application::windowScale * height);
//    int drawX = (int) (Application::windowScale * x);
//    int drawY = (int) (Application::windowScale * (Application::contentHeight - y - height));
    int realWindowWidth = (int)(Application::windowScale * Application::contentWidth);
    int realWindowHeight = (int)(Application::windowScale * Application::contentHeight);

    // nvg draw
    nvgResetTransform(vg);
    nvgEndFrame(vg);
    mpv_render_context_render(this->mpv_context, mpv_params);
    glViewport(0, 0, realWindowWidth, realWindowHeight); // restore viewport


//    todo: 判断哪种实现方式效率更高
//    method1: drawing texture to default Framebuffer
    // change layout
    Rect rect = getFrame();
    float new_min_x = rect.getMinX() / Application::contentWidth * 2 - 1;
    float new_min_y = 1 - rect.getMinY() / Application::contentHeight * 2;
    float new_max_x = rect.getMaxX() / Application::contentWidth * 2 - 1;
    float new_max_y = 1 - rect.getMaxY() / Application::contentHeight * 2;

//    if(this->fullscreen){
//        new_min_x = -1;
//        new_min_y = 1;
//        new_max_x = 1;
//        new_max_y = -1;
//    }

    float vertices[] = {
            new_max_x, new_min_y, 0.0f, 1.0f, 1.0f, //右上
            new_max_x, new_max_y, 0.0f, 1.0f, 0.0f, //右下
            new_min_x, new_max_y, 0.0f, 0.0f, 0.0f, //左下
            new_min_x, new_min_y, 0.0f, 0.0f, 1.0f //左上
    };
    glBindBuffer(GL_ARRAY_BUFFER, this->shader.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // load shader
    glUseProgram(shader.prog);
    glBindTexture(GL_TEXTURE_2D, this->media_texture);
    glBindVertexArray(shader.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

//    method2: using glBlitFramebuffer to render video
//    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->media_framebuffer);
//    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//    glBlitFramebuffer(0,
//                      0,
//                      drawWidth,
//                      drawHeight,
//                      drawX,
//                      drawY,
//                      drawX + drawWidth,
//                      drawY + drawHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

void VideoView::deleteShader() {
    if (shader.vao !=  0)
        glDeleteVertexArrays(1, &shader.vao);
    if (shader.vbo != 0)
        glDeleteBuffers(1, &shader.vbo);
    if (shader.ebo != 0)
        glDeleteBuffers(1, &shader.ebo);
    if (shader.prog != 0)
        glDeleteProgram(shader.prog);
}

void VideoView::invalidate() {
    View::invalidate();

    Rect rect = getFrame();
    Logger::error("invalidate: {}", rect.describe());
    if(int(rect.getWidth()) == 0 || int(rect.getHeight()) == 0 || this->media_texture == 0)
        return;

    //    change texture size
    int drawWidth = (int) (Application::windowScale * rect.getWidth());
    int drawHeight = (int) (Application::windowScale * rect.getHeight());
    glBindTexture(GL_TEXTURE_2D, this->media_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, drawWidth, drawHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    this->mpv_fbo.h = drawHeight;
    this->mpv_fbo.w = drawWidth;

}
