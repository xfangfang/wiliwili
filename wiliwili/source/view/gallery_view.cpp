//
// Created by fang on 2022/8/21.
//

#include <borealis/core/touch/tap_gesture.hpp>
#include <borealis/core/application.hpp>
#include <borealis/views/image.hpp>

#include "view/gallery_view.hpp"

GalleryView::GalleryView() {
    brls::Logger::debug("View GalleryView: create");
    this->setFocusable(true);
    this->registerAction("Prev", brls::ControllerButton::BUTTON_LB, [this](brls::View* view) -> bool {
        prev();
        return true;
    });
    this->registerAction(
        "Prev", brls::ControllerButton::BUTTON_LT,
        [this](brls::View* view) -> bool {
            prev();
            return true;
        },
        true);
    this->registerAction(
        "Prev", brls::ControllerButton::BUTTON_NAV_LEFT,
        [this](brls::View* view) -> bool {
            prev();
            return true;
        },
        true);
    this->registerAction("Next", brls::ControllerButton::BUTTON_RB, [this](brls::View* view) -> bool {
        next();
        return true;
    });
    this->registerAction(
        "Next", brls::ControllerButton::BUTTON_RT,
        [this](brls::View* view) -> bool {
            next();
            return true;
        },
        true);
    this->registerAction(
        "Next", brls::ControllerButton::BUTTON_NAV_RIGHT,
        [this](brls::View* view) -> bool {
            next();
            return true;
        },
        true);

    this->addGestureRecognizer(
        new brls::TapGestureRecognizer([this](brls::TapGestureStatus status, brls::Sound* soundToPlay) {
            if (status.state != brls::GestureState::END) return;
            float width = getWidth();
            float x     = getX();
            if (isnan(width) || isnan(x)) return;
            if (status.position.x < x + width * 0.25) {
                this->prev();
            } else if (status.position.x > x + width * 0.75) {
                this->next();
            }
        }));
}

GalleryView::~GalleryView() { brls::Logger::debug("View GalleryView: delete"); }

brls::View* GalleryView::create() { return new GalleryView(); }

void GalleryView::setData(GalleryData value) {
    this->clearViews();
    this->data = value;

    if (value.empty()) return;

    for (auto v : value) {
        auto* item = new ImageGalleryItem();
        item->setData(v);
        item->setSize(brls::Size(getWidth(), getHeight()));
        item->setPaddingLeft(getPaddingLeft());
        item->setPaddingRight(getPaddingRight());
        this->addView(item, this->getChildren().size());
        brls::Logger::debug("GalleryView set Data: {}/{}", v.second, v.first);
    }

    auto* first = (GalleryItem*)this->getChildren()[0];
    first->setVisibility(brls::Visibility::VISIBLE);
    first->show([]() {}, true, 500);
    this->index = 0;
}

void GalleryView::addCustomView(GalleryItem* view) {
    view->setSize(brls::Size(getWidth(), getHeight()));
    view->setPaddingLeft(getPaddingLeft());
    view->setPaddingRight(getPaddingRight());
    this->addView(view, this->getChildren().size());
    if (this->getChildren().size() == 1) {
        auto* first = (GalleryItem*)this->getChildren()[0];
        first->setVisibility(brls::Visibility::VISIBLE);
        first->show([]() {}, true, 500);
        this->index = 0;
    }
}

void GalleryView::prev() {
    if (this->index <= 0) {
        brls::Application::getCurrentFocus()->shakeHighlight(brls::FocusDirection::LEFT);
        return;
    }
    ((GalleryItem*)this->getChildren()[this->index])->animate(GalleryAnimation::EXIT_RIGHT);

    this->index--;
    ((GalleryItem*)this->getChildren()[this->index])->animate(GalleryAnimation::ENTER_LEFT);
}

void GalleryView::next() {
    if (this->index + 1 >= this->getChildren().size()) {
        brls::Application::getCurrentFocus()->shakeHighlight(brls::FocusDirection::RIGHT);
        return;
    }
    ((GalleryItem*)this->getChildren()[this->index])->animate(GalleryAnimation::EXIT_LEFT);

    this->index++;
    ((GalleryItem*)this->getChildren()[this->index])->animate(GalleryAnimation::ENTER_RIGHT);
}

void GalleryView::setIndicatorPosition(float height) { this->indicatorPosition = height; }

void GalleryView::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                       brls::FrameContext* ctx) {
    // Enable scissoring
    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);
    Box::draw(vg, x, y, width, height, style, ctx);
    nvgRestore(vg);

    // Draw bottom points
    unsigned int n = this->getChildren().size();
    if (n <= 1) return;

    auto padding = 10;
    auto circleR = 4;

    auto drawW = padding * (n - 1) + circleR * 2 * n;
    auto drawX = (width - drawW) / 2 + getMarginLeft();
    auto drawY = height * indicatorPosition;

    float offsetX = 0;
    for (unsigned int i = 0; i < n; i++) {
        nvgBeginPath(vg);
        if (i == this->index) {
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

/// ImageGalleryItem

ImageGalleryItem::ImageGalleryItem() { this->inflateFromXMLString(galleryItemXML); }

void ImageGalleryItem::setData(GalleryItemData value) {
    this->data = value;
    this->image->setImageFromRes(value.first);
    this->label->setText(value.second);
}

/// GalleryItem

GalleryItem::GalleryItem() {
    this->setVisibility(brls::Visibility::INVISIBLE);
    this->hide([]() {}, false, 0);
    this->detach();
}

void GalleryItem::animate(GalleryAnimation animation) {
    this->setVisibility(brls::Visibility::VISIBLE);
    switch (animation) {
        case GalleryAnimation::ENTER_LEFT:
        case GalleryAnimation::ENTER_RIGHT:
            if (animation == GalleryAnimation::ENTER_LEFT)
                //从左侧进入
                this->contentOffsetX = -getWidth();
            else
                //从右侧进入
                this->contentOffsetX = getWidth();
            brls::View::show([]() {}, true, 500);
            startScrolling(0);
            {
                // 滑入屏幕时按需获取焦点
                View* view = this->getDefaultFocus();
                if (view) {
                    brls::Logger::debug("GalleryItem defaultFocus: {}", view->describe());
                    brls::Application::giveFocus(view);
                }
            }
            break;
        case GalleryAnimation::EXIT_LEFT:
        case GalleryAnimation::EXIT_RIGHT:
            brls::View::hide([]() {}, true, 100);
            this->contentOffsetX = 0;
            {
                // 滑出屏幕时将焦点归还给 GalleryView
                View* view = this->getDefaultFocus();
                if (view) {
                    brls::Application::giveFocus(this->getParent());
                }
            }
            brls::View::hide([]() {}, true, 100);
            this->contentOffsetX = 0;
            if (animation == GalleryAnimation::EXIT_LEFT)
                //向左滑出
                startScrolling(-getWidth());
            else
                //向右滑出
                startScrolling(getWidth());
            break;
    }
}

void GalleryItem::startScrolling(float newScroll) {
    if (newScroll == this->contentOffsetX) return;

    this->contentOffsetX.stop();
    this->contentOffsetX.reset();
    this->contentOffsetX.addStep(newScroll, 500, brls::EasingFunction::quadraticOut);
    this->contentOffsetX.setTickCallback([this] { this->setTranslationX(this->contentOffsetX); });
    this->contentOffsetX.start();
    this->invalidate();
}