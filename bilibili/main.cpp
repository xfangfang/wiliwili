
/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2019-2020  natinusala
    Copyright (C) 2019  p-sam

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <borealis.hpp>
#include <bilibili.h>
#include "flex_frame.hpp"
#include "grid_list.hpp"
#include "video_frame.hpp"


namespace i18n = brls::i18n; // for loadTranslations() and getStr()
using namespace i18n::literals; // for _i18n


int main(int argc, char* argv[])
{
    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
    brls::Logger::debug("start");
    bilibili::BilibiliClient::init();
    i18n::loadTranslations();
    if (!brls::Application::init("main/name"_i18n))
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    brls::LayerView* layer = new brls::LayerView();

    VideoFrame* videoFrame = new VideoFrame();
    videoFrame->registerAction("back",brls::Key::Y,[videoFrame,layer]{
        // brls::Application::blockInputs();
        brls::Logger::debug("push Y ");
        // if (videoFrame->osd->isWait()){
        //     brls::Application::notify("Can't go back when buffering (this is a bug for switch)");
        //     return true;
        // }
        videoFrame->videoPlayer->stop();
        layer->changeLayer(0,true);
        // brls::Application::unblockInputs();
        return true;
    });
    FlexFrame* rootFrame = new FlexFrame();

    GridList<bilibili::Video>* gridList = new GridList<bilibili::Video>(4,250,200,20,[](auto video){
        VideoListItem *l1 = new VideoListItem(video.owner.name, video.title, video.pic+"@557w_316h_1e_1c.jpg", video.owner.face+"@90w_90h_1e_1c.jpg");
        l1->setWidth(250);
        l1->setHeight(200);
        return l1;
    });
    gridList->registerAction("refresh", brls::Key::X, [gridList]{
        bilibili::BilibiliClient::get_recommend(160,16,[gridList](bilibili::VideoList list){
            for (bilibili::Video video : list.data.archives){
                brls::Logger::debug("{}:{}",video.owner.name, video.title);
                brls::Logger::debug("    pic:{}",video.pic);
                brls::Logger::debug("    face:{}",video.owner.face);
            }
            gridList->addListData(list.data.archives);
        });
        return true;
    });
    gridList->getClickEvent()->subscribe([videoFrame,layer](bilibili::Video video, VideoListItem* view){
        // brls::Application::blockInputs();
        videoFrame->startPlay(video);
        // videoFrame->osd->setDefaultImage(view->getPicImageID());
        layer->changeLayer(1,true);
        // brls::Application::unblockInputs();
    });


    bilibili::BilibiliClient::get_recommend(160,16,[gridList](bilibili::VideoList list){
        for (bilibili::Video video : list.data.archives){
            brls::Logger::debug("{}:{}",video.owner.name, video.title);
            brls::Logger::debug("    pic:{}",video.pic);
            brls::Logger::debug("    face:{}",video.owner.face);
        }
        gridList->addListData(list.data.archives);
    });

    rootFrame->addTab("video", gridList);
    rootFrame->addTab("main/tabs/third"_i18n, new brls::Rectangle(nvgRGB(255, 0, 0)));
    rootFrame->addTab("main/tabs/fourth"_i18n, new brls::Rectangle(nvgRGB(0, 255, 0)));

    layer->addLayer(rootFrame);
    layer->addLayer(videoFrame);
    brls::Application::pushView(layer);

    // Run the app
    while (brls::Application::mainLoop()){

    }
    bilibili::BilibiliClient::clean();
    // Exit
    return EXIT_SUCCESS;
}