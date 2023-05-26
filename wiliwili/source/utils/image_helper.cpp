//
// Created by fang on 2022/7/16.
//

#include "utils/image_helper.hpp"
#include "borealis/core/singleton.hpp"
#include "borealis/core/cache_helper.hpp"
#include "utils/thread_helper.hpp"
#include "borealis/core/thread.hpp"
#include "stb_image.h"
#include <fmt/format.h>

#ifdef USE_WEBP
#include <webp/decode.h>
#endif

class ImageThreadPool : public cpr::ThreadPool,
                        public brls::Singleton<ImageThreadPool> {
public:
    ImageThreadPool()
        : cpr::ThreadPool(1, ImageHelper::REQUEST_THREADS,
                          std::chrono::milliseconds(5000)) {
        brls::Logger::info("max_thread_num: {}", this->max_thread_num);
        this->Start();
    }

    ~ImageThreadPool() override { this->Stop(); }
};

ImageHelper::ImageHelper() : imageView(nullptr) {
    brls::Logger::verbose("new ImageHelper {}", fmt::ptr(this));
}

ImageHelper::~ImageHelper() {
    brls::Logger::verbose("delete ImageHelper {}", fmt::ptr(this));
}

void ImageHelper::with(brls::Image* view, const std::string& url) {
    // 检查是否存在缓存
    int tex = brls::TextureCache::instance().getCache(url);
    if (tex > 0) {
        brls::Logger::verbose("cache hit: {}", url);
        view->innerSetImage(tex);
        return;
    }

    Ref item;
    if (requestPool.empty()) {
        item = std::make_shared<ImageHelper>();
    } else {
        item = requestPool.front();
        requestPool.pop_front();
    }

    auto it = requestMap.insert(std::make_pair(view, item));
    if (!it.second) {
        //ToDo 上一个 brls::Image* 请求仍未结束
        brls::Logger::warning("with view: {} {}", fmt::ptr(view),
                              fmt::ptr(item));
        return;
    }

    item->imageUrl = url;
    item->isCancel.store(false);
    item->setImageView(view);

    //todo: 可能会发生同时请求多个重复链接的情况，此种情况下最好合并为一个请求
    brls::Logger::verbose("request Image 1: {} {} {}", url, fmt::ptr(view),
                          fmt::ptr(item));
    ImageThreadPool::instance().Submit(
        std::bind(&ImageHelper::requestImage, item));
}

void ImageHelper::requestImage() {
    if (this->isCancel.load()) {
        brls::sync(std::bind(&ImageHelper::clean, this->imageView));
        return;
    }
    brls::Logger::verbose("request Image 2: {} {}", this->imageUrl,
                          this->isCancel.load());

    // 请求图片
    cpr::Response r = cpr::Get(
#ifndef VERIFY_SSL
        cpr::VerifySsl{false},
#endif
        cpr::Url{this->imageUrl}, cpr::ProgressCallback([this](...) -> bool {
            return !this->isCancel.load();
        }));

    // 图片请求失败或取消请求
    if (r.status_code != 200 || r.downloaded_bytes == 0 || this->isCancel) {
        brls::Logger::verbose("request undone: {} {} {} {}", r.status_code,
                              r.downloaded_bytes, this->isCancel.load(),
                              r.url.str());

        brls::sync(std::bind(&ImageHelper::clean, this->imageView));
        return;
    }

    brls::Logger::verbose("load pic:{} size:{} bytes by{} to {} {}",
                          r.url.str(), r.downloaded_bytes, fmt::ptr(this),
                          fmt::ptr(this->imageView),
                          this->imageView->describe());

    uint8_t* imageData = nullptr;
    int imageW = 0, imageH = 0;

#ifdef USE_WEBP
    if (imageUrl.size() > 5 &&
        imageUrl.substr(imageUrl.size() - 5, 5) == ".webp") {
        imageData =
            WebPDecodeRGBA((const uint8_t*)r.text.c_str(),
                           (size_t)r.downloaded_bytes, &imageW, &imageH);
    } else {
#endif
        int n;
        imageData = stbi_load_from_memory((unsigned char*)r.text.c_str(),
                                          (int)r.downloaded_bytes, &imageW,
                                          &imageH, &n, 4);
#ifdef USE_WEBP
    }
#endif

    brls::sync([this, imageData, imageW, imageH]() {
        if (!this->isCancel.load()) {
            // brls::Logger::info("sync ImageHelper {} view {}", fmt::ptr(this), fmt::ptr(this->imageView));
            // 再检查一遍缓存
            int tex = brls::TextureCache::instance().getCache(this->imageUrl);
            if (tex == 0) {
                if (imageData) {
                    NVGcontext* vg = brls::Application::getNVGContext();
                    tex = nvgCreateImageRGBA(vg, imageW, imageH, 0, imageData);
                } else {
                    brls::Logger::error("Failed to load image: {}",
                                        this->imageUrl);
                }
            }
            if (tex > 0) {
                brls::TextureCache::instance().addCache(this->imageUrl, tex);
                this->imageView->innerSetImage(tex);
            }
            clean(this->imageView);
        }
        if (imageData) {
#ifdef USE_WEBP
            WebPFree(imageData);
#else
            stbi_image_free(imageData);
#endif
        }
    });
}

void ImageHelper::clean(brls::Image* view) {
    auto it = requestMap.find(view);
    if (it == requestMap.end()) return;

    it->second->imageView->ptrUnlock();
    it->second->imageView = nullptr;
    // 请求没结束，取消请求
    it->second->isCancel.store(true);
    // 归还到对象池
    requestPool.push_back(it->second);
    // 从请求池中移除
    requestMap.erase(it);
}

void ImageHelper::clear(brls::Image* view) {
    brls::TextureCache::instance().removeCache(view->getTexture());
    view->clear();

    clean(view);
}

void ImageHelper::setImageView(brls::Image* view) {
    // 禁止图片组件销毁
    view->ptrLock();
    // 设置图片组件不处理纹理的销毁，由缓存统一管理纹理销毁
    view->setFreeTexture(false);

    this->imageView = view;
}

void ImageHelper::setRequestThreads(size_t num) {
    REQUEST_THREADS                            = num;
    ImageThreadPool::instance().min_thread_num = 1;
    ImageThreadPool::instance().max_thread_num = num;
}
