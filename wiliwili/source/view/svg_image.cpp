//
// Created by fang on 2022/9/17.
//

#include "view/svg_image.hpp"
#include "borealis/core/cache_helper.hpp"

SVGImage::SVGImage() {
    this->registerFilePathXMLAttribute("SVG", [this](const std::string& value) {
        this->setImageFromSVGFile(value);
    });

    // 交给缓存自动处理纹理的删除
    this->setFreeTexture(false);

    // 改变窗口大小时自动更新纹理
    subscription =
        brls::Application::getWindowSizeChangedEvent()->subscribe([this]() {
            if (!filePath.empty()) {
                brls::Visibility v = getVisibility();
                this->setVisibility(brls::Visibility::VISIBLE);
                setImageFromSVGFile(filePath);
                this->setVisibility(v);
            }
        });
}

void SVGImage::setImageFromSVGRes(const std::string& value) {
    this->setImageFromSVGFile(std::string(BRLS_RESOURCES) + value);
}

void SVGImage::setImageFromSVGFile(const std::string& value) {
    filePath   = value;
    size_t tex = this->getTexture();
    if (tex > 0) {
        brls::TextureCache::instance().removeCache(tex);
        brls::Logger::verbose("cache remove: {} {}", value, tex);
    }

    tex = brls::TextureCache::instance().getCache(value);
    if (tex > 0) {
        brls::Logger::verbose("cache hit: {} {}", value, tex);
        this->innerSetImage(tex);
        return;
    }

    this->document = lunasvg::Document::loadFromFile(value);
    if (this->document) {
        this->updateBitmap();
    } else {
        brls::Logger::error("cannot load svg image: {}", value);
        return;
    }

    tex = this->getTexture();
    if (tex > 0) {
        brls::Logger::verbose("cache svg: {} {}", value, tex);
        brls::TextureCache::instance().addCache(value, tex);
    } else {
        brls::Logger::error("svg got zero tex: {} {}", value, tex);
    }
}

void SVGImage::setImageFromSVGString(const std::string& value) {
    this->document = lunasvg::Document::loadFromData(value);
    if (this->document) {
        this->updateBitmap();
    } else {
        brls::Logger::error("cannot load svg image: {}", value);
    }
}

void SVGImage::updateBitmap() {
    if (!this->document) return;

    float width  = this->getWidth() * brls::Application::windowScale;
    float height = this->getHeight() * brls::Application::windowScale;
    auto bitmap  = this->document->renderToBitmap(width, height);
    bitmap.convertToRGBA();
    NVGcontext* vg = brls::Application::getNVGContext();
    int tex        = nvgCreateImageRGBA(vg, bitmap.width(), bitmap.height(), 0,
                                        bitmap.data());
    this->innerSetImage(tex);
}

void SVGImage::rotate(float value) { this->angle = value; }

SVGImage::~SVGImage() {
    brls::Application::getWindowSizeChangedEvent()->unsubscribe(subscription);
}

brls::View* SVGImage::create() { return new SVGImage(); }

void SVGImage::draw(NVGcontext* vg, float x, float y, float width, float height,
                    brls::Style style, brls::FrameContext* ctx) {
    if (this->texture == 0) return;

    nvgSave(vg);
    float cx = width / 2, cy = height / 2;
    nvgTranslate(vg, x + cx, y + cy);
    nvgRotate(vg, this->angle);

    this->paint.xform[4] = -cx;
    this->paint.xform[5] = -cy;

    nvgBeginPath(vg);
    nvgRoundedRect(vg, -cx, -cy, width, height, getCornerRadius());
    nvgFillPaint(vg, a(this->paint));
    nvgFill(vg);

    nvgRestore(vg);
}
