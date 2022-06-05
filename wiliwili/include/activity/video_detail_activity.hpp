//
// Created by fang on 2022/4/22.
//

#pragma once

#include <borealis.hpp>
#include <utility>
#include "view/video_view.hpp"
#include "view/user_info.hpp"
#include "presender/video_detail.hpp"
#include "bilibili.h"

using namespace brls;

class VideoDetailActivity : public brls::Activity, public VideoDetail
{
public:
    // Declare that the content of this activity is the given XML file
    CONTENT_FROM_XML_RES("activity/video_detail.xml");

    VideoDetailActivity(bilibili::Video video):video_data(std::move(video)){
        Logger::error("create VideoDetailActivity1: {}", video_data.bvid);
    }

    VideoDetailActivity(std::string bvid){
        video_data.bvid = std::move(bvid);
        Logger::error("create VideoDetailActivity2: {}", video_data.bvid);
    }

    void onContentAvailable() override{
        this->appletFrame->setHeaderVisibility(Visibility::GONE);
        this->startButton->registerClickAction([this](brls::View* view) {
            Logger::error("start video");
            this->video->start("http://vjs.zencdn.net/v/oceans.mp4");
//            this->video->start("http://www.w3schools.com/html/movie.mp4");
//            this->video->start("http://sample-videos.com/video123/flv/720/big_buck_bunny_720p_20mb.flv");
            return true;
        });
        this->video->registerClickAction([this](brls::View* view) {
            if(this->fullscreen){
                //全屏状态下切换播放状态
                this->video->togglePlay();
                this->video->showOSD();
            }else{
                //非全屏状态点击视频组件进入全屏
                this->setFullscreen();
            }
            return true;
        });

        BRLS_REGISTER_CLICK_BY_ID("video_intro", [this](brls::View* view){
            //todo：注意线程安全
            auto dialog = new brls::Dialog(this->videoIntroLabel->getFullText());
            dialog->addButton("ok", [](){});
            dialog->open();
            return true;
        });

        this->requestData(this->video_data);
    }

    void setFullscreen(){
        this->fullscreen = true;
        this->getView("video_detail_right_box")->setVisibility(Visibility::GONE);
        this->getView("video_detail_info_box")->setVisibility(Visibility::GONE);
        this->appletFrame->setFooterVisibility(Visibility::GONE);
        this->getView("video_detail_left_box")->setWidth(Application::contentWidth);
        this->getView("video_detail_left_box")->setMargins(0,0,0,0);
        this->video->setFullScreen(true);
        //按B退出全屏
        videoExitFullscreenID = this->video->registerAction("", BUTTON_B, [this](brls::View* view){
                this->exitFullscreen();
                return true;
            },true, false, SOUND_BACK);
    }

    void exitFullscreen(){
        this->fullscreen = false;
        this->getView("video_detail_right_box")->setVisibility(Visibility::VISIBLE);
        this->getView("video_detail_info_box")->setVisibility(Visibility::VISIBLE);
        this->appletFrame->setFooterVisibility(Visibility::VISIBLE);
        this->getView("video_detail_left_box")->setWidth(800);
        this->getView("video_detail_left_box")->setMargins(10,10,10,10);
        this->video->setFullScreen(false);
        this->video->setSize(Size(800, 480));
        //注销按B退出全屏的回调
        if(videoExitFullscreenID != -1){
            this->video->unregisterAction(videoExitFullscreenID);
            videoExitFullscreenID = -1;
        }
    }

    void onVideoInfo(const bilibili::VideoDetailResult &result) override{
        Logger::debug("[onVideoInfo] title:{} author:{}", result.title, result.owner.name);
        this->videoUserInfo->setUserInfo(result.owner.face, result.owner.name,result.owner.name);
        this->videoTitleLabel->setText(result.title);
        this->video->setTitle(result.title);
        this->videoIntroLabel->setText(result.desc);
        this->videoInfoLabel->setText("BVID: " + result.bvid);

    }

    void onVideoPageListInfo(const bilibili::VideoDetailPageListResult &result) override{
        for(const auto& i : result){
            Logger::debug("cid:{} title:{}", i.cid, i.part);
        }
    }

    void onVideoPlayUrl(const bilibili::VideoUrlResult & result) override{
        Logger::debug("quality: {}", result.quality);
        // todo: 将多个文件加入播放列表
        //todo: 播放失败时可以尝试备用播放链接
        for(const auto& i: result.durl){
            this->video->start(i.url);
            break;
        }
    }

    bool isTranslucent() {
        return true;
    }

    ~VideoDetailActivity() override{
        Logger::error("del VideoDetailActivity");
    }

private:
    BRLS_BIND(VideoView, video, "video/detail/video");
    BRLS_BIND(Button, startButton, "video/detail/button/start");
    BRLS_BIND(VideoView, video2, "video/detail/video2");
    BRLS_BIND(Button, startButton2, "video/detail/button/start2");
    BRLS_BIND(AppletFrame, appletFrame, "video/detail/frame");
    BRLS_BIND(UserInfoView, videoUserInfo, "video_author");
    BRLS_BIND(Label, videoTitleLabel, "video_title");
    BRLS_BIND(Label, videoIntroLabel, "video_intro");
    BRLS_BIND(Label, videoInfoLabel, "video_info");

    bilibili::Video video_data;
    bool fullscreen = false;

    brls::ActionIdentifier videoExitFullscreenID = -1;
};
