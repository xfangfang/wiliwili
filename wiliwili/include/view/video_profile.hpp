//
// Created by fang on 2023/4/1.
//

// register this view in main.cpp
//#include "view/video_profile.hpp"
//    brls::Application::registerXMLView("VideoProfile", VideoProfile::create);
// <brls:View xml=@res/xml/views/video_profile.xml

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

namespace brls {
class Label;
}

class VideoProfile : public brls::Box {
public:
    VideoProfile();

    void update();

    void draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext *ctx) override;

    ~VideoProfile() override;

    static View *create();

    BRLS_BIND(brls::Label, labelUrl, "profile/file/url");
    BRLS_BIND(brls::Label, labelSize, "profile/file/size");
    BRLS_BIND(brls::Label, labelFormat, "profile/file/format");
    BRLS_BIND(brls::Label, labelCache, "profile/file/cache");

    BRLS_BIND(brls::Label, labelVideoRes, "profile/video/res");
    BRLS_BIND(brls::Label, labelVideoCodec, "profile/video/codec");
    BRLS_BIND(brls::Label, labelVideoPixel, "profile/video/pixel");
    BRLS_BIND(brls::Label, labelVideoHW, "profile/video/hwdec");
    BRLS_BIND(brls::Label, labelVideoBitrate, "profile/video/bitrate");
    BRLS_BIND(brls::Label, labelVideoDrop, "profile/video/drop");
    BRLS_BIND(brls::Label, labelVideoSync, "profile/video/avsync");

    BRLS_BIND(brls::Label, labelAudioChannel, "profile/audio/channel");
    BRLS_BIND(brls::Label, labelAudioCodec, "profile/audio/codec");
    BRLS_BIND(brls::Label, labelAudioSampleRate, "profile/audio/sample");
    BRLS_BIND(brls::Label, labelAudioBitrate, "profile/audio/bitrate");
};