#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/gesture.hpp>
#include "utils/triple_action_animation.hpp"

class TripleActionBar;
class TripleActionGesture;

class TripleLikeButton : public brls::Box {
public:
    TripleLikeButton();
    ~TripleLikeButton() override;
    static brls::View* create() { return new TripleLikeButton(); }
    void setActions(std::function<bool()> allowed, std::function<void()> tap, std::function<void()> triple);
    void frame(brls::FrameContext* ctx) override;
    void onFocusLost() override;
    void willDisappear(bool resetState = false) override;
    brls::View* hitTest(brls::Point point) override;
    void resetInteraction();
    bool isHeld() const { return pointerHeld || controllerHeld; }
    const TripleActionAnimation& animation() const { return motion; }
    void complete(bool success);

protected:
    virtual const brls::ControllerState& getInteractionControllerState() const;

private:
    friend class TripleActionGesture;
    friend class TripleActionBar;
    TripleActionAnimation motion;
    std::function<bool()> allowed;
    std::function<void()> tap, triple;
    brls::GestureRecognizer* gesture;
    brls::Event<bool>::Subscription focusSubscription;
    bool pointerHeld = false, controllerHeld = false, focusedWindow = true, prepared = false;
    bool tripleEnabled = false;
    double heldSince   = 0;
    brls::Point pointer;
    bool begin(brls::Point position, bool controller);
    void move(brls::Point position);
    void end(bool canceled);
    void advance(double now);
    void prepareFrame();
};

// Effects are drawn by the button row, after its children and outside the
// like button's clip. No full-screen surface or idle rendering pass.
class TripleActionBar : public brls::Box {
public:
    static brls::View* create() { return new TripleActionBar(); }
    void attach(TripleLikeButton* like, brls::View* coin, brls::View* favorite);
    void setRelationRenderer(std::function<void(bool, bool, bool)> callback);
    void setRelationState(bool liked, bool coined, bool favorited);
    void complete(bool success);
    void resetInteraction();
    void frame(brls::FrameContext* ctx) override;
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style style, brls::FrameContext* ctx) override;
    void willDisappear(bool resetState = false) override;
    const std::array<bool, 3>& getDisplayedState() const { return displayed; }

private:
    TripleLikeButton* like = nullptr;
    brls::View* coin       = nullptr;
    brls::View* favorite   = nullptr;
    std::array<bool, 3> confirmed{}, displayed{};
    std::function<void(bool, bool, bool)> renderState;
    double paintTime = 0;
    void updateDisplayed();
};
