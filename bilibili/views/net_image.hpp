#pragma once

#include <borealis.hpp>
#include <bilibili.h>
#include <fstream>

#include "utils/utils.hpp"

class NetImage : public brls::Image {

    public:
        NetImage(std::string path, std::string placeHolder = BOREALIS_ASSET("icon/bilibili_video.png")):placeHolder(placeHolder){
            Image::setImage(placeHolder);
            this->setImage(path);
            this->setOpacity(1.0F);
        }

        //the param path must be a http link
        void setImage(std::string path){
            if(path.compare(0,4,"http") == 0 || path.compare(0,2,"//") == 0 ){
                this->url = path.compare(0,2,"//") ? path : "http:" + path;
                NVGcontext* vg = brls::Application::getNVGContext();
                bilibili::BilibiliClient::download(this->url, [this, vg](std::string buffer, size_t bufferSize){
                    if (bufferSize == 0){
                        brls::Logger::debug("undone pic:{}/{}",this->url,bufferSize);
                        return;
                    }
                    if (this->imageBuffer != nullptr)
                                delete[] this->imageBuffer;
                    this->imageBuffer = new unsigned char[bufferSize];
                    std::memcpy(this->imageBuffer, buffer.c_str(), bufferSize);
                    this->imageBufferSize = bufferSize;

                    RunOnUIThread([this, vg](){
                        // this->Image::setImage((unsigned char*)buffer.c_str(), bufferSize);
                            if (this->texture != -1)
                                nvgDeleteImage(vg, this->texture);
                            if (this->imageBuffer != nullptr)
                                this->texture = nvgCreateImageMem(vg, 0, this->imageBuffer, this->imageBufferSize);
                            this->invalidate();
                    });
                });
            }
        }

        void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
        {
            Image::draw(vg,x,y,width,height,style,ctx);
        }

    private:
        std::string url;
        std::string placeHolder;

};