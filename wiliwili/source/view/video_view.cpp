//
// Created by fang on 2022/4/23.
//

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "view/video_view.hpp"
#include "view/mpv_core.hpp"
#include "view/video_progress_slider.hpp"
#include "view/svg_image.hpp"
#include "utils/number_helper.hpp"
#include <fmt/core.h>
#include "activity/player_activity.hpp"

using namespace brls;


VideoView::VideoView() {
    mpvCore = &MPVCore::instance();
    this->inflateFromXMLRes("xml/views/video_view.xml");
    this->setHideHighlightBackground(true);
    this->setHideClickAnimation(true);

    this->registerBoolXMLAttribute("allowFullscreen", [this](bool value){
        this->allowFullscreen = value;
        if(!value){
            this->btnFullscreen->setVisibility(brls::Visibility::GONE);
            this->registerAction("cancel", brls::ControllerButton::BUTTON_B, [this](brls::View* view)-> bool {
                this->dismiss();
                return true;
            }, true);
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

    this->registerMpvEvent();

    osdSlider->getProgressEvent()->subscribe([](float progress){
        brls::Logger::debug("progress: {}", progress);
        //todo: less call
        //todo: wakeup osd
//        mpvCore->command_str(fmt::format("seek {} absolute-percent", progress * 100).c_str());
    });

    this->addGestureRecognizer(new brls::TapGestureRecognizer(this, [this](){
        if(isOSDShown()){
            this->hideOSD();
        } else {
            this->showOSD(true);
        }
    }));

    this->btnToggle->addGestureRecognizer(new brls::TapGestureRecognizer(this->btnToggle, [this](){
        if(mpvCore->isPaused()){
            mpvCore->resume();
        } else {
            mpvCore->pause();
        }
    }, brls::TapGestureConfig(false, brls::SOUND_NONE, brls::SOUND_NONE, brls::SOUND_NONE)));

    /// 默认允许设置全屏按钮
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

VideoView::~VideoView(){
    Logger::debug("trying delete VideoView...");
    this->unRegisterMpvEvent();
    brls::Logger::debug("Delete VideoView done");
}

void VideoView::draw(NVGcontext *vg, float x, float y, float width, float height, Style style, FrameContext *ctx) {
    if(!mpvCore->isValid())
        return;

    mpvCore->openglDraw(this->getFrame(), this->getAlpha());

    // draw osd
    if(wiliwili::unix_time() < this->osdLastShowTime){
        osdBottomBox->frame(ctx);
        osdTopBox->frame(ctx);
    }

    osdSpinner->frame(ctx);

}

void VideoView::invalidate() {
    View::invalidate();
}

void VideoView::onLayout(){
    brls::View::onLayout();

    brls::Rect rect = getFrame();
    static brls::Rect oldRect = rect;

    if(!(rect == oldRect)){
        brls::Logger::debug("Video view size: {} / {} scale: {}", rect.getWidth(), rect.getHeight(), Application::windowScale);
        this->mpvCore->setFrameSize(getFrame());
    }
    oldRect = rect;
}

void VideoView::setUrl(std::string url){
    // const char *cmd[] = {"loadfile", url.c_str(), NULL};
    // mpvCore->command_async(cmd);

    const char *cmd[] = {"loadfile", url.c_str(), NULL};
    mpvCore->command(cmd);
}

void VideoView::resume(){
    mpvCore->command_str("set pause no");
}

void VideoView::pause(){
    mpvCore->command_str("set pause yes");
}

void VideoView::stop(){
//     const char *cmd[] = {"stop",  NULL};
//     mpvCore->command_async(cmd);
    mpvCore->command_str("stop");
}

void VideoView::togglePlay(){
    if (this->mpvCore->isPaused()){
        this->resume();
    } else {
        this->pause();
    }
}

void VideoView::start(std::string url){
    brls::Logger::debug("start mpv: {}", url);
    this->setUrl(url);
    brls::Logger::debug("set url to mpv done");
}

/// OSD
void VideoView::showOSD(bool temp){
    if(temp)
        this->osdLastShowTime = wiliwili::unix_time() + VideoView::OSD_SHOW_TIME;
    else
        this->osdLastShowTime = 0xffffffff;
}

void VideoView::hideOSD(){
    this->osdLastShowTime = 0;
}

bool VideoView::isOSDShown(){
    return wiliwili::unix_time() < this->osdLastShowTime;
}

// Loading
void VideoView::showLoading(){
    osdSpinner->setVisibility(brls::Visibility::VISIBLE);
}

void VideoView::hideLoading(){
    osdSpinner->setVisibility(brls::Visibility::GONE);
}

void VideoView::setTitle(std::string title){
    brls::Threading::sync([this, title](){
        this->videoTitleLabel->setText(title);
    });
}

std::string VideoView::getTitle(){
    return this->videoTitleLabel->getFullText();
}

void VideoView::setDuration(std::string value){
    this->rightStatusLabel->setText(value);
}

void VideoView::setPlaybackTime(std::string value){
    this->leftStatusLabel->setText(value);
}

void VideoView::refreshFullscreenIcon() {
    if(isFullscreen()){
        btnFullscreenIcon->setImageFromSVGRes("svg/bpx-svg-sprite-fullscreen-off.svg");
    } else {
        btnFullscreenIcon->setImageFromSVGRes("svg/bpx-svg-sprite-fullscreen.svg");
    }
}

void VideoView::refreshToggleIcon() {
    if(mpvCore->isPaused()){
        btnToggleIcon->setImageFromSVGRes("svg/bpx-svg-sprite-play.svg");
    } else {
        btnToggleIcon->setImageFromSVGRes("svg/bpx-svg-sprite-pause.svg");
    }
}

void VideoView::setProgress(float value){
    this->osdSlider->setProgress(value);
}

float VideoView::getProgress(){
    return this->osdSlider->getProgress();
}

View* VideoView::create(){
    return new VideoView();
}

bool VideoView::isFullscreen(){
    auto rect = this->getFrame();
    return rect.getHeight() == brls::Application::contentHeight && \
            rect.getWidth() == brls::Application::contentWidth;
}

void VideoView::setFullScreen(bool fs){
    if(!allowFullscreen){
        brls::Logger::error("Not being allowed to set fullscreen");
        return;
    }

    if(fs == isFullscreen()){
        brls::Logger::error("Already set fullscreen state to: {}", fs);
        return;
    }

    brls::Logger::info("VideoView set fullscreen state: {}", fs);
    if (fs){
        this->unRegisterMpvEvent();
        auto container = new brls::Box();
        auto video = new VideoView();
        container->setDimensions(1280, 720);
        video->setDimensions(1280, 720);
        video->setTitle(this->getTitle());
        video->setDuration(this->rightStatusLabel->getFullText());
        video->setPlaybackTime(this->leftStatusLabel->getFullText());
        video->setProgress(this->getProgress());
        video->showOSD(this->osdLastShowTime != 0xffffffff);
        video->refreshFullscreenIcon();
        video->setHideHighlight(true);
        video->refreshToggleIcon();
        if(osdSpinner->getVisibility() == brls::Visibility::GONE){
            video->hideLoading();
        }
        container->addView(video);
        brls::Application::pushActivity(new brls::Activity(container), brls::TransitionAnimation::NONE);
    } else {
        brls::sync([this](){
            //todo: a better way to get videoView pointer
            PlayerActivity* last = dynamic_cast<PlayerActivity*>(Application::getActivitiesStack()[Application::getActivitiesStack().size() - 2]);
            if(last){
                VideoView* video = dynamic_cast<VideoView *>(last->getView("video/detail/video"));
                if(video){
                    video->setProgress(this->getProgress());
                    video->showOSD(this->osdLastShowTime != 0xffffffff);
                    video->setDuration(this->rightStatusLabel->getFullText());
                    video->setPlaybackTime(this->leftStatusLabel->getFullText());
                    video->registerMpvEvent();
                    video->refreshToggleIcon();
                    if(osdSpinner->getVisibility() == brls::Visibility::GONE){
                        video->hideLoading();
                    } else {
                        video->showLoading();
                    }
                    // 立刻准确地显示视频尺寸
                    video->onLayout();
                }
            }
            // Pop fullscreen videoView
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
        });
    }
}

View* VideoView::getDefaultFocus() {
    return this;
}

View* VideoView::getNextFocus(brls::FocusDirection direction, View* currentView) {
    if(this->isFullscreen())
        return this;
    return Box::getNextFocus(direction, currentView);
}

void VideoView::registerMpvEvent(){
    eventSubscribeID = mpvCore->getEvent()->subscribe([this](MpvEventEnum event){
        // brls::Logger::info("mpv event => : {}", event);
        switch (event){
            case MpvEventEnum::MPV_RESUME:
                this->showOSD(true);
                this->hideLoading();
                this->btnToggleIcon->setImageFromSVGRes("svg/bpx-svg-sprite-pause.svg");
                break;
            case MpvEventEnum::MPV_PAUSE:
                this->showOSD(false);
                this->btnToggleIcon->setImageFromSVGRes("svg/bpx-svg-sprite-play.svg");
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
                osdSlider->setProgress(mpvCore->playback_time / mpvCore->duration);
                break;
            default:
                break;
        }
    });
}

void VideoView::unRegisterMpvEvent(){
    mpvCore->getEvent()->unsubscribe(eventSubscribeID);
}