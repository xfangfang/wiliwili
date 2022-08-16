//
// Created by fang on 2022/4/23.
//

#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "view/video_progress_slider.hpp"
#include "utils/number_helper.hpp"
#include <fmt/core.h>

#ifdef __SWITCH__
#include <switch.h>
#endif

using namespace brls;


VideoView::VideoView() {
    mpvCore = &MPVCore::instance();
    this->inflateFromXMLRes("xml/views/video_view.xml");
    this->setHideHighlightBackground(true);

    this->registerBoolXMLAttribute("allowFullscreen", [this](bool value){
        this->allowFullscreen = value;
        if(!value){
            this->btnFullscreen->setVisibility(brls::Visibility::GONE);
        }
    });

    this->registerAction("-10s", brls::ControllerButton::BUTTON_LB, [this](brls::View* view)-> bool {
        mpvCore->command_str("seek -10");
        return true;
    }, false, true);

    this->registerAction("+10s", brls::ControllerButton::BUTTON_RB, [this](brls::View* view)-> bool {
        mpvCore->command_str("seek +10");
        return true;
    }, false, true);

    this->registerAction("toggleOSD", brls::ControllerButton::BUTTON_Y, [this](brls::View* view)-> bool {
        if(isOSDShown()){
            this->hideOSD();
        } else {
            this->showOSD(true);
        }
        return true;
    }, true);

    eventSubscribeID = mpvCore->getEvent()->subscribe([this](MpvEventEnum event){
        // brls::Logger::info("mpv event => : {}", event);
        switch (event){
            case MpvEventEnum::MPV_RESUME:
                this->showOSD(true);
                break;
            case MpvEventEnum::MPV_PAUSE:
                this->showOSD(false);
                break;
            case MpvEventEnum::START_FILE:
                this->showOSD(false);
                rightStatusLabel->setText("00:00");
                leftStatusLabel->setText("00:00");
                osdSlider->setProgress(0);
                break;
            case MpvEventEnum::LOADING_START:
                this->showLoading();
                break;
            case MpvEventEnum::LOADING_END:
                this->hideLoading();
                break;
            case MpvEventEnum::MPV_STOP:
                // todo: 当前播放结束，尝试播放下一个视频
                this->hideLoading();
                this->showOSD(false);
                break;
            case MpvEventEnum::MPV_LOADED:
                break;
            case MpvEventEnum::UPDATE_DURATION:
                rightStatusLabel->setText(wiliwili::sec2Time(mpvCore->duration));
                break;
            case MpvEventEnum::UPDATE_PROGRESS:
                leftStatusLabel->setText(wiliwili::sec2Time(mpvCore->video_progress));
                // osdSlider->setProgress(mpvCore->playback_time / mpvCore->duration);
                break;
            default:
                break;
        }
    });

    osdSlider->getProgressEvent()->subscribe([this](float progress){
        brls::Logger::info("process: {}", progress);
        //todo: less call
        //todo: wakeup osd
        mpvCore->command_str(fmt::format("seek {} absolute-percent", progress * 100).c_str());
    });

    this->addGestureRecognizer(new brls::TapGestureRecognizer(this, [this](){
        if(isOSDShown()){
            this->hideOSD();
        } else {
            this->showOSD(true);
        }
    }));

    if(allowFullscreen){
        this->btnFullscreen->addGestureRecognizer(new brls::TapGestureRecognizer(this->btnFullscreen, [this](){
            if(this->isFullscreen()){
                this->setFullScreen(false);
            } else {
                this->setFullScreen(true);
            }
        }, brls::TapGestureConfig(false, brls::SOUND_NONE, brls::SOUND_NONE, brls::SOUND_NONE)));

        this->registerAction("cancel", brls::ControllerButton::BUTTON_B, [this](brls::View* view)-> bool {
            if(this->isFullscreen()){
                this->setFullScreen(false);
            } else {
                this->dismiss();
            }
            return true;
        }, true);

        this->registerAction("全屏", brls::ControllerButton::BUTTON_A, [this](brls::View* view) {
            if(this->isFullscreen()){
                //全屏状态下切换播放状态
                this->togglePlay();
                this->showOSD(true);
            }else{
                //非全屏状态点击视频组件进入全屏
                this->setFullScreen(true);
            }
            return true;
        });
    }
}

VideoView::~VideoView(){
    Logger::debug("trying delete VideoView...");
    mpvCore->getEvent()->unsubscribe(eventSubscribeID);
#ifdef __SWITCH__
    appletSetMediaPlaybackState(false);
#endif
    brls::Logger::error("Delete VideoView done");
}

void VideoView::draw(NVGcontext *vg, float x, float y, float width, float height, Style style, FrameContext *ctx) {
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
    if(unix_time() < this->osdLastShowTime){
        osdBottomBox->frame(ctx);
        osdTopBox->frame(ctx);
    }

    osdSpinner->frame(ctx);

}

void VideoView::invalidate() {
    View::invalidate();

    // Rect rect = getFrame();
    // Logger::error("VideoView::invalidate: {}", rect.describe());
    // if(int(rect.getWidth()) == 0 || int(rect.getHeight()) == 0)
    //     return;
    // //todo: 检查是否和之前的长度宽度一致
    // //    change texture size
    // int drawWidth = (int) (Application::windowScale * rect.getWidth());
    // int drawHeight = (int) (Application::windowScale * rect.getHeight());

    // brls::Logger::debug("Video view size: {} / {}", drawWidth, drawHeight);
    // this->mpvCore->setFrameSize(drawWidth, drawHeight);

}

void VideoView::onLayout(){
    brls::View::onLayout();

    float width           = getWidth();
    float height          = getHeight();
    static float oldWidth = width;
    static float oldHeight = height;
    if (((int)oldWidth != (int)width && width != 0) || ((int)oldHeight != (int)height && height != 0))
    {
        brls::Logger::debug("Video view size: {} / {} scale: {}", width, height, Application::windowScale);
        this->mpvCore->setFrameSize(Application::windowScale * width, Application::windowScale * height);
    }
    oldWidth = width;
    oldHeight = height;
}

void VideoView::setUrl(std::string url){
    // const char *cmd[] = {"loadfile", url.c_str(), NULL};
    // mpvCore->command_async(cmd);

    const char *cmd[] = {"loadfile", url.c_str(), NULL};
    mpvCore->command(cmd);
}

void VideoView::resume(){
    //todo: 此处设置为 loading
    this->videoState = VideoState::PLAYING;
    mpvCore->command_str("set pause no");
    brls::Logger::error("VideoView::resume");
}

void VideoView::pause(){
    this->videoState = VideoState::PAUSED;
    mpvCore->command_str("set pause yes");
    brls::Logger::error("VideoView::pause");
}

void VideoView::stop(){
    brls::Logger::error("VideoView::stop 1");
    // const char *cmd[] = {"stop",  NULL};
    // mpvCore->command_async(cmd);
    mpvCore->command_str("stop");
    brls::Logger::error("VideoView::stop 2");
}