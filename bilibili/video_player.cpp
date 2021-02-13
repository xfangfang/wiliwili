
#include "video_player.hpp"

void VideoPlayer::onWindowSizeChanged() {
    brls::Logger::debug("size change:{}-{}",
        brls::Application::windowScale * brls::Application::contentWidth,
        brls::Application::windowScale * brls::Application::contentHeight
    );
    this->setBoundaries(this->getX(),this->getY(),
        (unsigned) roundf(brls::Application::windowScale * brls::Application::contentWidth),
        (unsigned) roundf(brls::Application::windowScale * brls::Application::contentHeight)
    );
    this->initFbo();
}

void VideoPlayer::mpvRender(VideoPlayer* player){
        // uint64_t flags = mpv_render_context_update(player->context);
        // if (!(flags & MPV_RENDER_UPDATE_FRAME))
        // {
        //     return;
        // }

        if(player->mediaFramebufferObject == 0) return;

        mpv_opengl_fbo mpfbo{
                (int) player->mediaFramebufferObject,
                (int) player->getWidth(),
                (int) player->getHeight(),
                GL_RGBA
            };
        int flip_y{0};

        mpv_render_param params[] = {
                        {MPV_RENDER_PARAM_OPENGL_FBO,   &mpfbo},
                        {MPV_RENDER_PARAM_FLIP_Y,       &flip_y},
                        {MPV_RENDER_PARAM_INVALID,      0}
                    };
        mpv_render_context_render(player->context, params);
}

void VideoPlayer::initFbo(){

    if (this->textures != 0){
        glDeleteTextures(1, &this->textures);
    }
    if (this->mediaFramebufferObject == 0){
        glGenFramebuffers(1, &this->mediaFramebufferObject);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, this->mediaFramebufferObject);
    glGenTextures(1, &this->textures);
    glBindTexture(GL_TEXTURE_2D, this->textures);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    #endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)this->getWidth(), (int)this->getHeight(), 0,
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                        this->textures, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    brls::Logger::debug("create fbo and texture done\n");
}

void VideoPlayer::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) {

    // brls::Logger::debug("width:{} height:{}",width,height);
    // int w = (int)(brls::Application::windowScale * width);
    // int h = (int)(brls::Application::windowScale * height);
    mpvRender(this);
    this->imgId = brls::Application::createImageFromTexture(this->textures,(int)width,(int)height, NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
    this->imgPaint = nvgImagePattern(vg, x, y, (int)width, (int)height, 0, this->imgId, this->alpha);
    nvgSave(vg);
    nvgBeginPath(vg);
    nvgRect(vg,x,y,width,height);
    // nvgRoundedRect(vg, x, y, (int)width, (int)height, 0);
    nvgFillPaint(vg, a(this->imgPaint));
    nvgFill(vg);
    nvgRestore(vg);
        
}