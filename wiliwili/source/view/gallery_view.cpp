//
// Created by fang on 2022/8/21.
//

#include "view/gallery_view.hpp"

const std::string galleryViewXML = R"xml(
    <brls:Box
            focusable="true">
    </brls:Box>
)xml";

GalleryView::GalleryView() {
    this->inflateFromXMLString(galleryViewXML);
    brls::Logger::debug("View GalleryView: create");
    this->registerAction("Prev", brls::ControllerButton::BUTTON_LB, [this](brls::View* view)-> bool {
        prev();
        return true;
    });
    this->registerAction("Prev", brls::ControllerButton::BUTTON_LT, [this](brls::View* view)-> bool {
        prev();
        return true;
    }, true);
    this->registerAction("Prev", brls::ControllerButton::BUTTON_NAV_LEFT, [this](brls::View* view)-> bool {
        prev();
        return true;
    }, true);
    this->registerAction("Next", brls::ControllerButton::BUTTON_RB, [this](brls::View* view)-> bool {
        next();
        return true;
    });
    this->registerAction("Next", brls::ControllerButton::BUTTON_RT, [this](brls::View* view)-> bool {
        next();
        return true;
    }, true);
    this->registerAction("Next", brls::ControllerButton::BUTTON_NAV_RIGHT, [this](brls::View* view)-> bool {
        next();
        return true;
    }, true);
}

GalleryView::~GalleryView() {
    brls::Logger::debug("View GalleryView: delete");
}

brls::View* GalleryView::create() {
    return new GalleryView();
}

void GalleryView::setData(GalleryData value){
    this->clearViews();
    this->data = value;

    if(value.size() == 0)
        return;

    for(auto v : value){
        GalleryItem* item = new GalleryItem();
        item->detach();
        item->setData(v);
        item->setSize(brls::Size(getWidth(), getHeight()));
        this->addView(item, this->getChildren().size());
        brls::Logger::debug("GalleryView set Data: {}/{}", v.second, v.first);
    }

    GalleryItem* first = (GalleryItem*)this->getChildren()[0];
    first->setVisibility(brls::Visibility::VISIBLE);
    first->show([](){}, true, 500);
    this->index = 0;
}

void GalleryView::prev(){
    if(this->index <= 0)
        return;
    ((GalleryItem*)this->getChildren()[this->index])->animate(GalleryAnimation::EXIT_RIGHT);

    this->index--;
    ((GalleryItem*)this->getChildren()[this->index])->animate(GalleryAnimation::ENTER_LEFT);
}

void GalleryView::next(){
    if(this->index + 1 >= this->data.size())
        return;
    ((GalleryItem*)this->getChildren()[this->index])->animate(GalleryAnimation::EXIT_LEFT);

    this->index++;
    ((GalleryItem*)this->getChildren()[this->index])->animate(GalleryAnimation::ENTER_RIGHT);
}

void GalleryView::draw(NVGcontext *vg, float x, float y, float width, float height, brls::Style style,
                       brls::FrameContext *ctx) {

    // Enable scissoring
    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);
    Box::draw(vg, x, y, width, height, style, ctx);
    nvgRestore(vg);

    // Draw bottom points
    uint n = this->data.size();
    if(n <= 1)
        return;

    auto padding = 10;
    auto circleR = 4;

    auto drawW = padding * (n - 1) + circleR * 2 * n;
    auto drawX = (width - drawW) / 2 + getMarginLeft();
    auto drawY = height * 0.98;

    float offsetX = 0;
    for(uint i=0; i<n; i++){
        nvgBeginPath(vg);
        if(i == this->index){
            nvgFillColor(vg, RGBA(160, 160, 160, 160));
        } else {
            nvgFillColor(vg, RGBA(160, 160, 160, 48));
        }
        nvgCircle(vg, drawX + offsetX, drawY, circleR);
        offsetX += circleR * 2 + padding;
        nvgFill(vg);
    }
}


const std::string galleryItemXML = R"xml(
    <brls:Box
        width="100%"
        height="100%"
        axis="column"
        grow="1"
        wireframe="false"
        justifyContent="center"
        alignItems="center">

        <brls:Image
                maxWidth="90%"
                maxHeight="80%"
                id="gallery/image"/>
        <brls:Label
                positionType="absolute"
                positionBottom="4%"
                id="gallery/label"
                fontSize="24"/>
    </brls:Box>
)xml";

GalleryItem::GalleryItem(){
    this->inflateFromXMLString(galleryItemXML);
    this->setVisibility(brls::Visibility::INVISIBLE);
    this->hide([](){}, false, 0);
}

GalleryItem* GalleryItem::create(){
    return new GalleryItem();
}

void GalleryItem::setData(GalleryItemData value){
    this->data = value;
    this->image->setImageFromRes(value.first);
    this->label->setText(value.second);
}

void GalleryItem::animate(GalleryAnimation animation){
    this->setVisibility(brls::Visibility::VISIBLE);
    switch (animation) {
        case GalleryAnimation::ENTER_LEFT:
            brls::View::show([](){}, true, 500);
            this->contentOffsetX = -getWidth();
            startScrolling(0);
            break;
        case GalleryAnimation::ENTER_RIGHT:
            brls::View::show([](){}, true, 500);
            this->contentOffsetX = getWidth();
            startScrolling(0);
            break;
        case GalleryAnimation::EXIT_LEFT:
            brls::View::hide([](){}, true, 100);
            this->contentOffsetX = 0;
            startScrolling(-getWidth());
            break;
        case GalleryAnimation::EXIT_RIGHT:
            brls::View::hide([](){}, true, 100);
            this->contentOffsetX = 0;
            startScrolling(getWidth());
            break;
    }
}


void GalleryItem::startScrolling(float newScroll)
{
    if (newScroll == this->contentOffsetX)
        return;

    this->contentOffsetX.stop();
    this->contentOffsetX.reset();
    this->contentOffsetX.addStep(newScroll, 500, brls::EasingFunction::quadraticOut);
    this->contentOffsetX.setTickCallback([this] {
        this->setTranslationX(this->contentOffsetX);
    });
    this->contentOffsetX.start();
    this->invalidate();
}