//
// Created by fang on 2022/6/4.
//

#pragma once

#include <cpr/cpr.h>
#include <qrcodegen.hpp>
#include <pystring.h>
#include <cstdlib>
#include <fmt/format.h>

#include "view/svg_image.hpp"

using qrcodegen::QrCode;
using qrcodegen::QrSegment;
using namespace lunasvg;

class QRImage : public SVGImage {
public:
    QRImage() {
        this->registerStringXMLAttribute("QRContent",
                                         [this](const std::string& value) { this->setImageFromQRContent(value); });

        this->registerColorXMLAttribute("QRBackground", [this](NVGcolor value) { this->setQRBackgroundColor(value); });

        this->registerColorXMLAttribute("QRColor", [this](NVGcolor value) { this->setQRMainColor(value); });

        this->registerFloatXMLAttribute("QRBorder", [this](float value) { this->setQRBorder((int)value); });
    }

    void setImageFromQRContent(const std::string& value) {
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

    static inline std::string getColor(NVGcolor color) {
        unsigned char r, g, b;
        r = color.r * 255;
        g = color.g * 255;
        b = color.b * 255;
        return fmt::format("{:02X}{:02X}{:02X}", r, g, b);
    }

    void updateQR() { this->setImageFromSVGString(this->toSvgString()); }

    [[nodiscard]] std::string toSvgString() const {
        int size   = qr.getSize();
        int border = this->QRBorder;
        if (border < 0) throw std::domain_error("Border must be non-negative");
        if (border > INT_MAX / 2 || border * 2 > INT_MAX - size) throw std::overflow_error("Border too large");

        std::ostringstream sb;
        sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        sb << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
              "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
        sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" "
              "viewBox=\"0 0 ";
        sb << (size + border * 2) << " " << (size + border * 2) << "\" stroke=\"none\">\n";
        sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#";
        sb << getColor(this->backgroundColor);
        sb << "\"/>\n";
        sb << "\t<path d=\"";
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (qr.getModule(x, y)) {
                    if (x != 0 || y != 0) sb << " ";
                    sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
                }
            }
        }
        sb << "\" fill=\"#";
        sb << getColor(this->mainColor);
        sb << "\"/>\n";
        sb << "</svg>\n";
        return sb.str();
    }

    static View* create() { return new QRImage(); }

private:
    QrCode qr                = QrCode::encodeText("", QrCode::Ecc::LOW);
    NVGcolor mainColor       = nvgRGB(0, 0, 0);
    NVGcolor backgroundColor = nvgRGB(255, 255, 255);
    int QRBorder             = 1;
};