//
// Created by fang on 2022/6/4.
//

#pragma once
#include <borealis.hpp>
#include <cpr/cpr.h>
#include <QrCode.hpp>
#include <pystring.h>
#include <fmt/format.h>

#include "view/svg_image.hpp"

using qrcodegen::QrCode;
using qrcodegen::QrSegment;
using namespace lunasvg;

class QRImage : public SVGImage {
public:
    QRImage() {
        this->registerStringXMLAttribute(
            "QRContent",
            [this](std::string value) { this->setImageFromQRContent(value); });

        this->registerColorXMLAttribute("QRBackground", [this](NVGcolor value) {
            this->setQRBackgroundColor(value);
        });

        this->registerColorXMLAttribute(
            "QRColor", [this](NVGcolor value) { this->setQRMainColor(value); });

        this->registerFloatXMLAttribute(
            "QRBorder", [this](float value) { this->setQRBorder(value); });
    }

    void setImageFromQRContent(const std::string value) {
        this->qr = QrCode::encodeText(value.c_str(), QrCode::Ecc::LOW);
        this->updateQR();
    }

    void setQRMainColor(NVGcolor c) {
        this->mainColor = c;
        this->updateQR();
    }

    void setQRBackgroundColor(NVGcolor c) {
        this->backgroundColor = c;
        this->updateQR();
    }

    void setQRBorder(int border) {
        this->QRBorder = border;
        this->updateQR();
    }

    void updateQR() {
        unsigned char r, g, b;
        std::string qr_svg = this->qr.toSvgString(this->QRBorder);

        if (mainColor.r != 0 || mainColor.g != 0 || mainColor.b != 0) {
            r = mainColor.r * 255;
            g = mainColor.g * 255;
            b = mainColor.b * 255;
            std::string new_main_color =
                fmt::format("#{:02X}{:02X}{:02X}", r, g, b);
            qr_svg = pystring::replace(qr_svg, "#000000", new_main_color);
        }

        if (backgroundColor.r != 1.0 || backgroundColor.g != 1.0 ||
            backgroundColor.b != 1.0) {
            r = backgroundColor.r * 255;
            g = backgroundColor.g * 255;
            b = backgroundColor.b * 255;
            std::string new_background_color =
                fmt::format("#{:02X}{:02X}{:02X}", r, g, b);
            qr_svg = pystring::replace(qr_svg, "#FFFFFF", new_background_color);
        }

        this->setImageFromSVGString(qr_svg);
    }

    static View* create() { return new QRImage(); }

private:
    QrCode qr                = QrCode::encodeText("", QrCode::Ecc::LOW);
    NVGcolor mainColor       = nvgRGB(0, 0, 0);
    NVGcolor backgroundColor = nvgRGB(255, 255, 255);
    int QRBorder             = 1;
};