"""
wiliwili - PS5 版按键图标字体。

把内置的 PS4 图标字体（resources/font/keymap_ps.ttf）复制一份，替换掉两个
DualSense 上不一样的图标：

  U+E0EF  OPTIONS   由 PS4 的 "O" 换成三条横线（PS5 的 OPTIONS 键）
  U+E0F0  CREATE    由 PS4 的 "S"（SHARE）换成竖胶囊加三道短斜线
                    （DualSense 的 CREATE 键）

其余图标（✕ ○ □ △、L1/R1/L2/R2、摇杆、PS 键）在 PS4 与 PS5 上相同，原样保留。

用法：
    python3 scripts/ps5/font/make_ps5_keymap.py
生成 resources/font/keymap_ps5.ttf，PS5 构建在 config_helper.cpp 里使用它。

依赖：fontTools（pip install fonttools）
"""
"""PS5 版按键图标：OPTIONS = ≡，CREATE = 竖胶囊 + 三道短斜线。"""
import math
from fontTools.ttLib import TTFont
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.pens.recordingPen import DecomposingRecordingPen

PS = "resources/font/keymap_ps.ttf"
XB = "resources/font/keymap_xbox.ttf"
OUT = "resources/font/keymap_ps5.ttf"


def arc(pen, cx, cy, r, a0, a1, steps):
    """二次贝塞尔逼近圆弧（顺时针，角度为度）。"""
    for i in range(steps):
        t0 = math.radians(a0 + (a1 - a0) * i / steps)
        t1 = math.radians(a0 + (a1 - a0) * (i + 1) / steps)
        tm = (t0 + t1) / 2
        k = r / math.cos((t1 - t0) / 2)
        pen.qCurveTo(
            (cx + k * math.cos(tm), cy + k * math.sin(tm)),
            (cx + r * math.cos(t1), cy + r * math.sin(t1)),
        )


def stadium(pen, x0, x1, y0, y1, clockwise=True):
    """竖着的胶囊形（两端半圆）。逆时针绕行即可在内圈挖空。"""
    r = (x1 - x0) / 2
    cx = (x0 + x1) / 2
    cy0, cy1 = y0 + r, y1 - r          # 上、下圆心
    if clockwise:
        pen.moveTo((x0, cy1))
        arc(pen, cx, cy1, r, 180, 0, 4)    # 上半圆
        pen.lineTo((x1, cy0))
        arc(pen, cx, cy0, r, 0, -180, 4)   # 下半圆
    else:
        pen.moveTo((x0, cy0))
        arc(pen, cx, cy0, r, 180, 360, 4)  # 下半圆
        pen.lineTo((x1, cy1))
        arc(pen, cx, cy1, r, 0, 180, 4)    # 上半圆
    pen.closePath()


def bar(pen, x0, x1, y0, y1):
    pen.moveTo((x0, y0))
    pen.lineTo((x1, y0))
    pen.lineTo((x1, y1))
    pen.lineTo((x0, y1))
    pen.closePath()


def options_glyph():
    """≡ —— 借用 xbox 字体里的三条横线，去掉外面的圆环。"""
    xb = TTFont(XB)
    g = xb["glyf"][xb.getBestCmap()[0xE0EF]]
    coords, endPts, flags = g.getCoordinates(xb["glyf"])
    pen = TTGlyphPen(None)
    start = 0
    for i, e in enumerate(endPts):
        pts = [tuple(coords[j]) for j in range(start, e + 1)]
        start = e + 1
        if len(pts) != 4:            # 圆环的两个轮廓
            continue
        pen.moveTo(pts[0])
        for p in pts[1:]:
            pen.lineTo(p)
        pen.closePath()
    return pen.glyph()


def create_glyph():
    """竖胶囊 + 三道短斜线。"""
    pen = TTGlyphPen(None)
    # 外框
    stadium(pen, 292, 708, -10, 610)
    # 内框（线宽 56）
    stadium(pen, 348, 652, 46, 554, clockwise=False)
    # 三道短斜线（反向绕行，作为独立的填充形状）
    for cy in (210, 300, 390):
        cx, half, dy, th = 500, 68, 20, 33
        pts = [
            (cx - half, cy - dy - th), (cx + half, cy + dy - th),
            (cx + half, cy + dy + th), (cx - half, cy - dy + th),
        ]
        pen.moveTo(pts[0])
        for p in pts[1:]:
            pen.lineTo(p)
        pen.closePath()
    return pen.glyph()


def main():
    ps = TTFont(PS)
    glyf = ps["glyf"]
    cmap = ps.getBestCmap()
    for code, poly in ((0xE0EF, options_glyph), (0xE0F0, create_glyph)):
        name = cmap[code]
        g = poly()
        glyf[name] = g
        g.recalcBounds(glyf)
        adv, _ = ps["hmtx"][name]
        ps["hmtx"][name] = (adv, g.xMin)
        print("U+%04X %s -> bbox (%d,%d)-(%d,%d)" % (code, name, g.xMin, g.yMin, g.xMax, g.yMax))
    ps["name"].setName("keymap_ps5 (DualSense hints for wiliwili)", 4, 3, 1, 0x409)
    ps.save(OUT)
    print("已写入", OUT)


main()
