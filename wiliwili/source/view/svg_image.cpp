//
// Created by fang on 2022/9/17.
//

#include "view/svg_image.hpp"

#include <borealis/core/application.hpp>
#include <borealis/core/cache_helper.hpp>

SVGImage::SVGImage() {
    this->registerFilePathXMLAttribute("SVG", [this](const std::string& value) { this->setImageFromSVGFile(value); });

    // 交给缓存自动处理纹理的删除
    this->setFreeTexture(false);

    this->registerFloatXMLAttribute("width", [this](float value) { this->setWidth(value); });
    this->registerFloatXMLAttribute("height", [this](float value) { this->setHeight(value); });
    this->registerAutoXMLAttribute("width", [] { brls::fatal("SVGImage: width cannot be auto."); });
    this->registerAutoXMLAttribute("height", [] { brls::fatal("SVGImage: height cannot be auto."); });
    this->registerPercentageXMLAttribute("width",
                                         [](float value) { brls::fatal("SVGImage: width cannot be percentage."); });
    this->registerPercentageXMLAttribute("height",
                                         [](float value) { brls::fatal("SVGImage: height cannot be percentage."); });

    // 改变窗口大小时自动更新纹理
    subscription = brls::Application::getWindowSizeChangedEvent()->subscribe([this]() {
        if (!filePath.empty()) {
            // 重新生成纹理时，如果svg组件本身是被隐藏的，通过 getWidth() 和 getHeight() 获取到的值是 0，无法生成正确的尺寸
            // 之前的做法是临时将组件显示出来，生成纹理后再隐藏，但对于Box中的svg组件，若组件本身是非隐藏的，但Box是隐藏的，同样无法获取尺寸
            // 目前的做法是直接使用设置尺寸时的 _width 和 _height 作为尺寸，缺点是svg组件只能使用固定的尺寸，无法自适应
            setImageFromSVGFile(filePath);
        }
    });
}

void SVGImage::setImageFromSVGRes(const std::string& value) {
#ifdef USE_LIBROMFS
    filePath = "@res/" + value;
    if (checkCache(filePath) > 0) return;
    auto image     = romfs::get(value);
    this->document = lunasvg::Document::loadFromData((const char*)image.string().data(), image.size());
    if (this->document) {
        this->updateBitmap();
    } else {
        brls::Logger::error("setImageFromSVGRes: cannot load svg image: {}", value);
        return;
    }

    size_t tex = this->getTexture();
    if (tex > 0) {
        brls::Logger::verbose("cache svg: {} {}", value, tex);
        brls::TextureCache::instance().addCache("@res/" + value, tex);
    } else {
        brls::Logger::error("svg got zero tex: {} {}", value, tex);
    }
#else
    this->setImageFromSVGFile(std::string(BRLS_RESOURCES) + value);
#endif
}

void SVGImage::setImageFromSVGFile(const std::string& value) {
    filePath = value;
#ifdef USE_LIBROMFS
    if (value.rfind("@res/", 0) == 0) return this->setImageFromSVGRes(value.substr(5));
#endif
    if (checkCache(value) > 0) return;

    this->document = lunasvg::Document::loadFromFile(value);
    if (this->document) {
        this->updateBitmap();
    } else {
        brls::Logger::error("setImageFromSVGFile: cannot load svg image: {}", value);
        return;
    }

    size_t tex = this->getTexture();
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
        brls::Logger::error("setImageFromSVGString: cannot load svg image: {}", value);
    }
}

void SVGImage::updateBitmap() {
    if (!this->document) return;

    float width  = this->_width * brls::Application::windowScale;
    float height = this->_height * brls::Application::windowScale;
    auto bitmap  = this->document->renderToBitmap(width, height);
    bitmap.convertToRGBA();
    NVGcontext* vg = brls::Application::getNVGContext();
    int tex        = nvgCreateImageRGBA(vg, bitmap.width(), bitmap.height(), 0, bitmap.data());
    if (tex <= 0) {
        brls::Logger::error("svg: {} update bitmap with texture 0.", filePath);
        return;
    }
    this->innerSetImage(tex);
}

void SVGImage::rotate(float value) { this->angle = value; }

SVGImage::~SVGImage() { brls::Application::getWindowSizeChangedEvent()->unsubscribe(subscription); }

brls::View* SVGImage::create() { return new SVGImage(); }

void SVGImage::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                    brls::FrameContext* ctx) {
    if (this->texture == 0) return;

    nvgSave(vg);
    float cx = width / 2, cy = height / 2;
    nvgTranslate(vg, x + cx, y + cy);
    nvgRotate(vg, this->angle);

    this->paint.xform[4] = -cx;
    this->paint.xform[5] = -cy;

    nvgBeginPath(vg);
    if (this->getCornerRadius() > 0.0f)
        nvgRoundedRect(vg, -cx, -cy, width, height, getCornerRadius());
    else
        nvgRect(vg, -cx, -cy, width, height);
    nvgFillPaint(vg, a(this->paint));
    nvgFill(vg);

    nvgRestore(vg);
}

void SVGImage::setWidth(float width) {
    _width = width;
    brls::View::setWidth(width);
}

void SVGImage::setHeight(float height) {
    _height = height;
    brls::View::setHeight(height);
}

void SVGImage::setDimensions(float width, float height) {
    _width  = width;
    _height = height;
    brls::View::setDimensions(width, height);
}

void SVGImage::setSize(brls::Size size) {
    _width  = size.width;
    _height = size.height;
    brls::View::setSize(size);
}