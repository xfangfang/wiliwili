//
// Created by fang on 2022/6/5.
//

#pragma once
#include <cstring>
#include <borealis.hpp>
#include <cpr/cpr.h>
#include <QrCode.hpp>
#include <lunasvg.h>

class SVGImage : public brls::Image {

public:
    SVGImage(){
        this->registerFilePathXMLAttribute("SVG", [this](std::string value) {
            this->setImageFromSVGFile(value);
        });
    }


    void setImageFromSVGRes(std::string name){
        this->setImageFromSVGFile(std::string(BRLS_RESOURCES) + name);
    }

    void setImageFromSVGFile(const std::string value){
        this->document = lunasvg::Document::loadFromFile(value.c_str());
        this->updateBitmap();
    }

    void setImageFromSVGString(const std::string value){
        this->document = lunasvg::Document::loadFromData(value);
        this->updateBitmap();
    }

    void updateBitmap(){
        float width = this->getWidth() * brls::Application::windowScale;
        float height = this->getHeight() * brls::Application::windowScale;
        auto bitmap = this->document->renderToBitmap(width, height);
        bitmap.convertToRGBA();
        NVGcontext* vg = brls::Application::getNVGContext();
        int tex = nvgCreateImageRGBA(vg,bitmap.width(), bitmap.height(), 0, bitmap.data());
        this->innerSetImage(tex);
    }

    //todo: 窗口大小调整后 图像模糊
//    void invalidateImageBounds(){
//        this->updateBitmap();
//        Image::invalidateImageBounds();
//    }

    static View* create(){
        return new SVGImage();
    }

private:
    std::unique_ptr<lunasvg::Document> document;
};