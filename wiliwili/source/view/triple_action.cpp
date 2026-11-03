#include "view/triple_action.hpp"

#include <borealis/core/application.hpp>
#include <borealis/core/i18n.hpp>
#include <borealis/core/time.hpp>

namespace {
double seconds() { return double(brls::getCPUTimeUsec()) / 1000000.; }
brls::Point center(brls::Rect f) { return {f.getMidX(), f.getMidY()}; }
float distance(brls::Point a, brls::Point b) { return std::hypot(a.x - b.x, a.y - b.y); }
brls::Point lightCenter(TripleLikeButton* button) { return center(button->getFrame()); }
void glow(NVGcontext* vg, brls::Point point, float radius, NVGcolor pink, float alpha) {
    if (radius <= .05f || alpha <= .001f) return;
    pink.a *= alpha;
    auto clear = pink;
    clear.a    = 0;
    nvgBeginPath(vg);
    nvgCircle(vg, point.x, point.y, radius);
    nvgFillPaint(vg, nvgRadialGradient(vg, point.x, point.y, radius * .04f, radius, pink, clear));
    nvgFill(vg);
}
}  // namespace

class TripleActionGesture : public brls::GestureRecognizer {
public:
    explicit TripleActionGesture(TripleLikeButton* owner) : owner(owner) {}
    brls::GestureState recognitionLoop(brls::TouchState touch, brls::MouseState mouse, brls::View*,
                                       brls::Sound*) override {
        const bool fromTouch = touch.phase != brls::TouchPhase::NONE;
        // The desktop driver also dispatches mouse hover while a finger is
        // held. It must not cancel that independent touch stream.
        if (owner->pointerHeld && pointerFromTouch && !fromTouch) return state;
        const auto phase = touch.phase == brls::TouchPhase::NONE ? mouse.leftButton : touch.phase;
        const auto point = touch.phase == brls::TouchPhase::NONE ? mouse.position : touch.position;
        if (phase == brls::TouchPhase::START) {
            state = owner->begin(point, false) ? brls::GestureState::UNSURE : brls::GestureState::FAILED;
            if (state != brls::GestureState::FAILED) pointerFromTouch = fromTouch;
        } else if (state == brls::GestureState::INTERRUPTED || !enabled || !owner->pointerHeld ||
                   phase == brls::TouchPhase::NONE) {
            if (owner->pointerHeld) owner->end(true);
            state = brls::GestureState::FAILED;
        } else if (state != brls::GestureState::FAILED) {
            if (phase == brls::TouchPhase::END) {
                owner->move(point);
                owner->end(false);
                state = brls::GestureState::END;
            } else {
                owner->move(point);
                if (!owner->pointerHeld) return state = brls::GestureState::FAILED;
                if (seconds() - owner->heldSince >= .25)
                    state = state == brls::GestureState::UNSURE ? brls::GestureState::START : brls::GestureState::STAY;
            }
        }
        return state;
    }

private:
    TripleLikeButton* owner;
    bool pointerFromTouch = false;
};

TripleLikeButton::TripleLikeButton() {
    setFocusable(true);
    gesture = new TripleActionGesture(this);
    gesture->setEnabled(false);
    addGestureRecognizer(gesture);
    focusSubscription = brls::Application::getWindowFocusChangedEvent()->subscribe([this](bool focused) {
        focusedWindow = focused;
        if (!focused && isHeld()) end(true);
    });
}
TripleLikeButton::~TripleLikeButton() {
    brls::Application::getWindowFocusChangedEvent()->unsubscribe(focusSubscription);
}
void TripleLikeButton::setActions(std::function<bool()> gate, std::function<void()> shortClick,
                                  std::function<void()> commit) {
    allowed       = std::move(gate);
    tap           = std::move(shortClick);
    triple        = std::move(commit);
    tripleEnabled = true;
    setHideClickAnimation(true);
    gesture->setEnabled(true);
    registerAction(brls::getStr("wiliwili/player/triple/hint"), brls::BUTTON_A, [this](brls::View*) {
        begin(center(getFrame()), true);
        return true;
    });
    registerAction(
        "", brls::BUTTON_B,
        [this](brls::View*) {
            if (!isHeld()) return false;
            end(true);
            return true;
        },
        true);
    // Keep focus on the like button until the held gesture ends.
    for (auto button : {brls::BUTTON_LEFT, brls::BUTTON_RIGHT, brls::BUTTON_UP, brls::BUTTON_DOWN,
                        brls::BUTTON_NAV_LEFT, brls::BUTTON_NAV_RIGHT, brls::BUTTON_NAV_UP, brls::BUTTON_NAV_DOWN})
        registerAction("", button, [this](brls::View*) { return isHeld(); }, true, true);
}
const brls::ControllerState& TripleLikeButton::getInteractionControllerState() const {
    return brls::Application::getControllerState();
}

bool TripleLikeButton::begin(brls::Point position, bool controller) {
    if (!focusedWindow || brls::Application::isInputBlocks() || isHeld() ||
        (motion.phase() != TripleActionAnimation::Phase::Idle &&
         motion.phase() != TripleActionAnimation::Phase::Dispersing))
        return false;
    if (allowed && !allowed()) return false;
    heldSince = seconds();
    if (!motion.begin(heldSince)) return false;
    pointer        = position;
    pointerHeld    = !controller;
    controllerHeld = controller;
    brls::Application::giveFocus(this);
    return true;
}
void TripleLikeButton::move(brls::Point position) {
    pointer      = position;
    const auto f = getFrame();
    brls::Rect area(f.getMinX() - 8, f.getMinY() - 8, f.getWidth() + 16, f.getHeight() + 16);
    if (pointerHeld && !area.pointInside(position)) end(true);
}
void TripleLikeButton::advance(double now) {
    if (motion.advance(now)) {
        const auto callback = triple;
        if (callback)
            callback();
        else
            motion.resolve(false, now);
    }
}
void TripleLikeButton::end(bool canceled) {
    if (!isHeld()) return;
    const double now = seconds(), held = now - heldSince;
    // Interrupted gestures must not commit simply because a late frame crossed
    // the deadline. A real release at/after three seconds does commit once.
    if (!canceled) advance(now);
    auto f                = getFrame();
    const bool shortClick = !canceled && held < .25 && f.pointInside(pointer);
    if (motion.charging()) motion.cancel(now);
    pointerHeld = controllerHeld = false;
    const auto callback          = tap;
    if (shortClick && callback) {
        brls::Application::getAudioPlayer()->play(brls::SOUND_CLICK);
        callback();
    }
}
void TripleLikeButton::complete(bool success) { motion.resolve(success, seconds()); }
void TripleLikeButton::resetInteraction() {
    pointerHeld = controllerHeld = false;
    motion.reset();
}
void TripleLikeButton::prepareFrame() {
    if (prepared) return;
    prepared = true;
    if (!isHeld() && !motion.active()) return;
    if (getVisibility() != brls::Visibility::VISIBLE) {
        resetInteraction();
        return;
    }
    if (isHeld() && (!focusedWindow || !isFocused() || brls::Application::isInputBlocks() ||
                     (pointerHeld && gesture->getState() == brls::GestureState::INTERRUPTED)))
        end(true);
    const auto& state = getInteractionControllerState();
    if (controllerHeld && !state.buttons[brls::BUTTON_A]) end(false);
    const double now = seconds();
    advance(now);
    if (motion.active()) brls::Application::setActiveEvent(true);
}
void TripleLikeButton::frame(brls::FrameContext* ctx) {
    prepareFrame();
    brls::Box::frame(ctx);
    prepared = false;
}
void TripleLikeButton::onFocusLost() {
    if (isHeld()) end(true);
    brls::Box::onFocusLost();
}
void TripleLikeButton::willDisappear(bool resetState) {
    resetInteraction();
    brls::Box::willDisappear(resetState);
}
brls::View* TripleLikeButton::hitTest(brls::Point point) {
    if (!tripleEnabled) return brls::Box::hitTest(point);
    return getVisibility() == brls::Visibility::VISIBLE && getFrame().pointInside(point) ? this : nullptr;
}

void TripleActionBar::attach(TripleLikeButton* button, brls::View* coinTarget, brls::View* favoriteTarget) {
    like     = button;
    coin     = coinTarget;
    favorite = favoriteTarget;
}
void TripleActionBar::setRelationRenderer(std::function<void(bool, bool, bool)> callback) {
    renderState = std::move(callback);
    if (renderState) renderState(displayed[0], displayed[1], displayed[2]);
}
void TripleActionBar::setRelationState(bool liked, bool coined, bool favorited) {
    confirmed        = {liked, coined, favorited};
    const auto phase = like ? like->motion.phase() : TripleActionAnimation::Phase::Idle;
    if (phase == TripleActionAnimation::Phase::Idle || phase == TripleActionAnimation::Phase::Charging ||
        phase == TripleActionAnimation::Phase::Dispersing) {
        displayed = confirmed;
        if (renderState) renderState(displayed[0], displayed[1], displayed[2]);
    }
}
void TripleActionBar::complete(bool success) {
    if (like) like->complete(success);
}
void TripleActionBar::resetInteraction() {
    if (like) like->resetInteraction();
    updateDisplayed();
}
void TripleActionBar::updateDisplayed() {
    if (!like) return;
    auto next        = displayed;
    const auto phase = like->motion.phase();
    if (phase == TripleActionAnimation::Phase::Idle || phase == TripleActionAnimation::Phase::Dispersing)
        next = confirmed;
    else if (phase == TripleActionAnimation::Phase::Bursting || phase == TripleActionAnimation::Phase::Fading) {
        const auto image        = like->motion.sample(paintTime);
        const auto source       = lightCenter(like);
        const float maxDistance = std::max(coin ? distance(source, center(coin->getFrame())) : 0,
                                           favorite ? distance(source, center(favorite->getFrame())) : 0);
        const float radius = image.wave * (maxDistance + like->getHeight());
        next[0]            = confirmed[0];
        if (coin && radius >= distance(source, center(coin->getFrame()))) next[1] = confirmed[1];
        if (favorite && radius >= distance(source, center(favorite->getFrame()))) next[2] = confirmed[2];
    }
    if (next != displayed) {
        displayed = next;
        if (renderState) renderState(displayed[0], displayed[1], displayed[2]);
    }
}
void TripleActionBar::frame(brls::FrameContext* ctx) {
    if (getVisibility() != brls::Visibility::VISIBLE) {
        resetInteraction();
        return;
    }
    if (like) {
        like->prepareFrame();
        if (like->motion.active()) paintTime = seconds();
        updateDisplayed();
    }
    brls::Box::frame(ctx);
    // Invisible children do not reach their frame() override.
    if (like) like->prepared = false;
}
void TripleActionBar::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style style,
                           brls::FrameContext* ctx) {
    brls::Box::draw(vg, x, y, w, h, style, ctx);
    if (!like || !like->motion.active()) return;
    const auto image  = like->motion.sample(paintTime);
    const auto source = lightCenter(like);
    const float unit  = std::clamp(std::min(like->getWidth(), like->getHeight()) * .5f, 16.f, 44.f);
    const auto pink   = nvgRGB(251, 114, 153);
    const bool light  = brls::Application::getThemeVariant() == brls::ThemeVariant::LIGHT;
    nvgSave(vg);
    // Each draw only covers its small halo; no offscreen image or screen-sized
    // transparent layer is allocated for this effect.
    for (size_t i = 0; i < image.count; ++i) {
        const auto& p = image.particles[i];
        const brls::Point position(source.x + p.x * unit, source.y + p.y * unit);
        glow(vg, position, p.radius * unit * 3.4f, pink, p.alpha * (light ? .66f : .85f));
        glow(vg, position, p.radius * unit, pink, p.alpha);
    }
    const float radius = unit * (.5f + image.energy * .7f) * (1 - image.contraction * .91f);
    glow(vg, source, radius * 2.1f, pink, image.energy * (light ? .45f : .70f));
    glow(vg, source, radius, pink, image.energy * (.38f + image.contraction * .7f));
    if (image.waveAlpha > .001f) {
        const float maxDistance = std::max(coin ? distance(source, center(coin->getFrame())) : 0,
                                           favorite ? distance(source, center(favorite->getFrame())) : 0);
        const float wave = std::max(.1f, image.wave * (maxDistance + like->getHeight()));
        nvgBeginPath(vg);
        nvgCircle(vg, source.x, source.y, wave);
        auto color = pink;
        color.a    = image.waveAlpha * .18f;
        nvgStrokeColor(vg, color);
        nvgStrokeWidth(vg, unit * .48f);
        nvgStroke(vg);
        color.a = image.waveAlpha * .82f;
        nvgStrokeColor(vg, color);
        nvgStrokeWidth(vg, 1.6f);
        nvgStroke(vg);
    }
    for (size_t i = 0; i < displayed.size(); ++i) {
        if (!displayed[i] || image.targetAlpha <= .001f) continue;
        auto* target = i == 0 ? static_cast<brls::View*>(like) : i == 1 ? coin : favorite;
        if (target) glow(vg, center(target->getFrame()), unit * 1.8f, pink, image.targetAlpha);
    }
    nvgRestore(vg);
}
void TripleActionBar::willDisappear(bool resetState) {
    resetInteraction();
    brls::Box::willDisappear(resetState);
}
