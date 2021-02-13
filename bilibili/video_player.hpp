#pragma once

#include <borealis.hpp>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <glad/glad.h>
#include <thread>
#include <math.h>
#ifdef __SWITCH__
    #include <switch.h>
#endif

using namespace brls::i18n::literals;

static void *get_proc_address_mpv(void *unused, const char *name) {
    return (void *)glfwGetProcAddress(name);
}

static inline void check_error(int status)
{
    if (status < 0) {
        brls::Application::notify(mpv_error_string(status));
        brls::Logger::error(mpv_error_string(status));
    }
}

typedef enum VideoState{
    PLAYING,
    STOPPED,
    WAITTING_STOP,
    PAUSED,
} VideoState;

class VideoPlayer : public brls::View
{
  public:
    VideoPlayer(){
        onWindowSizeChanged();
        handle = mpv_create();
        if (!handle) {
            brls::Logger::error("failed creating context");
        }
        // Done setting up options.
        mpv_set_option_string(handle, "terminal", "yes");
        mpv_set_option_string(handle, "msg-level", "all=v");
        mpv_set_option_string(handle, "vd-lavc-threads", "4");
        mpv_set_option_string(handle, "vd-lavc-fast", "yes");
        mpv_set_option_string(handle, "vd-lavc-skiploopfilter", "all");
        mpv_set_option_string(handle, "audio-channels", "stereo");
        // mpv_set_option_string(handle, "vo", "gpu");
        // mpv_set_option_string(handle, "demuxer-lavf-format","flv");
        // mpv_set_option_string(handle, "demuxer-lavf-probescore","24");
        // mpv_set_option_string(handle, "demuxer-lavf-probe-info", "nostreams");
        // mpv_set_option_string(handle, "demuxer-lavf-analyzeduration", "1");
        // mpv_set_option_string(handle, "video-timing-offset", "0");

        // mpv_set_option_string(handle, "background", "#EB0000");
        mpv_set_option_string(handle, "user-agent","wiliwili/0.1(NintendoSwitch)");
        mpv_set_option_string(handle, "referrer","http://www.bilibili.com/");
        mpv_set_option_string(handle, "network-timeout","16");
        // mpv_set_option_string(handle, "cache-pause-wait","1");
        // mpv_set_option_string(handle, "cache-secs","20");
        // mpv_set_option_string(handle, "demuxer-readahead-secs","20");
        // mpv_set_option_string(handle, "cache-pause-initial", "yes");
        // mpv_set_option_string(handle, "loop-playlist", "inf");
        // mpv_set_option_string(handle, "prefetch-playlist", "yes");
        mpv_set_option_string(handle,"idle","yes");
        // merge-files
        // mpv_set_option_string(handle, "config", "no");

        check_error(mpv_initialize(handle));
        mpv_opengl_init_params gl_init_params{ get_proc_address_mpv, nullptr, nullptr};
        // int mpv_advanced_control = 1;
        mpv_render_param params[]{
                {MPV_RENDER_PARAM_API_TYPE,           const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
                {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
                // {MPV_RENDER_PARAM_ADVANCED_CONTROL,   &mpv_advanced_control},
                {MPV_RENDER_PARAM_INVALID,            nullptr}
        };
        check_error(mpv_render_context_create(&context, handle, params));
        check_error(mpv_observe_property(handle, 0, "core-idle", MPV_FORMAT_FLAG));
        check_error(mpv_observe_property(handle, 0, "pause", MPV_FORMAT_FLAG));
        check_error(mpv_observe_property(handle, 0, "duration", MPV_FORMAT_DOUBLE));
        check_error(mpv_observe_property(handle, 0, "playback-time", MPV_FORMAT_DOUBLE));
        check_error(mpv_observe_property(handle, 0, "eof-reached", MPV_FORMAT_FLAG));
        check_error(mpv_observe_property(handle, 0, "track-list", MPV_FORMAT_NODE));
        check_error(mpv_observe_property(handle, 0, "chapter-list", MPV_FORMAT_NODE));

        initFbo();
        // this->mpvRenderThread = new std::thread(mpvRender, this);
    }

    void onWindowSizeChanged() override;

    static void mpvRender(VideoPlayer* player);
    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override ;
    void initFbo();

    void resume(){
        check_error(mpv_command_string(handle,"set pause no"));
        this->videoState = VideoState::PAUSED;
        #ifdef __SWITCH__
            appletSetMediaPlaybackState(true);
        #endif
        brls::Logger::debug("resume");
    }

    void pause(){
        brls::Logger::debug("pause");
        check_error(mpv_command_string(handle,"set pause yes"));
        this->videoState = VideoState::PAUSED;
        #ifdef __SWITCH__
            appletSetMediaPlaybackState(false);
        #endif
    }

    void stop(){
        if (!this->getStop()){
            check_error(mpv_command_string(handle,"stop"));
            this->videoState = VideoState::STOPPED;
            brls::Logger::debug("real stop");
        }
    }

    //TODO seek playlist 
    void seek(int sec){
      std::string cmd = "no-osd seek "+std::to_string(sec) + " absolute";
      check_error(mpv_command_string(handle, cmd.c_str()));
    }

    long getPosition() {
        long position = 0;
        mpv_get_property(handle, "playback-time", MPV_FORMAT_INT64, &position);
        return position;
    }

    long getDuration() {
        long duration = 0;
        mpv_get_property(handle, "duration", MPV_FORMAT_INT64, &duration);
        return duration;
    }

    int getPause() {
        return getFlagProperty("pause");
    }

    int getStop() {
        return getFlagProperty("playback-abort");
    }

    int getCore() {
        int pause;
        mpv_get_property(handle,"core-idle",MPV_FORMAT_FLAG, &pause);
        return pause;
    }

    mpv_event *getEvent() {
        return mpv_wait_event(handle, 0);
    }

    long getLongProperty(std::string property){
        long temp;
        mpv_get_property(handle, property.c_str(), MPV_FORMAT_INT64, &temp);
        return temp;
    }

    int getFlagProperty(std::string property){
        int temp;
        mpv_get_property(handle, property.c_str(), MPV_FORMAT_FLAG, &temp);
        return temp;
    }

    void debug(){
        brls::Logger::debug("video player info");
        brls::Logger::debug("   position/duration {}/{}",this->getPosition(),this->getDuration());
        brls::Logger::debug("   speed {}KB/s",this->getLongProperty("cache-speed")*1.0/1024);
        brls::Logger::debug("   buffering {}%",this->getLongProperty("cache-buffering-state"));
        brls::Logger::debug("   threads {}",this->getLongProperty("vd-lavc-threads"));
        brls::Logger::debug("   probescore {}",this->getLongProperty("demuxer-lavf-probescore"));
        // brls::Logger::debug("   vo {}",this->getLongProperty("vo"));
        mpv_command_string(handle,"list-options");
    }

    void startPlayList(){
        check_error(mpv_command_string(handle,"set playlist-pos 0"));
        this->resume();
        brls::Logger::debug("start play");
    }

    void setPlayList(std::vector<std::string> list){
        check_error(mpv_command_string(handle,"playlist-clear"));
        if (list.size() > 0){
            const char *cmd[] = {"loadfile", list[0].c_str(), "replace", NULL};
            check_error(mpv_command(handle, cmd));
        }
        for(size_t i = 1; i < list.size(); i++){
            const char *cmd[] = {"loadfile", list[i].c_str(), "append", NULL};
            check_error(mpv_command(handle, cmd));
        }
    }

    VideoState getState(){
        return this->videoState;
    }
    
    ~VideoPlayer(){
        check_error(mpv_unobserve_property(handle, 0));
        check_error(mpv_command_string(handle,"quit"));
        this->mpvRenderRun = false;
        if (this->mpvRenderThread != nullptr) {
            this->mpvRenderThread->join();
        }
        if (this->mediaFramebufferObject != 0){
            glDeleteFramebuffers(1, &this->mediaFramebufferObject);
        }
        if (this->textures != 0){
            glDeleteTextures(1, &this->textures);
        }
        if (this->context) {
            mpv_render_context_free(this->context);
        }
        if (this->handle) {
            mpv_terminate_destroy(this->handle);
        }
        #ifdef __SWITCH__
            appletSetMediaPlaybackState(false);
        #endif
        brls::Logger::debug("delete video player");
    }

    private:
        mpv_handle* handle;
        mpv_render_context* context;
        VideoState videoState = VideoState::STOPPED;
        GLuint mediaFramebufferObject = 0;
        GLuint textures = 0;
        NVGpaint imgPaint;
        int imgId;
        std::thread *mpvRenderThread = nullptr;
        bool mpvRenderRun = true;

};
