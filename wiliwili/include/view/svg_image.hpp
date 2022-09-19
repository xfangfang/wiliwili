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
    SVGImage();

    void setImageFromSVGRes(std::string name);

    void setImageFromSVGFile(const std::string value);

    void setImageFromSVGString(const std::string value);

    void updateBitmap();

    //todo: 窗口大小调整后 图像模糊
    //    void invalidateImageBounds(){
    //        this->updateBitmap();
    //        Image::invalidateImageBounds();
    //    }

    static View* create();

private:
    std::unique_ptr<lunasvg::Document> document = nullptr;
};