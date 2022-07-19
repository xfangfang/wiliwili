//
// Created by fang on 2022/7/10.
//

#include <borealis.hpp>
#include "view/video_view.hpp"
#include "view/user_info.hpp"
#include "activity/player_activity.hpp"


class DataSourceCommentList
        : public RecyclingGridDataSource
{
public:
    DataSourceCommentList(bilibili::VideoCommentListResult result):dataList(result){

    }
    RecyclingGridItem* cellForRow(RecyclingGrid* recycler, size_t index) override{
        //从缓存列表中取出 或者 新生成一个表单项
        VideoComment* item = (VideoComment*)recycler->dequeueReusableCell("Cell");

        item->setData(this->dataList[index]);
        return item;
    }

    size_t getItemCount() override{
        return dataList.size();
    }

    void onItemSelected(RecyclingGrid* recycler, size_t index) override{

    }

    void appendData(const bilibili::VideoCommentListResult& data){
        this->dataList.insert(this->dataList.end(), data.begin(), data.end());
    }

    void clearData() override{
        this->dataList.clear();
    }

private:
    bilibili::VideoCommentListResult dataList;
};

PlayerActivity::PlayerActivity(bilibili::Video video): video_data(std::move(video)){
    Logger::error("create VideoDetailActivity1: {}", video_data.bvid);
}

PlayerActivity::PlayerActivity(std::string bvid){
    video_data.bvid = std::move(bvid);
    Logger::error("create VideoDetailActivity2: {}", video_data.bvid);
}

void PlayerActivity::onContentAvailable() {
//    this->startButton->registerClickAction([this](brls::View* view) {
//        Logger::error("start video");
//        this->video->start("http://vjs.zencdn.net/v/oceans.mp4");
//        //            this->video->start("http://www.w3schools.com/html/movie.mp4");
////                    this->video->start("http://sample-videos.com/video123/flv/720/big_buck_bunny_720p_20mb.flv");
//        return true;
//    });
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

    this->video->registerAction("Debug", brls::ControllerButton::BUTTON_Y, [this](brls::View* view){
        this->video->stop();
        return true;
    });

BRLS_REGISTER_CLICK_BY_ID("video_intro", [this](brls::View* view){
    //todo：注意线程安全
    auto dialog = new brls::Dialog(this->videoIntroLabel->getFullText());
        dialog->addButton("ok", [](){});
        dialog->open();
        return true;
    });

    recyclingGrid->registerCell("Cell", []() { return VideoComment::create(); });
    recyclingGrid->onNextPage([](){

    });
    this->requestData(this->video_data);
}

void PlayerActivity::setFullscreen(){
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

void PlayerActivity::exitFullscreen(){
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

void PlayerActivity::onVideoInfo(const bilibili::VideoDetailResult &result) {
    Logger::debug("[onVideoInfo] title:{} author:{}", result.title, result.owner.name);
    brls::sync([this, result](){
        this->videoUserInfo->setUserInfo(result.owner.face, result.owner.name,result.owner.name);
        this->videoTitleLabel->setText(result.title);
        this->video->setTitle(result.title);
        this->videoIntroLabel->setText(result.desc);
        this->videoInfoLabel->setText("BVID: " + result.bvid);
    });

}

void PlayerActivity::onVideoPageListInfo(const bilibili::VideoDetailPageListResult &result) {
    for(const auto& i : result){
        Logger::debug("cid:{} title:{}", i.cid, i.part);
    }
}

void PlayerActivity::onVideoPlayUrl(const bilibili::VideoUrlResult & result) {
    Logger::debug("quality: {}", result.quality);
    // todo: 将多个文件加入播放列表
    //todo: 播放失败时可以尝试备用播放链接
    brls::sync([this, result](){
        for(const auto& i: result.durl){
            this->video->start(i.url);
            break;
        }
    });
}


PlayerActivity::~PlayerActivity() {
    Logger::error("del PlayerActivity");
    this->video->stop();
}

void PlayerActivity::onCommentInfo(const bilibili::VideoCommentResultWrapper &result) {
    brls::Logger::debug("on comment info 1");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    brls::Logger::debug("on comment info 2");
    brls::sync([this, result]() {
//        for(auto i: result.top_replies){
//            brls::Logger::debug("comment: {}", i.content.message);
//
//            break;
//        }

//        for(auto i : result.replies){
//            brls::Logger::debug("comment: {}", i.content.message);
//        }

        this->recyclingGrid->setDataSource(new DataSourceCommentList(result.replies));
    });
}
