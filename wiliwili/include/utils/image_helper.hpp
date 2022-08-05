//
// Created by fang on 2022/7/16.
//

#pragma once

#include <borealis.hpp>
#include <cpr/cpr.h>

class ImageHelper{
public:
    static std::vector<std::shared_ptr<ImageHelper>> imagePool;

    static std::shared_ptr<ImageHelper> with(brls::View* view){
        for(auto i: imagePool){
            if(i->isAvailable()) {
                i->setCurrentView(view);
                return i;
            }
        }
        auto item = std::make_shared<ImageHelper>(view);
        imagePool.push_back(item);
        return item;
    }

    ImageHelper(brls::View* view):currentView(view){}

    ~ImageHelper(){
        brls::Logger::debug("del ImageHelper");
    }

    void setCurrentView(brls::View* view){
        if(!this->isAvailable())
            brls::fatal("ImageHelper is not available now");
        this->currentView = view;
    }

    void cancel(){
        this->isCancel = true;
    }

    bool isAvailable(){
        return this->available;
    }

    ImageHelper* load(std::string url){
        this->imageUrl = url;
        return this;
    }

    ImageHelper* into(brls::Image* image){
        if(!this->available)
            brls::fatal("Not available");
        this->imageView = image;
        this->available = false;
        this->isCancel = false;
        image->setImageAsync([this, image](std::function<void(const std::string& , size_t length)> cb){
            cpr::GetCallback([this, image, cb](cpr::Response r) {
                brls::Logger::error("net image status code: {} / {}", r.status_code, r.downloaded_bytes);
                if (r.status_code != 200 || r.downloaded_bytes == 0 || this->isCancel){
                    brls::Logger::debug("undone pic:{}", r.url.str());
                    this->available = true;
                } else {
                    brls::Logger::debug("load pic:{} size:{}bytes to {}", r.url.str(), r.downloaded_bytes, (size_t)this);
                    brls::Logger::debug("load pic to image1 {}", (size_t)image);
                    cb(r.text, (size_t)r.downloaded_bytes);
                    this->available = true;
                }
            }, cpr::Url{this->imageUrl},cpr::ProgressCallback([this](cpr::cpr_off_t downloadTotal, cpr::cpr_off_t downloadNow,
                                                                     cpr::cpr_off_t uploadTotal, cpr::cpr_off_t uploadNow, intptr_t userdata) -> bool {
                if(this->isCancel){
                    return false;
                }
                return true;
            }));
        });
        return this;
    }

    /// 清空图片内容
    static void clear(brls::Image* view){
        brls::Threading::sync([view](){
            view->clear();
        });
        for(auto i: imagePool){
            if(i->getImageView() == view && !i->isAvailable()){
                // 图片正在加载中
                brls::Logger::error("clear image2: {}", (size_t)view);
                i->cancel();
            }
        }
    }

    brls::Image* getImageView(){
        return this->imageView;
    }



private:
    bool isCancel = false;
    bool available = true;
    brls::View* currentView;
    brls::Image* imageView;
    std::string imageUrl;
};