//
// Created by fang on 2022/5/31.
//

#pragma once

#include "nanovg.h"

// Thanks to: https://github.com/ollix/svg2nvg

void RenderWidgetUp(NVGcontext* context) {
    nvgBeginPath(context);
    nvgMoveTo(context, 0.0, 2.5);
    nvgLineTo(context, 10.834, 0.0);
    nvgLineTo(context, 13.334, 8.166);
    nvgLineTo(context, 2.5, 10.666);
    nvgClosePath(context);
    nvgMoveTo(context, 2.5, 1.0);
    nvgLineTo(context, 1.0, 8.166);
    nvgLineTo(context, 10.834, 9.666);
    nvgLineTo(context, 12.334, 2.5);

    nvgClosePath(context);
    nvgPathWinding(context, NVG_HOLE);
    nvgMoveTo(context, 3.0, 2.833);
    nvgLineTo(context, 3.5, 5.833);
    nvgLineTo(context, 5.5, 3.333);
    nvgLineTo(context, 6.5, 5.833);
    nvgLineTo(context, 2.5, 3.333);
    nvgClosePath(context);
    nvgPathWinding(context, NVG_HOLE);
    nvgMoveTo(context, 7.667, 2.833);
    nvgLineTo(context, 7.167, 7.333);
    nvgLineTo(context, 8.167, 6.666);
    nvgLineTo(context, 9.25, 6.666);
    nvgClosePath(context);
    nvgPathWinding(context, NVG_HOLE);
    nvgMoveTo(context, 9.25, 5.666);
    nvgLineTo(context, 8.167, 5.666);
    nvgLineTo(context, 8.167, 3.833);
    nvgLineTo(context, 9.25, 3.833);
    nvgClosePath(context);
    nvgPathWinding(context, NVG_HOLE);
    nvgStrokeWidth(context, 1.0);
    nvgFillColor(context, nvgRGBA(148, 153, 160, 255));
    nvgFill(context);
}