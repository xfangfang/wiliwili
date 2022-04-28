#pragma once

#include <borealis.hpp>
#include <bilibili.h>
#include "grid_list.hpp"
#include "pages/video_frame.hpp"
#include "pages/login_frame.hpp"
#include "pages/person_frame.hpp"
#include "utils/utils.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

class Context{
    public:
        static void playVideo(bilibili::Video video){
            VideoFrame* videoFrame = new VideoFrame();
            brls::Application::pushView(videoFrame);
            videoFrame->startPlay(video);
        }
};

class Application{
    public:

        static bool init(){
            brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
            brls::Logger::debug("start");
            i18n::loadTranslations();
            if (!brls::Application::init("main/name"_i18n))
            {
                brls::Logger::error("Unable to init Borealis application");
                return EXIT_FAILURE;
            }
            // get bilibili cookie from config file
            Cookie cookie = Utils::programConfig.getCookie();
            // set bilibili cookie and cookie update callback
            bilibili::BilibiliClient::init(cookie,[](Cookie newCookie){
                ProgramConfig config;
                config.setCookie(newCookie);
                Utils::saveProgramConf(config);
            });
            return EXIT_SUCCESS;
        }

        static void mainloop(){
            // Run the app
            while (brls::Application::mainLoop()){
                RunOnUIThread::run();
            }
        }

        static void clean(){
            bilibili::BilibiliClient::clean();
        }

        static brls::View* _buildPersonPage(){
            brls::LayerView* personlayer = new brls::LayerView();
            PersonFrame* personFrame = new PersonFrame();
            personFrame->refresh();
            personlayer->addLayer(personFrame);
            return personlayer;
        }

        static brls::View* _buildVideoListPage(){
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
            gridList->getClickEvent()->subscribe([](bilibili::Video video, GridListItem* view){
                Context::playVideo(video);
            });

            bilibili::BilibiliClient::get_recommend(160,16,[gridList](bilibili::VideoList list){
                for (bilibili::Video video : list.data.archives){
                    brls::Logger::debug("{}:{}",video.owner.name, video.title);
                    brls::Logger::debug("    pic:{}",video.pic);
                    brls::Logger::debug("    face:{}",video.owner.face);
                }
                gridList->addListData(list.data.archives);
            });

            return gridList;
        }

        static void buildPages(){
            brls::TabFrame* rootFrame = new brls::TabFrame();

            rootFrame->setTitle("main/name"_i18n);
            rootFrame->setIcon(BOREALIS_ASSET("icon/bilibili_128x128.jpg"));
            rootFrame->setSiderbarWidth(160);
            rootFrame->setSiderbarMargins(40,30,40,30);
            rootFrame->setHeaderStyle(brls::HeaderStyle::DOWN);

            rootFrame->addTab("个人", _buildPersonPage());
            rootFrame->addTab("推荐", _buildVideoListPage());
            rootFrame->addTab("main/tabs/third"_i18n, new brls::Rectangle(nvgRGB(255, 0, 0)));
            rootFrame->addTab("main/tabs/fourth"_i18n, new brls::Rectangle(nvgRGB(0, 255, 0)));
            brls::Application::pushView(rootFrame);
        }

};