//
// Created by fang on 2022/5/26.
//

#pragma once

#include <borealis.hpp>
#include "net_image.hpp"
#include "utils/number_helper.hpp"

using namespace brls;

class VideoCardView : public brls::Box {

public:
    VideoCardView(){
        this->inflateFromXMLRes("xml/views/video_card.xml");
    }

    void setCard(std::string pic, std::string title, std::string username, int pubdate,
                 int view_count=0, int danmaku=0, int duration=0
                 ){
        this->labelUsername->setText(username + "·" +wiliwili::sec2date(pubdate));
        this->labelTitle->setIsWrapping(true);
        this->labelTitle->setText(title);
        this->picture->setImageFromNet(pic);
        this->labelCount->setText(""_i18n +wiliwili::num2w(view_count));
        this->labelDanmaku->setText(wiliwili::num2w(danmaku));
        this->labelDuration->setText(wiliwili::sec2Time(duration));
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
};