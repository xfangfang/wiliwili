#pragma once

#include "application.hpp"
#include <borealis.hpp>
#include <bilibili.h>
#include <thread>
#include <chrono>
#include "views/net_image.hpp"
#include "grid_list.hpp"
#include "grid_frame.hpp"
#include "pages/login_frame.hpp"
#include "utils/utils.hpp"
#include "pages/video_frame.hpp"

class PersonFrame : public brls::AbsoluteLayout
{
    using VideoList = bilibili::space_user_videos::VideoList;
    using Video = bilibili::space_user_videos::Video;
    using CollectionList = bilibili::space_user_collections::CollectionList;
    using Collection = bilibili::space_user_collections::Collection;
    using CollectionVideo = bilibili::space_user_collections::CollectionVideo;
    using CollectionDetail = bilibili::space_user_collections::CollectionDetail;
    public:
        PersonFrame(){
            
            this->style = new brls::Style();
            this->style->List.marginLeftRight = 0;
            this->style->List.marginTopBottom = 0;
            this->style->List.spacing = 0;
            this->style->List.Item.height = 100;

            // Create views
            this->backgroudTopImage = new brls::Image(BOREALIS_ASSET("pictures/person_background_top.png"));
            this->addView(this->backgroudTopImage);

            this->avatarImage = new NetImage(BOREALIS_ASSET("icon/akari.jpg"));
            this->addView(this->avatarImage);
        
            this->videoList = new GridList<Video>(8,125,100,10,[this](auto video){
                VideoListItem *l1 = new VideoListItem(video.author, video.title, video.pic+"@216w_158h_1e_1c.jpg", this->userProfile.face+"@45w_45h_1e_1c.jpg");
                l1->setWidth(125);
                l1->setHeight(100);
                return l1;
            });
            this->videoList->getClickEvent()->subscribe([this](Video data, GridListItem* view){
                this->playVideo(data);
            });
            this->addView(this->videoList);

            this->collectionList = new GridList<Collection>(8,125,110,10,[this](auto collection){
                VideoListItem *l1 = new VideoListItem(collection.upper.name, collection.title, collection.cover+"@216w_158h_1e_1c.jpg", this->userProfile.face+"@45w_45h_1e_1c.jpg");
                l1->setWidth(125);
                l1->setHeight(100);
                return l1;
            });
            this->collectionList->getClickEvent()->subscribe([this](Collection data, GridListItem* view){
                this->popupUserCollection(data);
            });
            this->addView(this->collectionList);

            this->navigationMap.add(
                this->videoList,
                brls::FocusDirection::DOWN,
                this->collectionList);
            this->navigationMap.add(
                this->collectionList,
                brls::FocusDirection::UP,
                this->videoList);
        }
        brls::View* getDefaultFocus() override {
            return this->videoList->getDefaultFocus();
        }
        brls::View* getNextFocus(brls::FocusDirection direction, brls::View* currentView) override{
            return this->navigationMap.getNextFocus(direction, currentView);
        }
        void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash){
            int x = this->getX();
            int y = this->getY();
            int w = this->getWidth();
            int h = this->getHeight();

            // this->backgroudImage->setBoundaries(x + w * 0.75 - 240, y + h/2 + 100 ,480,155);

            this->backgroudTopImage->setBoundaries(x, y ,w, h / 3 - 28);
            this->backgroudTopImage->setScaleType(brls::ImageScaleType::SCALE);

            this->avatarImage->setBoundaries(x + 16, y + 16, 100, 100);
            this->avatarImage->setScaleType(brls::ImageScaleType::SCALE);
            this->avatarImage->setCornerRadius(100);

            this->videoList->setBoundaries(x + 16, y + h / 3 + 32, w - 32, h * 2 / 3 - 32);

            this->collectionList->setBoundaries(x + 16, y + h * 2 / 3 + 32, w - 32, h * 2 / 3 - 32);
        }

        void refresh(){
            bilibili::BilibiliClient::get_my_info([this](bilibili::UserDetail user){
                if(user.mid == -1){
                    // not login
                    // open login frame
                    brls::Logger::debug("User need login");
                    RunOnUIThread([this](){
                        brls::Logger::debug("Open login frame");
                        ((brls::LayerView *)this->getParent())->pushLayer(new LoginFrame([this](){
                            RunOnUIThread([this](){
                                ((brls::LayerView *)this->getParent())->popLayer();
                            });
                            this->refresh();
                        }));
                    });
                } else {
                    this->userProfile = user;
                    // this->userProfile.mid = 442018683;
                    this->avatarImage->setImage(this->userProfile.face+"@180w_180h_1e_1c.jpg");
                    bilibili::BilibiliClient::get_user_videos(this->userProfile.mid, 1, 7, [this](VideoList list){
                        this->videoListData = list;
                        this->videoList->addListData(this->videoListData.getList());;
                        if(this->videoListData.has_more){
                            this->videoList->addMoreAction([this]{
                                brls::Application::notify("more videos");
                                this->popupUserVideo(this->userProfile.mid, this->videoListData.page.count);
                            });
                        }
                        for(auto video: list.getList()){
                            brls::Logger::debug("video:{}",video.title);
                        }
                    });
                    bilibili::BilibiliClient::get_user_collections(this->userProfile.mid , 1, 7, [this](CollectionList list){
                        this->collectionListData = list;
                        this->collectionList->addListData(this->collectionListData.getList());
                        if(this->collectionListData.has_more){
                            this->collectionList->addMoreAction([this](){
                                brls::Application::notify("more collections");
                                this->gridFrameCollection = new GridFrame<Collection>(7,125,110,20,[this](auto collection){
                                    VideoListItem *l1 = new VideoListItem(collection.upper.name, collection.title, collection.cover+"@216w_158h_1e_1c.jpg", this->userProfile.face+"@45w_45h_1e_1c.jpg");
                                    l1->setWidth(125);
                                    l1->setHeight(100);
                                    return l1;
                                }, true); 
                                this->gridFrameCollection->setNumPerPage(28);
                                this->gridFrameCollection->setRequestCallback(std::bind(&PersonFrame::getUserCollection, this, std::placeholders::_1, std::placeholders::_2));
                                this->gridFrameCollection->getNextData();
                                this->gridFrameCollection->getClickEvent()->subscribe([this](Collection data, GridListItem* view){
                                    this->popupUserCollection(data);
                                });
                                brls::AppletFrame* ap = new brls::AppletFrame(true, true);
                                ap->setContentView(this->gridFrameCollection);
                                brls::PopupFrame::open("Collections", ap, "", "");
                            });
                        }
                        for (auto collection: list.getList()){
                            brls::Logger::debug("collection:{}",collection.title);
                        }
                    });
                }
            });
        }

        void getUserCollection(size_t pn, size_t ps){
            bilibili::BilibiliClient::get_user_collections(this->userProfile.mid , pn, ps, [this](CollectionList list){
                brls::Logger::error("get user collections");
                if(this->gridFrameCollection)
                    this->gridFrameCollection->loadData(list.getList());
            });
        }

        void playVideo(Video video){
            bilibili::Video v;
            v.title = video.title;
            brls::Logger::debug("play video: {}", v.title);
            VideoFrame* videoFrame = new VideoFrame();
            brls::Application::pushView(videoFrame);
            videoFrame->startPlay(v);
        }

        void popupUserVideo(size_t mid, size_t count){
            this->gridFramePersonVideo = new GridFrame<Video>(5,175,140,25,[this](auto video){
                VideoListItem *l1 = new VideoListItem(video.author, video.title, video.pic+"@216w_158h_1e_1c.jpg", "");
                l1->setWidth(175);
                l1->setHeight(140);
                return l1;
            }, true);
            this->gridFramePersonVideo->getClickEvent()->subscribe([this](Video data, GridListItem* view){
                this->playVideo(data);
            });
            this->gridFramePersonVideo->setNumPerPage(20);
            this->gridFramePersonVideo->setRequestCallback([this, mid](int pn, int ps){
                this->getUserPersonVideo(mid, pn, ps);
            });
            this->gridFramePersonVideo->getNextData();
            brls::AppletFrame* ap = new brls::AppletFrame(true, true);
            ap->setContentView(this->gridFramePersonVideo);
            brls::PopupFrame::open("Videos", ap, this->userProfile.name, std::to_string(count)+"个内容");
        }

        void getUserPersonVideo(size_t mid, size_t pn, size_t ps){
            bilibili::BilibiliClient::get_user_videos(mid, pn, ps, [this](VideoList list){
                for (auto d: list.getList()){
                    brls::Logger::debug("video: {}", d.title);
                }
                if(this->gridFramePersonVideo)
                    this->gridFramePersonVideo->loadData(list.getList());
            });
        }

        void popupUserCollection(Collection data){
            brls::Logger::error("open collection:{}", data.title);
            this->gridFrameCollectionVideo = new GridFrame<CollectionVideo>(5,175,140,25,[this](auto video){
                VideoListItem *l1 = new VideoListItem(video.upper.name, video.title, video.cover+"@216w_158h_1e_1c.jpg", video.upper.face+"@45w_45h_1e_1c.jpg");
                l1->setWidth(175);
                l1->setHeight(140);
                return l1;
            }, true); 
            this->gridFrameCollectionVideo->setNumPerPage(20);
            this->gridFrameCollectionVideo->setRequestCallback([this, data](int pn, int ps){
                this->getUserCollectionVideo(data.id, pn, ps);
            });
            this->gridFrameCollectionVideo->getNextData();
            brls::AppletFrame* ap = new brls::AppletFrame(true, true);
            ap->setContentView(this->gridFrameCollectionVideo);
            brls::PopupFrame::open(data.title, ap, this->userProfile.name, std::to_string(data.media_count)+"个内容");
        }

        void getUserCollectionVideo(size_t id, size_t pn, size_t ps){
            bilibili::BilibiliClient::get_collection_videos(id, pn, ps, [this](CollectionDetail list){
                for (auto d : list.getList())
                {
                    brls::Logger::debug("video: {}", d.title);
                }
                
                if(this->gridFrameCollectionVideo)
                    this->gridFrameCollectionVideo->loadData(list.getList());
            });
        }

        void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override {

            // //background
            // nvgBeginPath(vg);
            // nvgRect(vg,x,y,width,height);
            // nvgFillColor(vg, nvgRGB(255,255,255));
            // nvgFill(vg);


            // lines
            // NVGcolor lineColor = nvgRGB(221,221,221);
            // nvgBeginPath(vg);
            // nvgRect(vg, x + width / 2, y + height / 3, 1, height * 2 / 3 - 56);
            // nvgFillColor(vg, lineColor);
            // nvgFill(vg);


            brls::AbsoluteLayout::draw(vg,x,y,width,height,style,ctx);

            //title text
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgFontSize(vg, 24);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgBeginPath(vg);
            nvgText(vg, x + 132 ,y + 41, this->userProfile.name.c_str(), nullptr);

            //title sign
            nvgFillColor(vg, nvgRGB(255, 255, 255));
            nvgFontSize(vg, 18);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgBeginPath(vg);
            nvgText(vg, x + 132 ,y + 91, this->userProfile.sign.c_str(), nullptr);

            //hint my videos
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgFontSize(vg, 28);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgBeginPath(vg);
            nvgText(vg, x + 16, y + height / 3 , "Videos", nullptr);

            //hint my collections
            nvgBeginPath(vg);
            nvgText(vg, x + 16, y + height * 2 / 3 , "Collections", nullptr);

            //hint my videos no items
            nvgFillColor(vg, nvgRGB(113,113,113));
            nvgFontSize(vg, 18);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            if(this->videoListData.getList().size() == 0){
                nvgBeginPath(vg);
                nvgText(vg, x + width / 2, y + height / 2 , "There is no video", nullptr);
            }
            
            //hint my collections no items
            if(this->collectionListData.getList().size() == 0){
                nvgBeginPath(vg);
                nvgText(vg, x + width / 2, y + height * 5 / 6 , "There is no collection", nullptr);
            }

        }

        ~PersonFrame(){
            delete this->style;
        }

    private:
        brls::NavigationMap navigationMap;

        QrImage* qrImage;
        brls::Image* backgroudImage;
        brls::Image* backgroudTopImage;
        brls::Button* videoMore;
        NetImage* avatarImage;
        GridList<Video>* videoList;
        GridList<Collection>* collectionList;
        GridFrame<Collection>* gridFrameCollection;
        GridFrame<CollectionVideo>* gridFrameCollectionVideo;
        GridFrame<Video>* gridFramePersonVideo;
        brls::Style* style;

        bilibili::UserDetail userProfile;
        VideoList videoListData;
        CollectionList collectionListData;
};