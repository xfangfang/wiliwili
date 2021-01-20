#pragma once

#include <borealis.hpp>
#include <bilibili.h>
#include <fstream>

class NetImage : public brls::Image {

    public:
        NetImage(std::string path){
            client = new bilibili::BilibiliClient();
            this->setImage(path);
            this->setOpacity(1.0F);
        }

        void setImage(std::string path){
            if(path.compare(0,4,"http") == 0){
                this->url = path;
                client->download(this->url, [this](unsigned char * buffer, size_t bufferSize){
                    if (this->tempBuffer != nullptr)
                        delete[] this->tempBuffer;
                    this->tempBuffer = new unsigned char[(size_t)bufferSize];
                    std::memcpy(this->tempBuffer, buffer, bufferSize);
                    this->tempBufferSize = bufferSize;
                });
                this->setImage(BOREALIS_ASSET("icon/bilibili_128x128.jpg"));
                this->setOpacity(1.0F);
                this->tempBuffer = nullptr;
            } else {
                Image::setImage(path);
            }
        }

        void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
        {
            if(this->tempBuffer){
                Image::setImage(this->tempBuffer, this->tempBufferSize);
                delete[] this->tempBuffer;
                this->tempBuffer = nullptr;
            }
            Image::draw(vg,x,y,width,height,style,ctx);
        }

    private:
        std::string url;
        bilibili::BilibiliClient *client;
        unsigned char* tempBuffer;
        size_t tempBufferSize;

};