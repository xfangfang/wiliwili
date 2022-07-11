//
// Created by fang on 2022/5/3.
//

#pragma once

#include <borealis.hpp>
#include <cpr/cpr.h>

class NetImage : public brls::Image {

public:
    NetImage(){
        this->registerFilePathXMLAttribute("net_image", [this](std::string value) {
            this->setImageFromNet(value);
        });
    }

    ~NetImage()
    {
        brls::Logger::debug("~NetImage {}", (size_t)this);
    }

    void willDisappear(bool resetState = false)
    {
        // Nothing to do
        isWillDisappear = true;
        this->abort();
    }

    void willAppear(bool resetState = false)
    {
        isWillDisappear = false;
    }

    void abort(){

    }
    // todo: 自动缓存

    void onLayout(){
        brls::Image::onLayout();
        if (this->texture == 0)
            return;
        brls::Logger::error("net image on layout: {}/{}", this->getWidth(), this->getHeight());
    }

    void setImageFromNet(const std::string& url){
        // todo: allow cancel request
        this->ptrLock();
        cpr::GetCallback([this](cpr::Response r) {
//            if (isWillDisappear) return;
            if (r.downloaded_bytes == 0){
                brls::Logger::debug("undone pic:{}", r.url.str());
            } else {
                brls::Logger::debug("load pic:{} size:{}bytes to {}", r.url.str(), r.downloaded_bytes, (size_t)this);
                brls::Threading::sync([this, r](){
                    this->setImageFromMem((unsigned char *) r.text.c_str(),(int) r.downloaded_bytes);
                    this->ptrUnlock();
                });
            }
        }, cpr::Url{url});
    }

    static View* create(){
        return new NetImage();
    }

private:
    bool isWillDisappear = false;

};