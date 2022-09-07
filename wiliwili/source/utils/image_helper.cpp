//
// Created by fang on 2022/7/16.
//

#include "utils/image_helper.hpp"
#include "utils/singleton.hpp"
#include "borealis/core/thread.hpp"

std::vector<std::shared_ptr<ImageHelper>> ImageHelper::imagePool;
std::default_random_engine ImageHelper::random_engine;

template <typename K, typename T>
struct Node {
    K key;
    T value;
    Node(K k, T v) : key(k), value(v) {}
};
template <typename K, typename T>
class LRUCache {
public:
    LRUCache(int c, T defaultValue) : capacity(c), defaultValue(defaultValue) {}

    T get(K key) {
        if (cacheMap.find(key) == cacheMap.end()) {
            return defaultValue;
        }
        cacheList.splice(cacheList.begin(), cacheList, cacheMap[key]);
        cacheMap[key] = cacheList.begin();
        return cacheMap[key]->value;
    }
    void set(K key, T value) {
        if (cacheMap.find(key) == cacheMap.end()) {
            if (cacheList.size() == capacity) {
                cacheMap.erase(cacheList.back().key);
                cacheList.pop_back();
            }
            cacheList.push_front(Node<K, T>(key, value));
            cacheMap[key] = cacheList.begin();
        } else {
            cacheMap[key]->value = value;
            cacheList.splice(cacheList.begin(), cacheList, cacheMap[key]);
            cacheMap[key] = cacheList.begin();
        }
    }

    std::list<Node<K, T>>& getCacheList() { return cacheList; }

private:
    int capacity;
    T defaultValue;
    std::list<Node<K, T>> cacheList;
    std::unordered_map<K, typename std::list<Node<K, T>>::iterator> cacheMap;
};

class TextureCache : public Singleton<TextureCache> {
public:
    int getCache(const std::string& url) { return cache.get(url); }

    void addCache(const std::string& url, int texture) {
        if (texture <= 0) return;

        cache.set(url, texture);
    }

    void clean() {
        auto vg = brls::Application::getNVGContext();
        for (auto& i : cache.getCacheList()) {
            nvgDeleteImage(vg, i.value);
        }
    }

    LRUCache<std::string, int> cache = LRUCache<std::string, int>(200, 0);
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

void ImageHelper::cancel() { this->isCancel = true; }

bool ImageHelper::isAvailable() { return this->available; }

ImageHelper* ImageHelper::load(std::string url) {
    this->imageUrl = url;
    return this;
}

ImageHelper* ImageHelper::into(brls::Image* image) {
    if (!this->available) brls::fatal("Not available");
    this->imageView = image;
    this->available = false;
    this->isCancel  = false;

    image->setFreeTexture(false);

    int tex = TextureCache::instance().getCache(this->imageUrl);
    if (tex > 0) {
        image->innerSetImage(tex);
        this->available = true;
        return this;
    }

    image->ptrLock();
    cpr::GetCallback(
        [this, image](cpr::Response r) {
            brls::Logger::debug("net image status code: {} / {}", r.status_code,
                                r.downloaded_bytes);
            if (r.status_code != 200 || r.downloaded_bytes == 0 ||
                this->isCancel) {
                brls::Logger::debug("undone pic:{}", r.url.str());
                this->available = true;
                image->ptrUnlock();
            } else {
                brls::Logger::debug("load pic:{} size:{}bytes to {}",
                                    r.url.str(), r.downloaded_bytes,
                                    (size_t)this);
                brls::Logger::debug("load pic to image1 {} {}", (size_t)image,
                                    image->describe());
                std::uniform_real_distribution<double> u(10, 100);
                brls::Threading::delay(u(random_engine), [this, image, r]() {
                    image->setImageFromMem((unsigned char*)r.text.c_str(),
                                           (size_t)r.downloaded_bytes);
                    if (image->getTexture() > 0)
                        TextureCache::instance().addCache(this->imageUrl,
                                                          image->getTexture());
                    this->available = true;
                    image->ptrUnlock();
                });
            }
        },
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
    return this;
}

/// 清空图片内容
void ImageHelper::clear(brls::Image* view) {
    view->clear(false);
    for (auto i : imagePool) {
        if (i->getImageView() == view && !i->isAvailable()) {
            // 图片正在加载中
            brls::Logger::debug("clear image2: {}", (size_t)view);
            i->cancel();
        }
    }
}

brls::Image* ImageHelper::getImageView() { return this->imageView; }
