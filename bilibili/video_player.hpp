#pragma once

#include <borealis.hpp>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <glad/glad.h>


using namespace brls::i18n::literals;

static void *get_proc_address_mpv(void *unused, const char *name) {
    return (void *)glfwGetProcAddress(name);
}

static inline void check_error(int status)
{
    if (status < 0) {
      brls::Logger::error(mpv_error_string(status));
        // exit(1);
    }
}

class VideoPlayer : public brls::View
{
  protected:
    void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash) override;

  public:
    VideoPlayer();

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;

    void setBackgroundColor(NVGcolor color);

    void setUrl(std::string url);

    void play();

    void pause();

    void seekTo(int second);

    //second
    int getTime();

    int getWholeTime();

    void mpv_render(void *);

    ~VideoPlayer();

  private:
    NVGcolor color = nvgRGB(30, 30, 30);
    std::string url;
    mpv_handle* ctx;
    mpv_render_context* context;
};

VideoPlayer::VideoPlayer()
{
  ctx = mpv_create();
  if (!ctx) {
      brls::Logger::error("failed creating context");
  }

  // Done setting up options.
  mpv_set_option_string(ctx, "terminal", "yes");
  mpv_set_option_string(ctx, "msg-level", "all=info");
  mpv_set_option_string(ctx, "vd-lavc-threads", "4");
  mpv_set_option_string(ctx, "vd-lavc-fast", "yes");
  mpv_set_option_string(ctx, "vd-lavc-skiploopfilter", "all");
  mpv_set_option_string(ctx, "background", "#EBEBEB");

  mpv_set_option_string(ctx, "audio-channels", "stereo");
  check_error(mpv_initialize(ctx));
  mpv_opengl_init_params gl_init_params{ get_proc_address_mpv, nullptr, nullptr};
  mpv_render_param params[]{
          {MPV_RENDER_PARAM_API_TYPE,           const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
          {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
          {MPV_RENDER_PARAM_INVALID,            nullptr}
  };

  check_error(mpv_render_context_create(&context, ctx, params));

}

void VideoPlayer::mpv_render(void* param) {
  // render mpv


}

VideoPlayer::~VideoPlayer() {
  if (context) {
    mpv_render_context_free(context);
  }
  if (ctx) {
    mpv_terminate_destroy(ctx);
  }
}




void VideoPlayer::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
  NVGcolor color = a(this->color);

  if (color.a == 0.0f)
      return;

  // nvgBeginPath(vg);
  // nvgRect(vg, x, y, width/2, height/2);
  // nvgFillColor(vg, color);
  // nvgFill(vg);

  // nvgBeginPath(vg);
  // nvgCircle(vg, x+100, y+100, 100);
  // nvgFillColor(vg, nvgRGBA(255,192,0,255));
  // nvgFill(vg);


  mpv_opengl_fbo fbo = {
    .fbo = 0,
    .w = (int)brls::Application::contentWidth,
    .h = (int)brls::Application::contentHeight,
  };
  int flip_y{1};
  mpv_render_param params[] = {{MPV_RENDER_PARAM_OPENGL_FBO,  &fbo},
                               {MPV_RENDER_PARAM_FLIP_Y,      &flip_y},
                               {MPV_RENDER_PARAM_INVALID,     nullptr}};
  mpv_render_context_render(context, params);

}

void VideoPlayer::setUrl(std::string url)
{
  // this->url = url;
}

void VideoPlayer::play()
{

  brls::Application::notify("start...");
  check_error(mpv_command_string(this->ctx, "set video-margin-ratio-left 0.2"));
  check_error(mpv_command_string(this->ctx, "set video-margin-ratio-top 0.12"));
  check_error(mpv_command_string(this->ctx, "set video-margin-ratio-bottom 0.1"));

  // Play this file.
  // check_error(mpv_command_string(ctx, "set demuxer-lavf-format flv"));
  // const char *cmd[] = {"loadfile", "http://samples.ffmpeg.org/FLV/11-04-2008.flv", "replace", "speed=1",nullptr};
  const char *cmd[] = {"loadfile", "https://www.bilibili.com/video/BV1q7411p7no", NULL};
  // const char *cmd[] = {"loadfile", BOREALIS_ASSET("test.flv"),NULL};
  // check_error(mpv_command_string(ctx, "set demuxer-lavf-format flv"));
  check_error(mpv_command(ctx, cmd));
}

void VideoPlayer::pause()
{

}

void VideoPlayer::seekTo(int second)
{

}

int VideoPlayer::getTime()
{
  return 100;
}

int VideoPlayer::getWholeTime()
{
  return 1000;
}

void VideoPlayer::setBackgroundColor(NVGcolor color)
{
    this->color = color;
}


void VideoPlayer::layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash)
{
    // Nothing to do
}
