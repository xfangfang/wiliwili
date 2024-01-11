//
// Created by fang on 2023/4/1.
//

#include <borealis/core/application.hpp>
#include <borealis/views/label.hpp>

#include "view/video_profile.hpp"
#include "view/mpv_core.hpp"

VideoProfile::VideoProfile() {
    this->inflateFromXMLRes("xml/views/video_profile.xml");
    brls::Logger::debug("View VideoProfile: create");
}

void VideoProfile::update() {
    auto mpvCore = &MPVCore::instance();

    // file
    if (mpvCore->filepath != labelUrl->getFullText()) labelUrl->setText(mpvCore->filepath);
    labelSize->setText(fmt::format("{:.2f}MB", mpvCore->getInt("file-size") / 1048576.0));
    labelFormat->setText(mpvCore->getString("file-format"));
    auto cache = mpvCore->getNodeMap("demuxer-cache-state");
    labelCache->setText(fmt::format("{:.2f}MB ({:.1f} sec)", cache["fw-bytes"].u.int64 / 1048576.0,
                                    mpvCore->getDouble("demuxer-cache-duration")));
    brls::Logger::debug(
        "total-bytes: {:.2f}MB; cache-duration: "
        "{:.2f}; "
        "underrun: {}; fw-bytes: {:.2f}MB; bof-cached: "
        "{}; eof-cached: {}; file-cache-bytes: {}; "
        "raw-input-rate: {:.2f};",
        cache["total-bytes"].u.int64 / 1048576.0, cache["cache-duration"].u.double_, cache["underrun"].u.flag,
        cache["fw-bytes"].u.int64 / 1048576.0, cache["bof-cached"].u.flag, cache["eof-cached"].u.flag,
        cache["file-cache-bytes"].u.int64 / 1048576.0, cache["raw-input-rate"].u.int64 / 1048576.0);

    // video
    labelVideoRes->setText(fmt::format(
        "{} x {}@{} (window: {} x {} framebuffer: {} x {})", mpvCore->getInt("video-params/w"),
        mpvCore->getInt("video-params/h"), mpvCore->getInt("container-fps"), brls::Application::contentWidth,
        brls::Application::contentHeight, brls::Application::windowWidth, brls::Application::windowHeight));
    labelVideoCodec->setText(mpvCore->getString("video-codec"));
    labelVideoPixel->setText(mpvCore->getString("video-params/pixelformat"));
    labelVideoHW->setText(mpvCore->getString("hwdec-current"));
    labelVideoBitrate->setText(std::to_string(mpvCore->getInt("video-bitrate") / 1024) + "kbps");
    labelVideoDrop->setText(fmt::format("{} (decoder) {} (output)", mpvCore->getInt("decoder-frame-drop-count"),
                                        mpvCore->getInt("frame-drop-count")));
    labelVideoSync->setText(fmt::format("{:.5f}", mpvCore->getDouble("avsync")));

    // audio
    labelAudioCodec->setText(mpvCore->getString("audio-codec"));
    labelAudioChannel->setText(mpvCore->getString("audio-params/channel-count"));
    labelAudioSampleRate->setText(std::to_string(mpvCore->getInt("audio-params/samplerate") / 1000) + "kHz");
    labelAudioBitrate->setText(std::to_string(mpvCore->getInt("audio-bitrate") / 1024) + "kbps");
}

void VideoProfile::draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
                        brls::FrameContext *ctx) {
    static brls::Time time = 0;
    brls::Time now         = brls::getCPUTimeUsec();
    if ((now - time) > 1000000) {
        this->update();
        time = now;
    }

    Box::draw(vg, x, y, width, height, style, ctx);
}

VideoProfile::~VideoProfile() { brls::Logger::debug("View VideoProfile: delete"); }

brls::View *VideoProfile::create() { return new VideoProfile(); }