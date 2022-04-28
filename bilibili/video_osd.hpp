#pragma once

#include <borealis.hpp>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <glad/glad.h>
#include "video_player.hpp"
#include "timer.hpp"
#include "views/net_image.hpp"

#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 100
#define PADDING 75

class VideoOSD : public brls::AbsoluteLayout
{
  public:
    VideoOSD(VideoPlayer *videoPlayer):videoPlayer(videoPlayer){
        this->setProgressPercentage(50);
        // spinner
        this->progressSpinner = new brls::ProgressSpinner();

        // resolution
        this->qualityIndex = 0;
        this->accept_description.push_back("1080p");
        this->accept_description.push_back("720p");
        this->accept_description.push_back("480p");
        this->accept_description.push_back("360p");

        //avatar
        this->avatarImage = new NetImage(BOREALIS_ASSET("icon/bilibili_128x128.jpg"));
        this->addView(avatarImage);

        //hint
        this->hint = new brls::Hint();
        this->addView(hint);
        

        this->registerAction("-5s", brls::Key::L, [this] {
            brls::Application::notify("-5s");
            brls::Logger::debug("p:{}, d:{}",this->videoPlayer->getPosition(),this->videoPlayer->getDuration());
            this->videoPlayer->seek(this->videoPlayer->getPosition()-5);
            return true;
        },true);

        this->registerAction("+5s", brls::Key::R, [this] {
            brls::Application::notify("+5s");
            this->videoPlayer->seek(this->videoPlayer->getPosition()+5);
            brls::Logger::debug("p:{}, d:{}",this->videoPlayer->getPosition(),this->videoPlayer->getDuration());
            return true;
        },true);
        this->registerAction("play/pause", brls::Key::B, [this] {
            this->hideOSD();
            return true;
        },true);
        this->registerAction("play/pause", brls::Key::A, [this] {
            if(this->videoPlayer->getPause()){
                this->videoPlayer->resume();
            }
            this->videoPlayer->pause();
            return true;
        });
        this->registerAction("resolution", brls::Key::X, [this] {
            brls::ValueSelectedEvent::Callback valueCallback = [this](int result) {
                if (result == -1)
                    return;
                brls::Logger::debug("quality:{}",result);
                this->qualityIndex = result;
            };
            brls::Dropdown::open("quality", this->accept_description, valueCallback, this->qualityIndex);
            return true;
        });

        this->showOSD();
    }

    void setTitle(std::string title){
        this->title = title;
    }

    void setAvatar(std::string avatar){
        this->avatar = avatar;
        this->avatarImage->setImage(avatar);
    }

    void toggle(){
        if(this->OSDisShown){
            this->hideOSD();
        } else {
            this->showOSD();
        }
    }

    void hideOSD(){
        this->OSDisShown = false;
        this->avatarImage->hide([](){}, false, brls::ViewAnimation::FADE);
        this->hint->hide([](){}, false, brls::ViewAnimation::FADE);
        brls::Application::giveFocus(this->parent);
    }
    void showOSD(){
        this->OSDisShown = true;
        this->avatarImage->show([](){}, true, brls::ViewAnimation::FADE);
        this->hint->show([](){}, true, brls::ViewAnimation::FADE);
        brls::Application::giveFocus(this);
    }
    bool isShown(){
        return this->OSDisShown;
    }
    brls::View* getDefaultFocus() override {
        return this;
    //   if(this->OSDisShown){
    //     return this->playButton->getDefaultFocus();
    //   }
    //   return nullptr;
    }
    brls::View* getNextFocus(brls::FocusDirection direction, brls::View* currentView) override {
      return this->navigationMap.getNextFocus(direction, currentView);
    }

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override {
        mpv_event *event = this->videoPlayer->getEvent();
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (event != nullptr){
            switch (event->event_id){
                case MPV_EVENT_FILE_LOADED:
                    brls::Logger::debug("loaded");
                    // brls::Application::notify("file loaded");
                    this->wait = false;
                break;
                case MPV_EVENT_START_FILE:
                    brls::Logger::debug("start");
                    // brls::Application::notify("start buffering");
                    this->wait = true;
                break;
                case MPV_EVENT_END_FILE:
                    brls::Logger::debug("end");
                    this->wait = false;
                break;
                case MPV_EVENT_PROPERTY_CHANGE:
                    if (strcmp(prop->name, "playback-time") == 0){
                        // brls::Logger::debug("playback-time:{}",this->videoPlayer->getPosition());
                    } else if (strcmp(prop->name, "duration") == 0){
                        // brls::Logger::debug("duration:{}",this->videoPlayer->getDuration());
                    } else if (strcmp(prop->name, "core-idle") == 0){
                        // int flag = *(int *)prop->data;
                        if(this->videoPlayer->getCore() == 1 && this->videoPlayer->getPause() == 0 ){ // waiting for internet or something
                            this->wait = true;
                            //  this->showOSD();
                        }else{
                            this->wait = false;
                            if(this->videoPlayer->getState() == VideoState::WAITTING_STOP){
                                this->videoPlayer->stop();
                            }
                            // this->hideOSD();
                        }
                    }
                break;
                default:
                break;
            }
        }
        

        unsigned progressBarWidth = width * 0.4;
        unsigned progressBarHeight= 25;
        unsigned progressBarX     = x + 50;
        unsigned progressBarY     = height - 50;

        if(this->OSDisShown){

            //background
            nvgBeginPath(vg);
            nvgRect(vg,x,y,this->getWidth(),style->AppletFrame.headerHeightRegular);
            nvgFillColor(vg, a(ctx->theme->backgroundColorRGB));
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgRect(vg,x,y+this->getHeight()-style->AppletFrame.footerHeight,this->getWidth(),style->AppletFrame.footerHeight);
            nvgFillColor(vg, a(ctx->theme->backgroundColorRGB));
            nvgFill(vg);

            //title text
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgFontSize(vg, style->AppletFrame.titleSize);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgBeginPath(vg);
            nvgText(vg, x + 125  , y + 50 , this->title.c_str(), nullptr);


            // progress bar
            this->progressPercentage = this->videoPlayer->getPosition() * 100.0 / this->videoPlayer->getDuration();
            nvgBeginPath(vg);
            nvgMoveTo(vg, progressBarX, progressBarY + progressBarHeight / 2);
            nvgLineTo(vg, progressBarX + progressBarWidth, progressBarY + progressBarHeight / 2);
            nvgStrokeColor(vg, a(ctx->theme->listItemSeparatorColor));
            nvgStrokeWidth(vg, progressBarHeight / 3);
            nvgLineCap(vg, NVG_ROUND);
            nvgStroke(vg);

            if (this->progressPercentage > 0.0f)
            {
                nvgBeginPath(vg);
                nvgMoveTo(vg, progressBarX, progressBarY + progressBarHeight / 2);
                nvgLineTo(vg, progressBarX + ((float)progressBarWidth * this->progressPercentage) / 100, progressBarY + progressBarHeight / 2);
                nvgStrokeColor(vg, a(ctx->theme->listItemValueColor));
                nvgStrokeWidth(vg, progressBarHeight / 3);
                nvgLineCap(vg, NVG_ROUND);
                nvgStroke(vg);
            }
        }

        if(this->wait)
            this->progressSpinner->frame(ctx);

        brls::AbsoluteLayout::draw(vg,x,y,width,height,style,ctx);
    }

    void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash) override {
        int x = this->getX();
        // int y = this->getY();

        this->avatarImage->setBoundaries(
            x + 50,
            y + 25,
            50,
            50
        );
        this->avatarImage->setCornerRadius(100);

        this->progressSpinner->setWidth(100);
        this->progressSpinner->setHeight(100);
        this->progressSpinner->setBoundaries(
            this->getWidth()/2 - 50,
            this->getHeight()/2 - 50,
            100,
            100);

        unsigned hintWidth = this->width - style->AppletFrame.separatorSpacing * 2 - style->AppletFrame.footerTextSpacing * 2;
        this->hint->setBoundaries(
            this->x + this->width - hintWidth - style->AppletFrame.separatorSpacing - style->AppletFrame.footerTextSpacing,
            this->y + this->height - style->AppletFrame.footerHeight,
            hintWidth,
            style->AppletFrame.footerHeight);
        this->hint->invalidate();

    }

    void setDuration(float time){
        this->wholeTime = time;
    }

    bool isWait(){
        return this->wait;
    }
  private:
    void setProgressPercentage(float p){
        this->progressPercentage = p;
    }
    brls::NavigationMap navigationMap;
    brls::ProgressSpinner* progressSpinner;
    brls::Hint* hint;
    brls::Button* playButton;
    brls::Button* resolutionButton;
    VideoPlayer* videoPlayer;
    NetImage* avatarImage;

    
    bool OSDisShown = true;
    bool wait = true;;
    float progressPercentage = 0.0f;
    float wholeTime = 0;
    float currentTime = 0;
    int qualityIndex;
    std::vector<std::string> accept_description;
    std::vector<int> accept_quality;

    std::string avatar;
    std::string title;
    

};