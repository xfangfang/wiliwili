//
// Created by fang on 2022/6/5.
//

#pragma once
#include <cstring>
#include <borealis.hpp>
#include <cpr/cpr.h>
#include <qrcodegen.hpp>
#include <lunasvg.h>

class SVGImage : public brls::Image {
public:
    SVGImage();

    ~SVGImage();

    void setImageFromSVGRes(std::string name);

    void setImageFromSVGFile(const std::string value);

    void setImageFromSVGString(const std::string value);

    void updateBitmap();

    static View *create();

private:
    std::unique_ptr<lunasvg::Document> document = nullptr;
    brls::VoidEvent::Subscription subscription;
    std::string filePath = "";
};