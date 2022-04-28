#pragma once

#include <borealis.hpp>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <glad/glad.h>
#include <bilibili.h>
#include "video_player.hpp"
#include "timer.hpp"
#include "video_osd.hpp"

class VideoFrame : public brls::AbsoluteLayout
{
    public:
        VideoFrame(){
            // Create views
            this->videoPlayer = new VideoPlayer();
            this->osd = new VideoOSD(this->videoPlayer);

            this->setBoundaries(0,0,brls::Application::contentWidth,brls::Application::contentHeight);
            this->addView(this->videoPlayer);
            this->addView(this->osd);

            this->registerAction("debug",brls::Key::X,[this]{
                this->videoPlayer->debug();
                return true;
            },true);
            this->registerAction("toggle",brls::Key::B,[this]{
                this->osd->toggle();
                return true;
            },true);
            this->registerAction("show",brls::Key::A,[this]{
                this->osd->showOSD();
                return true;
            },true);
            this->registerAction("back",brls::Key::Y,[this]{
                // brls::Application::blockInputs();
                brls::Logger::debug("push Y ");
                // if (videoFrame->osd->isWait()){
                //     brls::Application::notify("Can't go back when buffering (this is a bug for switch)");
                //     return true;
                // }
                this->videoPlayer->stop();
                brls::Application::popView();
                // layer->changeLayer(0,true);
                // brls::Application::unblockInputs();
                return true;
            });
        }
        brls::View* getDefaultFocus() override{
            if(this->osd->isShown()){
                return this->osd->getDefaultFocus();
            }
            return this;
        }
        brls::View* getNextFocus(brls::FocusDirection direction, brls::View* currentView) override{
            return this->navigationMap.getNextFocus(direction, currentView);
        }
        void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash){
            int x = this->getX();
            int y = this->getY();

            this->osd->setBoundaries(x,y,this->getWidth(),this->getHeight());

        }

        void setVideoPage(bilibili::VideoPage page){
            this->currentPage = page;
        }
        bilibili::VideoPage getVideoPage(){
            return this->currentPage;
        }
        void startPlay(bilibili::Video video){
            this->osd->showOSD();
            this->osd->setTitle(video.title);
            this->osd->setAvatar(video.owner.face);
            this->currentVideo = video;
            brls::Logger::debug("try to get video url");
            // brls::Application::notify("try to get video url");
            bilibili::BilibiliClient::get_playurl(video.cid, 80, [this](bilibili::VideoPage page){
                this->setVideoPage(page);
                std::vector<std::string> fileList;
                for (bilibili::VideoSlice slice : this->currentPage.durl){
                    fileList.push_back(slice.url);
                    brls::Logger::debug("video url: {}",slice.url);
                }
                this->videoPlayer->setPlayList(fileList);
                brls::Logger::debug("set playlist done");
                this->videoPlayer->startPlayList();
            });
        }

        brls::NavigationMap navigationMap;
        VideoOSD* osd;
        VideoPlayer* videoPlayer;
        bilibili::VideoPage currentPage;
        bilibili::Video currentVideo;

};