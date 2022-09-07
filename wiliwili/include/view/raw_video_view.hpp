//
// Created by fang on 2022/7/17.
//

// register this view in main.cpp
//#include "view/raw_video_view.hpp"
//    brls::Application::registerXMLView("RawVideoView", RawVideoView::create);
// <brls:View xml=@res/xml/views/raw_video_view.xml

#pragma once

#include <borealis.hpp>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
}

typedef enum VideoPlayState {
    VIDEO_PLAYING,
    VIDEO_STOPPED,
    VIDEO_LOADING,
    VIDEO_PAUSED,
} VideoPlayState;

class RawVideoView : public brls::Box {
public:
    RawVideoView();

    ~RawVideoView();

    static View *create();

    int playVideo(std::string path);

    void draw(NVGcontext *vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext *ctx) override;

    void InitVideo() {
        frame       = av_frame_alloc();
        this->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!this->codec) brls::fatal("H264 Codec not available");

        this->codec_context = avcodec_alloc_context3(codec);
        if (!this->codec_context) brls::fatal("Failed to alloc codec context");

        // 跳过环路滤波（减少CPU占用，不过画面可能会有小方块）
        //        this->codec_context->skip_loop_filter = AVDISCARD_ALL;
        this->codec_context->flags |= AV_CODEC_FLAG_LOW_DELAY;
        this->codec_context->flags2 |= AV_CODEC_FLAG2_FAST;
        // this->codec_context->flags2 |= AV_CODEC_FLAG2_CHUNKS;
        this->codec_context->thread_type  = FF_THREAD_SLICE;
        this->codec_context->thread_count = 4;

        if (avcodec_open2(this->codec_context, this->codec, nullptr) < 0) {
            avcodec_free_context(&this->codec_context);
            brls::fatal("Failed to open codec context");
        }
    }

    bool FreeVideo() {
        bool ret = true;

        if (this->frame) av_frame_free(&this->frame);

        // avcodec_alloc_context3(codec);
        //        if(this->codec_context)
        //        {
        //            avcodec_close(this->codec_context);
        //            avcodec_free_context(&this->codec_context);
        //        }

        return ret;
    }

    bool VideoCB(uint8_t *buf, size_t buf_size) {
        // callback function to decode video buffer

        AVPacket packet;
        av_init_packet(&packet);
        packet.data         = buf;
        packet.size         = buf_size;
        AVFrame *temp_frame = av_frame_alloc();
        if (!temp_frame) {
            brls::Logger::error("UpdateFrame Failed to alloc AVFrame");
            av_packet_unref(&packet);
            return false;
        }

    send_packet:
        // Push
        int r = avcodec_send_packet(this->codec_context, &packet);
        if (r != 0) {
            if (r == AVERROR(EAGAIN)) {
                brls::Logger::error(
                    "AVCodec internal buffer is full removing frames before "
                    "pushing");
                r = avcodec_receive_frame(this->codec_context, temp_frame);
                // send decoded frame for sdl texture update
                if (r != 0) {
                    brls::Logger::error("Failed to pull frame");
                    av_frame_free(&temp_frame);
                    av_packet_unref(&packet);
                    return false;
                }
                goto send_packet;
            } else {
                char errbuf[128];
                av_make_error_string(errbuf, sizeof(errbuf), r);
                brls::Logger::error("Failed to push frame: {}", errbuf);
                av_frame_free(&temp_frame);
                av_packet_unref(&packet);
                return false;
            }
        }

        this->mtx.lock();
        // Pull
        r = avcodec_receive_frame(this->codec_context, this->frame);
        this->mtx.unlock();

        if (r != 0) brls::Logger::error("Failed to pull frame");

        av_frame_free(&temp_frame);
        av_packet_unref(&packet);
        return true;
    }

private:
    int image                    = 0;
    VideoPlayState mVideoPlaySta = VIDEO_PLAYING;
    AVFrame *frame;
    AVCodec *codec;
    AVCodecContext *codec_context;
    std::mutex mtx;
    struct SwsContext *pSwsCtx;
    //    AVCodecContext *pAVctx;
    // BRLS_BIND(brls::Label, label, "RawVideoView/label");
};