//
// Created by fang on 2022/5/26.
//

#pragma once

#include <borealis.hpp>
#include "net_image.hpp"
#include "utils/number_helper.hpp"
#include "utils/svg_icon.hpp"

using namespace brls;

class VideoCardView : public brls::Box {

public:
    VideoCardView(){
        this->inflateFromXMLRes("xml/views/video_card.xml");
    }

    void setCard(std::string pic, std::string title, std::string username, int pubdate,
                 int view_count=0, int danmaku=0, int duration=0
                 ){
        this->labelUsername->setText(username + "·" +sec2date(pubdate));
        this->labelTitle->setIsWrapping(true);
        this->labelTitle->setText(title);
        this->picture->setImageFromNet(pic);
        this->labelCount->setText("播放: "+num2w(view_count));
        this->labelDanmaku->setText("弹幕: "+num2w(danmaku));
        this->labelDuration->setText(sec2Time(duration));
    }

    void draw(NVGcontext *vg, float x, float y, float width, float height, Style style, FrameContext *ctx) {
        Box::draw(vg, x, y, width, height, style, ctx);
//        RenderWidgetUp(vg);

    }

    static brls::View* create(){
        return new VideoCardView();
    }

private:
    BRLS_BIND(NetImage, picture, "video/card/picture");
    BRLS_BIND(Label, labelTitle, "video/card/label/title");

    BRLS_BIND(Label, labelUsername, "video/card/label/username");
    BRLS_BIND(Label, labelCount, "video/card/label/count");
    BRLS_BIND(Label, labelDanmaku, "video/card/label/danmaku");
    BRLS_BIND(Label, labelDuration, "video/card/label/duration");

//    BRLS_BIND(brls::Image, imageUp, "video/card/image/up");
//    BRLS_BIND(brls::Image, imageCount, "video/card/image/count");
//    BRLS_BIND(brls::Image, imageDanmaku, "video/card/image/danmaku");
//    BRLS_BIND(brls::Image, imageDuration, "video/card/image/duration");
};