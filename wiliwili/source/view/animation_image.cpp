//
// Created by fang on 2022/12/27.
//

#include <borealis/core/application.hpp>
#include <borealis/core/cache_helper.hpp>

#include "view/animation_image.hpp"

AnimationImage::AnimationImage() {
    brls::Logger::debug("View AnimationImage: create");

    registerFloatXMLAttribute("frame", [this](float value) { this->setFrame(value); });

    registerFloatXMLAttribute("frameTime", [this](float value) { this->setFrameTime(value); });

    registerFilePathXMLAttribute("image", [this](std::string path) { this->setImage(path); });

    this->setClipsToBounds(false);
}

void AnimationImage::setFrame(size_t value) {
    this->frame = value;
    this->refreshImage();
}

void AnimationImage::setImage(const std::string& value) {
    if (texture) {
        brls::TextureCache::instance().removeCache(texture);
        texture = 0;
    }
    texture = brls::TextureCache::instance().getCache(value);
    auto vg = brls::Application::getNVGContext();
    if (!texture) {
#ifdef USE_LIBROMFS
        auto image = romfs::get(value.substr(5));
        texture    = nvgCreateImageMem(vg, 0, (unsigned char*)image.string().data(), (int)image.size());
#else
        texture = nvgCreateImage(vg, value.c_str(), 0);
#endif
        if (texture) brls::TextureCache::instance().addCache(value, texture);
    }
    this->refreshImage();
}

void AnimationImage::refreshImage() {
    if (!texture) return;

    auto vg     = brls::Application::getNVGContext();
    this->paint = nvgImagePattern(vg, 0, 0, getWidth() * frame, getHeight(), 0, this->texture, 1.0f);
}

void AnimationImage::setFrameTime(size_t value) { this->frame_time = value; }

AnimationImage::~AnimationImage() {
    brls::Logger::debug("View AnimationImage: delete");
    if (texture != 0) brls::TextureCache::instance().removeCache(texture);
}

brls::View* AnimationImage::create() { return new AnimationImage(); }
void AnimationImage::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                          brls::FrameContext* ctx) {
    brls::Application::setActiveEvent(true);
    size_t time_now = brls::getCPUTimeUsec();
    if (time_now - last_refresh_time > frame_time) {
        last_refresh_time = time_now;
        last_x            = width * current_frame;
        current_frame++;
        current_frame %= frame;
    }

    this->paint.xform[4] = x - last_x;
    this->paint.xform[5] = y;

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, width, height, getCornerRadius());
    nvgFillPaint(vg, a(this->paint));
    nvgFill(vg);
}

void AnimationImage::onLayout() { this->refreshImage(); }
