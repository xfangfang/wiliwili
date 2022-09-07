//
// Created by fang on 2022/7/16.
//

#pragma once

#include <borealis.hpp>
#include <cpr/cpr.h>
#include <ctime>
#include <random>

class ImageCache {
public:
    ImageCache(std::string d, size_t l) : data(d), length(l) {}

    std::string data;
    size_t length = 0;

    int requestCache(const std::string& url);
};

class ImageHelper {
public:
    static std::vector<std::shared_ptr<ImageHelper>> imagePool;
    static std::default_random_engine random_engine;

    static void init();

    static void clean();

    static std::shared_ptr<ImageHelper> with(brls::View* view);

    ImageHelper(brls::View* view);

    ~ImageHelper();

    void setCurrentView(brls::View* view);

    void cancel();

    bool isAvailable();

    ImageHelper* load(std::string url);

    ImageHelper* into(brls::Image* image);

    /// 清空图片内容
    static void clear(brls::Image* view);

    brls::Image* getImageView();

private:
    bool isCancel  = false;
    bool available = true;
    brls::View* currentView;
    brls::Image* imageView;
    std::string imageUrl;
};