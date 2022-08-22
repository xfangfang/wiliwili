//
// Created by fang on 2022/8/21.
//

// register this view in main.cpp
//#include "view/gallery_view.hpp"
//    brls::Application::registerXMLView("GalleryView", GalleryView::create);

#pragma once

#include <borealis.hpp>

typedef std::pair<std::string, std::string> GalleryItemData;
typedef std::vector<GalleryItemData> GalleryData;

enum class GalleryAnimation
{
    ENTER_LEFT,
    ENTER_RIGHT,
    EXIT_LEFT,
    EXIT_RIGHT
};


class GalleryItem: public brls::Box {
public:
    GalleryItem();

    virtual ~GalleryItem() = default;

    static GalleryItem* create();

    virtual void setData(GalleryItemData value);

    void animate(GalleryAnimation animation);


    void startScrolling(float newScroll);

private:
    BRLS_BIND(brls::Image, image, "gallery/image");
    BRLS_BIND(brls::Label, label, "gallery/label");
    GalleryItemData data;
    brls::Animatable contentOffsetX = 0.0f;
};

class GalleryView : public brls::Box {

public:
    GalleryView();

    ~GalleryView();

    static View *create();

    void setData(GalleryData value);

    void prev();

    void next();

    void draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext *ctx) override;

private:
    GalleryData data;
    uint index = 0;
};