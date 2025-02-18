//
// Created by fang on 2022/7/16.
//

#include <borealis/core/singleton.hpp>
#include <borealis/core/application.hpp>
#include <borealis/core/cache_helper.hpp>
#include <borealis/core/thread.hpp>
#include <stb_image.h>

#include "utils/image_helper.hpp"
#include "utils/string_helper.hpp"
#include "api/bilibili/util/http.hpp"

#ifdef USE_WEBP
#include <webp/decode.h>
#endif

#ifdef BOREALIS_USE_GXM
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#define STB_DXT_IMPLEMENTATION
#include <borealis/extern/nanovg/stb_dxt.h>
#include <borealis/extern/nanovg/nanovg_gxm.h>

static inline __attribute__((always_inline)) uint32_t nearest_po2(uint32_t val) {
    val--;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val++;

    return val;
}

static inline __attribute__((always_inline)) uint64_t morton_1(uint64_t x) {
    x = x & 0x5555555555555555;
    x = (x | (x >> 1)) & 0x3333333333333333;
    x = (x | (x >> 2)) & 0x0F0F0F0F0F0F0F0F;
    x = (x | (x >> 4)) & 0x00FF00FF00FF00FF;
    x = (x | (x >> 8)) & 0x0000FFFF0000FFFF;
    x = (x | (x >> 16)) & 0xFFFFFFFFFFFFFFFF;
    return x;
}

static inline __attribute__((always_inline)) void d2xy_morton(uint64_t d, uint64_t *x, uint64_t *y) {
    *x = morton_1(d);
    *y = morton_1(d >> 1);
}

static inline __attribute__((always_inline)) void extract_block(const uint8_t *src, uint32_t width, uint8_t *block) {
    for (int j = 0; j < 4; j++) {
        memcpy(&block[j * 4 * 4], src, 16);
        src += width * 4;
    }
}

/**
 * Compress RGBA data to DXT1 or DXT5
 * @param dst DXT data
 * @param src RGBA data
 * @param w source width
 * @param h source height
 * @param stride source stride
 * @param last_size max block size in pixel of last compression round, default is 64
 * @param isdxt5 false for DXT1, true for DXT5
 */
static void dxt_compress_ext(uint8_t *dst, uint8_t *src, uint32_t w, uint32_t h, uint32_t stride, uint32_t last_size, bool isdxt5) {
    uint8_t block[64];
    uint32_t align_w = MAX(nearest_po2(w), last_size);
    uint32_t align_h = MAX(nearest_po2(h), last_size);
    uint32_t s = MIN(align_w, align_h);
    uint32_t num_blocks = s * s / 16;
    const uint32_t block_size = isdxt5 ? 16 : 8;
    uint64_t d, offs_x, offs_y;

    for (d = 0; d < num_blocks; d++, dst += block_size) {
        d2xy_morton(d, &offs_x, &offs_y);
        if (offs_x * 4 >= h || offs_y * 4 >= w)
            continue;
        extract_block(src + offs_y * 16 + offs_x * stride * 16, stride, block);
        stb_compress_dxt_block(dst, block, isdxt5, STB_DXT_NORMAL);
    }
    if (align_w > align_h)
        return dxt_compress_ext(dst, src + s * 4, w - s, h, stride, s, isdxt5);
    if (align_w < align_h)
        return dxt_compress_ext(dst, src + stride * s * 4, w, h - s, stride, s, isdxt5);
}

static void dxt_compress(uint8_t *dst, uint8_t *src, uint32_t w, uint32_t h, bool isdxt5) {
    dxt_compress_ext(dst, src, w, h, w, 64, isdxt5);
}
#endif

class ImageThreadPool : public cpr::ThreadPool, public brls::Singleton<ImageThreadPool> {
public:
    ImageThreadPool() : cpr::ThreadPool(1, ImageHelper::REQUEST_THREADS, std::chrono::milliseconds(5000)) {
        brls::Logger::info("max_thread_num: {}", this->max_thread_num);
        this->Start();
    }

    ~ImageThreadPool() override { this->Stop(); }

    CURLSH* getShare() {
        return share.getShare();
    }

private:
    bilibili::CurlSharedObject share;
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

void ImageHelper::load(const std::string &url) {
    this->imageUrl = bilibili::HTTP::VERIFY.verify ? url :pystring::replace(url, "https", "http", 1);

#ifdef BOREALIS_USE_GXM
    std::vector<std::string> urls = pystring::rsplit(this->imageUrl, "@", 1);
    if (pystring::endswith(urls[0], "jpg")) {
        this->imageFlag = NVG_IMAGE_DXT1;
    } else {
        this->imageFlag = NVG_IMAGE_DXT5;
    }
#endif

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

static inline void freeImageData(uint8_t* imageData, bool isWebp) {
#ifdef USE_WEBP
    if (isWebp)
        WebPFree(imageData);
    else
#endif
        stbi_image_free(imageData);
}

void ImageHelper::requestImage() {
    brls::Logger::verbose("request Image 2: {} {}", this->imageUrl, this->isCancel);

    // 请求图片
    cpr::Session session;
    CURL* curl = session.GetCurlHolder()->handle;
    curl_easy_setopt(curl, CURLOPT_SHARE, ImageThreadPool::instance().getShare());
    curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, bilibili::HTTP::DNS_CACHE_TIMEOUT);
    session.SetTimeout(cpr::Timeout{bilibili::HTTP::TIMEOUT});
    session.SetConnectTimeout(cpr::ConnectTimeout{bilibili::HTTP::CONNECTION_TIMEOUT});
    session.SetVerifySsl(bilibili::HTTP::VERIFY);
    session.SetProxies(bilibili::HTTP::PROXIES);
    session.SetUrl(cpr::Url{this->imageUrl});
    session.SetProgressCallback(cpr::ProgressCallback([this](...) -> bool { return !this->isCancel; }));
    cpr::Response r = session.Get();

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

#ifdef BOREALIS_USE_GXM
    bool dxt5 = this->imageFlag & NVG_IMAGE_DXT5;
    size_t size = nearest_po2(imageW) * nearest_po2(imageH);
    if (!dxt5)
        size >> 1;
    auto *compressed = (uint8_t *)malloc(size);
    dxt_compress(compressed, imageData, imageW, imageH, dxt5);
    freeImageData(imageData, isWebp);
    imageData = compressed;
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
#ifdef BOREALIS_USE_GXM
                bool dxt5 = this->imageFlag & NVG_IMAGE_DXT5;
                tex = nvgCreateImageRGBA(vg, imageW, imageH, (dxt5 ? NVG_IMAGE_DXT5 : NVG_IMAGE_DXT1) | NVG_IMAGE_LPDDR, imageData);
#else
                tex = nvgCreateImageRGBA(vg, imageW, imageH, 0, imageData);
#endif
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
#ifdef BOREALIS_USE_GXM
            free(imageData);
#else
            freeImageData(imageData, isWebp);
#endif
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

std::string ImageHelper::parseGifImageUrl(const std::string& url, const std::string& ext) {
#ifdef USE_WEBP
    std::string image_url = url;
    if (pystring::endswith(url, "gif")) {
        // gif 图片暂时按照 jpg 来解析
        image_url += pystring::replace(ext, ".webp", ".jpg");
    } else {
        image_url += ext;
    }
    return image_url;
#else
    return url + ext;
#endif
}

std::string ImageHelper::parseNoteImageUrl(const std::string& url, size_t w, size_t h) {
    return parseGifImageUrl(url, wiliwili::format(note_custom_ext, w, h));
}