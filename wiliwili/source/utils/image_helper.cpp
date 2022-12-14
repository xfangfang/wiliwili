//
// Created by fang on 2022/7/16.
//

#include "utils/image_helper.hpp"
#include "utils/singleton.hpp"
#include "utils/cache_helper.hpp"
#include "utils/thread_helper.hpp"
#include "borealis/core/thread.hpp"

std::vector<std::shared_ptr<ImageHelper>> ImageHelper::imagePool;
std::default_random_engine ImageHelper::random_engine;

class ImageThreadPool : public cpr::ThreadPool,
                        public Singleton<ImageThreadPool> {
public:
    ImageThreadPool()
        : cpr::ThreadPool(THREAD_POOL_MIN_THREAD_NUM,
                          THREAD_POOL_MAX_THREAD_NUM,
                          std::chrono::milliseconds(5000)) {
        this->Start();
    }

    ~ImageThreadPool() { this->Stop(); }
};

void ImageHelper::init() {
    random_engine.seed(time(0));

    brls::Application::getExitEvent()->subscribe([]() {
        ImageHelper::clean();
        TextureCache::instance().clean();
    });
}

void ImageHelper::clean() {}

std::shared_ptr<ImageHelper> ImageHelper::with(brls::View* view) {
    for (auto i : imagePool) {
        if (i->isAvailable()) {
            i->setCurrentView(view);
            return i;
        }
    }
    auto item = std::make_shared<ImageHelper>(view);
    imagePool.push_back(item);
    return item;
}

ImageHelper::ImageHelper(brls::View* view) : currentView(view) {}

ImageHelper::~ImageHelper() { brls::Logger::debug("del ImageHelper"); }

void ImageHelper::setCurrentView(brls::View* view) {
    if (!this->isAvailable()) brls::fatal("ImageHelper is not available now");
    this->currentView = view;
}

void ImageHelper::cancel() {
    std::unique_lock<std::mutex> lock(this->availableMutex);
    // 已经加载结束、出错或被取消
    if (this->available) {
        if (this->imageView) this->imageView->clear();
        brls::Logger::verbose("Cancel loading pictures (done): {}",
                              (size_t)this->imageView);
        return;
    }
    this->isCancel = true;
    brls::Logger::verbose("Cancel loading pictures: {}",
                          (size_t)this->imageView);
}

bool ImageHelper::isAvailable() { return this->available; }

void ImageHelper::setAvailable(bool value) {
    this->available = value;
    if (value) {
        this->imageView = nullptr;
    } else {
        this->isCancel = false;
    }
}

ImageHelper* ImageHelper::load(std::string url) {
    this->imageUrl = url;
    return this;
}

ImageHelper* ImageHelper::into(brls::Image* image) {
    std::unique_lock<std::mutex> lock(this->availableMutex);
    if (!this->available) {
        brls::Logger::error("Image: {} is not available now", (size_t)image);
        return this;
    }

    this->imageView = image;
    image->setFreeTexture(false);
    this->setAvailable(false);

    // 禁止删除图片
    image->ptrLock();

    // 先检查缓存
    int tex = TextureCache::instance().getCache(this->imageUrl);
    if (tex > 0) {
        brls::Logger::verbose("cache hit: {}", this->imageUrl);
        image->innerSetImage(tex);
        this->setAvailable(true);
        image->ptrUnlock();
        return this;
    }

    //todo: 可能会发生同时请求多个重复链接的情况，此种情况下最好合并为一个请求

    ImageThreadPool::instance().Submit([this, image]() {
        cpr::Response r = cpr::Get(
#ifndef VERIFY_SSL
            cpr::VerifySsl{false},
#endif
            cpr::Url{this->imageUrl},
            cpr::ProgressCallback(
                [this](cpr::cpr_off_t downloadTotal, cpr::cpr_off_t downloadNow,
                       cpr::cpr_off_t uploadTotal, cpr::cpr_off_t uploadNow,
                       intptr_t userdata) -> bool {
                    if (this->isCancel) {
                        return false;
                    }
                    return true;
                }));
        brls::Logger::verbose("net image status code: {} / {}", r.status_code,
                              r.downloaded_bytes);
        std::unique_lock<std::mutex> lock(this->availableMutex);
        if (r.status_code != 200 || r.downloaded_bytes == 0 || this->isCancel) {
            brls::Logger::verbose("undone pic:{}", r.url.str());
            this->setAvailable(true);
            image->ptrUnlock();
        } else {
            brls::Logger::verbose("load pic:{} size:{}bytes by{} to {} {}",
                                  r.url.str(), r.downloaded_bytes, (size_t)this,
                                  (size_t)image, image->describe());
            brls::sync([this, image, r]() {
                std::unique_lock<std::mutex> lock(this->availableMutex);
                if (this->isCancel) {
                    this->setAvailable(true);
                    image->ptrUnlock();
                    return;
                }

                // 还需要再检查一遍缓存
                int tex = TextureCache::instance().getCache(this->imageUrl);
                if (tex > 0) {
                    brls::Logger::verbose("cache hit 2: {}", this->imageUrl);
                    image->innerSetImage(tex);
                } else {
                    image->setImageFromMem((unsigned char*)r.text.c_str(),
                                           (size_t)r.downloaded_bytes);
                    if (image->getTexture() > 0)
                        TextureCache::instance().addCache(this->imageUrl,
                                                          image->getTexture());
                }
                this->setAvailable(true);
                image->ptrUnlock();
            });
        }
    });
    return this;
}

/// 清空图片内容
void ImageHelper::clear(brls::Image* view) {
    TextureCache::instance().removeCache(view->getTexture());

    view->clear();
    for (auto i : imagePool) {
        if (i->getImageView() == view) {
            // 图片正在加载中
            brls::Logger::verbose("clear image2: {}", (size_t)view);
            i->cancel();
        }
    }
}

brls::Image* ImageHelper::getImageView() { return this->imageView; }
