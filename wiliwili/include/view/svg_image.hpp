//
// Created by fang on 2022/6/5.
//

#pragma once
#include <borealis/views/image.hpp>
#include <lunasvg.h>

class SVGImage : public brls::Image {
public:
    SVGImage();

    ~SVGImage() override;

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

    void setImageFromSVGRes(const std::string& value);

    void setImageFromSVGFile(const std::string& value);

    void setImageFromSVGString(const std::string& value);

    void rotate(float value);

    void updateBitmap();

    static View* create();

private:
    std::unique_ptr<lunasvg::Document> document = nullptr;
    brls::VoidEvent::Subscription subscription;
    std::string filePath;
    float angle = 0;
};