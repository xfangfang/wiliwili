//
// Created by fang on 2022/6/4.
//

#pragma once
#include <borealis.hpp>
#include <cpr/cpr.h>
#include <QrCode.hpp>
#include <cstring>

#include <lunasvg.h>

#include "view/svg_image.hpp"

using qrcodegen::QrCode;
using qrcodegen::QrSegment;
using namespace lunasvg;


class QRImage : public brls::Image {

public:
    QRImage(){
        this->registerStringXMLAttribute("qr_content", [this](std::string value) {
            this->setImageFromQRContent(value);
        });

//        this->setFillColor(nvgRGB(0, 0, 0));
//        this->setBackgroundColor(nvgRGB(255, 255, 255));

    }

//    void draw(NVGcontext* vg, float x, float y, float width, float height, Style style, FrameContext* ctx){
//        SVGImage::draw(vg, x, y, width, height, style, ctx);
//    }

    void setImageFromQRContent(const std::string value){
        this->qr = QrCode::encodeText(value.c_str(), QrCode::Ecc::LOW);
        document = Document::loadFromData(this->qr.toSvgString(1));
        this->updateBitmap();
    }

    void updateBitmap(){
        auto bitmap = document->renderToBitmap(this->getWidth() * brls::Application::windowScale,
                                               this->getHeight() * brls::Application::windowScale);
        bitmap.convertToRGBA();
        NVGcontext* vg = Application::getNVGContext();
        this->innerSetImate(nvgCreateImageRGBA(vg,
                                               bitmap.width(), bitmap.height(),
                                               0, bitmap.data()));
    }

    void invalidateImageBounds(){
//        updateBitmap();
        Image::invalidateImageBounds();
    }

    static View* create(){
        return new QRImage();
    }


private:
    QrCode qr = QrCode::encodeText("", QrCode::Ecc::LOW);
    std::unique_ptr<Document> document;
};