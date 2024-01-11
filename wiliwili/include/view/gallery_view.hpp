//
// Created by fang on 2022/8/21.
//

// register this view in main.cpp
//#include "view/gallery_view.hpp"
//    brls::Application::registerXMLView("GalleryView", GalleryView::create);

#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>

namespace brls {
class Image;
class Label;
}  // namespace brls

typedef std::pair<std::string, std::string> GalleryItemData;
typedef std::vector<GalleryItemData> GalleryData;

enum class GalleryAnimation { ENTER_LEFT, ENTER_RIGHT, EXIT_LEFT, EXIT_RIGHT };

class GalleryItem : public brls::Box {
public:
    GalleryItem();

    ~GalleryItem() override = default;

    void animate(GalleryAnimation animation);

    void startScrolling(float newScroll);

private:
    brls::Animatable contentOffsetX = 0.0f;
};

class ImageGalleryItem : public GalleryItem {
public:
    ImageGalleryItem();

    virtual void setData(GalleryItemData value);

private:
    BRLS_BIND(brls::Image, image, "gallery/image");
    BRLS_BIND(brls::Label, label, "gallery/label");
    GalleryItemData data;
};

class GalleryView : public brls::Box {
public:
    GalleryView();

    ~GalleryView() override;

    static View *create();

    void setData(GalleryData value);

    void addCustomView(GalleryItem *view);

    void prev();

    void next();

    void setIndicatorPosition(float height);

    void draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext *ctx) override;

private:
    GalleryData data;
    unsigned int index      = 0;
    float indicatorPosition = 0.98;
};