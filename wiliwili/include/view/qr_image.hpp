//
// Created by fang on 2022/6/4.
//

#pragma once
#include <borealis.hpp>
#include <cpr/cpr.h>
#include <qrcodegen.hpp>
#include <pystring/pystring.h>
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
    static std::string toSvgString(const QrCode &qr, int border) {
        if (border < 0)
            throw std::domain_error("Border must be non-negative");
        if (border > INT_MAX / 2 || border * 2 > INT_MAX - qr.getSize())
            throw std::overflow_error("Border too large");

        std::ostringstream sb;
        sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        sb << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
        sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 ";
        sb << (qr.getSize() + border * 2) << " " << (qr.getSize() + border * 2) << "\" stroke=\"none\">\n";
        sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
        sb << "\t<path d=\"";
        for (int y = 0; y < qr.getSize(); y++) {
            for (int x = 0; x < qr.getSize(); x++) {
                if (qr.getModule(x, y)) {
                    if (x != 0 || y != 0)
                        sb << " ";
                    sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
                }
            }
        }
        sb << "\" fill=\"#000000\"/>\n";
        sb << "</svg>\n";
        return sb.str();
    }

    void updateQR() {
        unsigned char r, g, b;
        std::string qr_svg = QRImage::toSvgString(this->qr, this->QRBorder);

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