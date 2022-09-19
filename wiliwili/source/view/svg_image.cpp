//
// Created by fang on 2022/9/17.
//

#include "view/svg_image.hpp"
#include "utils/cache_helper.hpp"

SVGImage::SVGImage() {
    this->registerFilePathXMLAttribute(
        "SVG", [this](std::string value) { this->setImageFromSVGFile(value); });

    // 交给缓存自动处理纹理的删除
    this->setFreeTexture(false);
}

void SVGImage::setImageFromSVGRes(std::string name) {
    this->setImageFromSVGFile(std::string(BRLS_RESOURCES) + name);
}

void SVGImage::setImageFromSVGFile(const std::string value) {
    int tex = TextureCache::instance().getCache(value);
    if (tex > 0) {
        brls::Logger::verbose("cache hit: {} {}", value, tex);
        this->innerSetImage(tex);
        return;
    }

    this->document = lunasvg::Document::loadFromFile(value.c_str());
    this->updateBitmap();

    tex = this->getTexture();
    if (tex > 0) {
        brls::Logger::verbose("cache svg: {} {}", value, tex);
        TextureCache::instance().addCache(value, tex);
    }
}

void SVGImage::setImageFromSVGString(const std::string value) {
    this->document = lunasvg::Document::loadFromData(value);
    this->updateBitmap();
}

void SVGImage::updateBitmap() {
    float width  = this->getWidth() * brls::Application::windowScale;
    float height = this->getHeight() * brls::Application::windowScale;
    auto bitmap  = this->document->renderToBitmap(width, height);
    bitmap.convertToRGBA();
    NVGcontext* vg = brls::Application::getNVGContext();
    int tex        = nvgCreateImageRGBA(vg, bitmap.width(), bitmap.height(), 0,
                                        bitmap.data());
    this->innerSetImage(tex);
}

brls::View* SVGImage::create() { return new SVGImage(); }