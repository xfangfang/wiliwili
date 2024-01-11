//
// Created by fang on 2022/7/16.
//

#include <borealis/core/singleton.hpp>
#include <borealis/core/application.hpp>
#include <borealis/core/cache_helper.hpp>
#include <borealis/core/thread.hpp>
#include <stb_image.h>

#include "utils/image_helper.hpp"
#include "api/bilibili/util/http.hpp"

#ifdef USE_WEBP
#include <webp/decode.h>
#endif

class ImageThreadPool : public cpr::ThreadPool, public brls::Singleton<ImageThreadPool> {
public:
    ImageThreadPool() : cpr::ThreadPool(1, ImageHelper::REQUEST_THREADS, std::chrono::milliseconds(5000)) {
        brls::Logger::info("max_thread_num: {}", this->max_thread_num);
        this->Start();
    }

    ~ImageThreadPool() override { this->Stop(); }
};

ImageHelper::ImageHelper(brls::Image* view) : imageView(view) {}

ImageHelper::~ImageHelper() { brls::Logger::verbose("delete ImageHelper {}", (size_t)this); }

std::shared_ptr<ImageHelper> ImageHelper::with(brls::Image* view) {
    std::lock_guard<std::mutex> lock(requestMutex);
    std::shared_ptr<ImageHelper> item;

    if (!requestPool.empty() && (*requestPool.begin())->getImageView() == nullptr) {
        // 复用请求，挪到队尾
        item = *requestPool.begin();
        item->setImageView(view);
        requestPool.splice(requestPool.end(), requestPool, requestPool.begin());
    } else {
        // 新建 ImageHelper 实例
        item = std::make_shared<ImageHelper>(view);
        requestPool.emplace_back(item);
    }

    auto iter         = --requestPool.end();
    requestMap[view]  = iter;
    item->currentIter = iter;
    // 重置 "取消" 标记位
    item->isCancel = false;
    // 禁止图片组件销毁
    item->imageView->ptrLock();
    // 设置图片组件不处理纹理的销毁，由缓存统一管理纹理销毁
    item->imageView->setFreeTexture(false);

    brls::Logger::verbose("with view: {} {} {}", (size_t)view, (size_t)item.get(), (size_t)(*iter).get());

    return item;
}

void ImageHelper::load(std::string url) {
    this->imageUrl = url;

    brls::Logger::verbose("load view: {} {}", (size_t)this->imageView, (size_t)this);

    //    std::unique_lock<std::mutex> lock(this->loadingMutex);

    // 检查是否存在缓存
    int tex = brls::TextureCache::instance().getCache(this->imageUrl);
    if (tex > 0) {
        brls::Logger::verbose("cache hit: {}", this->imageUrl);
        this->imageView->innerSetImage(tex);
        this->clean();
        return;
    }

    //todo: 可能会发生同时请求多个重复链接的情况，此种情况下最好合并为一个请求

    // 缓存网络图片
    brls::Logger::verbose("request Image 1: {} {}", this->imageUrl, this->isCancel);
    ImageThreadPool::instance().Submit([this]() {
        brls::Logger::verbose("Submit view: {} {} {} {}", (size_t)this->imageView, (size_t)this, this->imageUrl,
                              this->isCancel);
        if (this->isCancel) {
            this->clean();
            return;
        }
        this->requestImage();
    });
}

void ImageHelper::requestImage() {
    brls::Logger::verbose("request Image 2: {} {}", this->imageUrl, this->isCancel);

    // 请求图片
    cpr::Response r = cpr::Get(bilibili::HTTP::VERIFY, bilibili::HTTP::PROXIES, cpr::Url{this->imageUrl},
                               cpr::ProgressCallback([this](...) -> bool { return !this->isCancel; }));

    // 图片请求失败或取消请求
    if (r.status_code != 200 || r.downloaded_bytes == 0 || this->isCancel) {
        brls::Logger::verbose("request undone: {} {} {} {}", r.status_code, r.downloaded_bytes, this->isCancel,
                              r.url.str());

        this->clean();
        return;
    }

    brls::Logger::verbose("load pic:{} size:{} bytes by{} to {} {}", r.url.str(), r.downloaded_bytes, (size_t)this,
                          (size_t)this->imageView, this->imageView->describe());

    uint8_t* imageData = nullptr;
    int imageW = 0, imageH = 0;
    bool isWebp = false;

#ifdef USE_WEBP
    if (imageUrl.size() > 5 && imageUrl.substr(imageUrl.size() - 5, 5) == ".webp") {
        imageData = WebPDecodeRGBA((const uint8_t*)r.text.c_str(), (size_t)r.downloaded_bytes, &imageW, &imageH);
        isWebp    = true;
    } else {
#endif
        int n;
        imageData =
            stbi_load_from_memory((unsigned char*)r.text.c_str(), (int)r.downloaded_bytes, &imageW, &imageH, &n, 4);
#ifdef USE_WEBP
    }
#endif

    brls::sync([this, r, imageData, imageW, imageH, isWebp]() {
        // 再检查一遍缓存
        int tex = brls::TextureCache::instance().getCache(this->imageUrl);
        if (tex > 0) {
            brls::Logger::verbose("cache hit 2: {}", this->imageUrl);
            this->imageView->innerSetImage(tex);
        } else {
            NVGcontext* vg = brls::Application::getNVGContext();
            if (imageData) {
                tex = nvgCreateImageRGBA(vg, imageW, imageH, 0, imageData);
            } else {
                brls::Logger::error("Failed to load image: {}", this->imageUrl);
            }

            if (tex > 0) {
                brls::TextureCache::instance().addCache(this->imageUrl, tex);
                if (!this->isCancel) {
                    brls::Logger::verbose("load image: {}", this->imageUrl);
                    this->imageView->innerSetImage(tex);
                }
            }
        }
        if (imageData) {
#ifdef USE_WEBP
            if (isWebp)
                WebPFree(imageData);
            else
#endif
                stbi_image_free(imageData);
        }
        this->clean();
    });
}

void ImageHelper::clean() {
    std::lock_guard<std::mutex> lock(requestMutex);

    // 允许图片组件销毁
    if (this->imageView) this->imageView->ptrUnlock();
    // 移除请求，复用 ImageHelper (挪到队首)
    requestPool.splice(requestPool.begin(), requestPool, this->currentIter);
    this->imageView   = nullptr;
    this->currentIter = requestPool.end();
}

void ImageHelper::clear(brls::Image* view) {
    brls::TextureCache::instance().removeCache(view->getTexture());
    view->clear();

    std::lock_guard<std::mutex> lock(requestMutex);

    // 请求不存在
    if (requestMap.find(view) == requestMap.end()) return;

    brls::Logger::verbose("clear view: {} {}", (size_t)view, (size_t)(*requestMap[view]).get());

    // 请求没结束，取消请求
    if ((*requestMap[view])->imageView == view) {
        (*requestMap[view])->cancel();
    }
    requestMap.erase(view);
}

void ImageHelper::cancel() {
    brls::Logger::verbose("Cancel request: {}", this->imageUrl);
    this->isCancel = true;
}

void ImageHelper::setRequestThreads(size_t num) {
    REQUEST_THREADS                            = num;
    ImageThreadPool::instance().min_thread_num = 1;
    ImageThreadPool::instance().max_thread_num = num;
}

void ImageHelper::setImageView(brls::Image* view) { this->imageView = view; }

brls::Image* ImageHelper::getImageView() { return this->imageView; }
