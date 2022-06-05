//
// Created by fang on 2022/6/5.
//

#pragma once
#include <nanosvg.h>
#include <borealis.hpp>
#include <cpr/cpr.h>
#include <QrCode.hpp>
#include <cstring>
#include <lunasvg.h>

namespace math{

    struct Vec {
        float x = 0.f;
        float y = 0.f;

        Vec() {}
        Vec(float xy) : x(xy), y(xy) {}
        Vec(float x, float y) : x(x), y(y) {}

        float& operator[](int i) {
            return (i == 0) ? x : y;
        }
        const float& operator[](int i) const {
            return (i == 0) ? x : y;
        }
        /** Negates the vector.
        Equivalent to a reflection across the `y = -x` line.
        */
        Vec neg() const {
            return Vec(-x, -y);
        }
        Vec plus(Vec b) const {
            return Vec(x + b.x, y + b.y);
        }
        Vec minus(Vec b) const {
            return Vec(x - b.x, y - b.y);
        }
        Vec mult(float s) const {
            return Vec(x * s, y * s);
        }
        Vec mult(Vec b) const {
            return Vec(x * b.x, y * b.y);
        }
        Vec div(float s) const {
            return Vec(x / s, y / s);
        }
        Vec div(Vec b) const {
            return Vec(x / b.x, y / b.y);
        }
        float dot(Vec b) const {
            return x * b.x + y * b.y;
        }
        float arg() const {
            return std::atan2(y, x);
        }
        float norm() const {
            return std::hypot(x, y);
        }
        Vec normalize() const {
            return div(norm());
        }
        float square() const {
            return x * x + y * y;
        }
        float area() const {
            return x * y;
        }
        /** Rotates counterclockwise in radians. */
        Vec rotate(float angle) {
            float sin = std::sin(angle);
            float cos = std::cos(angle);
            return Vec(x * cos - y * sin, x * sin + y * cos);
        }
        /** Swaps the coordinates.
        Equivalent to a reflection across the `y = x` line.
        */
        Vec flip() const {
            return Vec(y, x);
        }
        Vec min(Vec b) const {
            return Vec(std::fmin(x, b.x), std::fmin(y, b.y));
        }
        Vec max(Vec b) const {
            return Vec(std::fmax(x, b.x), std::fmax(y, b.y));
        }
        Vec abs() const {
            return Vec(std::fabs(x), std::fabs(y));
        }
        Vec round() const {
            return Vec(std::round(x), std::round(y));
        }
        Vec floor() const {
            return Vec(std::floor(x), std::floor(y));
        }
        Vec ceil() const {
            return Vec(std::ceil(x), std::ceil(y));
        }
        bool equals(Vec b) const {
            return x == b.x && y == b.y;
        }
        bool isZero() const {
            return x == 0.f && y == 0.f;
        }
        bool isFinite() const {
            return std::isfinite(x) && std::isfinite(y);
        }
        Vec clamp(Rect bound) const;
        Vec clampSafe(Rect bound) const;
        Vec crossfade(Vec b, float p) {
            return this->plus(b.minus(*this).mult(p));
        }

        // Method aliases
        bool isEqual(Vec b) const {
            return equals(b);
        }
    };

    static float getLineCrossing(math::Vec p0, math::Vec p1, math::Vec p2, math::Vec p3) {
        math::Vec b = p2.minus(p0);
        math::Vec d = p1.minus(p0);
        math::Vec e = p3.minus(p2);
        float m = d.x * e.y - d.y * e.x;
        // Check if lines are parallel, or if either pair of points are equal
        if (std::abs(m) < 1e-6)
            return NAN;
        return -(d.x * b.y - d.y * b.x) / m;
    }
}

static NVGcolor getNVGColor(uint32_t color) {
    return nvgRGBA(
            (color >> 0) & 0xff,
            (color >> 8) & 0xff,
            (color >> 16) & 0xff,
            (color >> 24) & 0xff);
}

class SVGImage : public brls::Image {

public:
    const char* SVG_UNIT = "px";
    SVGImage(){
        this->registerFilePathXMLAttribute("svg", [this](std::string value) {
            this->setImageFromSVGFile(value);
        });

        this->registerColorXMLAttribute("FillColor", [this](NVGcolor value){
            this->setFillColor(value);
        });

        this->registerColorXMLAttribute("BackgroundColor", [this](NVGcolor value){
            this->setBackgroundColor(value);
        });

        this->registerBoolXMLAttribute("TextureMode", [this](bool value){
            this->textureMode = value;
        });

        this->setScalingType(brls::ImageScalingType::FIT);
        this->setInterpolation(brls::ImageInterpolation::NEAREST);
    }

    ~SVGImage()
    {
        nsvgDelete(this->image);
    }


    void draw(NVGcontext* vg, float x, float y, float width, float height, Style style, FrameContext* ctx){
        if(textureMode){
            return brls::Image::draw(vg, x, y, width, height, style, ctx);
        }

        if(this->backgroundColor.a > 0){
            nvgBeginPath(vg);
            nvgFillColor(vg, a(backgroundColor));
            nvgRect(vg, x , y, width, height);
            nvgFill(vg);
        }


        if (this->image == nullptr)
            return;

        float x0, y0, raw_w, raw_h, scale;

        raw_w = this->image->width;
        raw_h = this->image->height;

        if (width/raw_w > height/raw_h) {
            scale = height / raw_h;
            x0 = x + (width - raw_w * scale) / 2;
            y0 = y;
        } else {
            scale = width / raw_w;
            x0 = x;
            y0 = y + (height - raw_h * scale) / 2;
        }



        for (auto shape = this->image->shapes; shape != NULL; shape = shape->next) {

            nvgSave(vg);
            nvgBeginPath(vg);

            // Opacity
            this->setAlpha(shape->opacity);

            for (auto path = shape->paths; path != NULL; path = path->next) {
                nvgMoveTo(vg, x0 + path->pts[0] * scale, y0 + path->pts[1] * scale);
                for (auto i = 1; i < path->npts; i += 3) {
                    float* p = &path->pts[i*2];
                    nvgBezierTo(vg, x0 + p[0] * scale,y0 + p[1] * scale,
                                x0 + p[2] * scale,y0 + p[3] * scale,
                                x0 + p[4] * scale,y0 + p[5] * scale);
                }
                if(path->closed)
                    nvgClosePath(vg);

                //Thanks to: https://github.com/VCVRack/Rack/blob/05fa24a72bccf4023f5fb1b0fa7f1c26855c0926/src/window/Svg.cpp#L201
                int crossings = 0;
                math::Vec p0 = math::Vec(path->pts[0], path->pts[1]);
                math::Vec p1 = math::Vec(path->bounds[0] - 1.0, path->bounds[1] - 1.0);

                // Iterate all other paths
                for (NSVGpath* path2 = shape->paths; path2; path2 = path2->next) {
                    if (path2 == path)
                        continue;

                    // Iterate all lines on the path
                    if (path2->npts < 4)
                        continue;
                    for (int i = 1; i < path2->npts + 3; i += 3) {
                        float* p = &path2->pts[2 * i];
                        // The previous point
                        math::Vec p2 = math::Vec(p[-2], p[-1]);
                        // The current point
                        math::Vec p3 = (i < path2->npts) ? math::Vec(p[4], p[5]) : math::Vec(path2->pts[0], path2->pts[1]);
                        float crossing = math::getLineCrossing(p0, p1, p2, p3);
                        float crossing2 = math::getLineCrossing(p2, p3, p0, p1);
                        if (0.0 <= crossing && crossing < 1.0 && 0.0 <= crossing2) {
                            crossings++;
                        }
                    }
                }

                if (crossings % 2 == 0)
                    nvgPathWinding(vg, NVG_SOLID);
                else
                    nvgPathWinding(vg, NVG_HOLE);
            }

            // Fill shape
            if (shape->fill.type) {
                if(this->color.a > 0)
                    nvgFillColor(vg, a(this->color));
                else
                    nvgFillColor(vg, a(getNVGColor(shape->fill.color)));
                nvgFill(vg);
            }

            // Stroke shape
            if (shape->stroke.type) {
                nvgStrokeWidth(vg, shape->strokeWidth);
                // strokeDashOffset, strokeDashArray, strokeDashCount not yet supported
                nvgLineCap(vg, (NVGlineCap) shape->strokeLineCap);
                nvgLineJoin(vg, (int) shape->strokeLineJoin);

                nvgStrokeColor(vg, a(getNVGColor(shape->stroke.color)));
                nvgStroke(vg);
            }
            nvgRestore(vg);
        }

    }

    void setFillColor(NVGcolor c){
        this->color = c;
    }

    NVGcolor getFillColor(){
        return this->color;
    }

    void setBackgroundColor(NVGcolor c){
        this->backgroundColor = c;
    }

    NVGcolor getBackgroundColor(){
        return this->backgroundColor;
    }

    void setImageFromSVGRes(std::string name){
        this->setImageFromSVGFile(std::string(BRLS_RESOURCES) + name);
    }

    void setImageFromSVGFile(const std::string value){
        if(textureMode){
            auto document = lunasvg::Document::loadFromFile(value.c_str());
            auto bitmap = document->renderToBitmap(this->getWidth() * brls::Application::windowScale,
                                                   this->getHeight() * brls::Application::windowScale);
            bitmap.convertToRGBA();
            NVGcontext* vg = Application::getNVGContext();
            this->innerSetImate(nvgCreateImageRGBA(vg,
                                                   bitmap.width(), bitmap.height(),
                                                   0, bitmap.data()));
        }else
            this->image = nsvgParseFromFile(value.c_str(), SVGImage::SVG_UNIT, 96);

    }

    void setImageFromSVGString(const std::string value){
        Logger::error("value: {}", value);
        if(char* buffer = (char*)malloc(value.length() + 1)){
            strcpy(buffer, value.c_str());
            this->image = nsvgParse(buffer, SVGImage::SVG_UNIT, 96);
            free(buffer);
        }
    }

    static View* create(){
        return new SVGImage();
    }


private:
    bool textureMode = true;
    NVGcolor backgroundColor = nvgRGBA(255,255,255, 0);
    NVGcolor color = nvgRGBA(0,0,0, 0);
    struct NSVGimage* image = nullptr;
};