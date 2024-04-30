//
// Created by fang on 2023/1/11.
//

#pragma once

#include <mutex>
#include <optional>

#include <nanovg.h>
#include <borealis/core/singleton.hpp>
#include <borealis/core/animation.hpp>
#include <borealis/core/geometry.hpp>
#include <borealis/core/font.hpp>

#include "utils/event_helper.hpp"

// 每个分片内的svg数据，一般 1/30 s 一帧
class MaskSvg {
public:
    MaskSvg(const std::string &svg, uint64_t t) : svg(svg), showTime(t) {}
    std::string svg;
    uint64_t showTime;
};

// 一般 10s 一个分片
class MaskSlice {
public:
    MaskSlice(uint64_t time, uint64_t offsetStart, uint64_t offsetEnd)
        : time(time), offsetStart(offsetStart), offsetEnd(offsetEnd) {}
    uint64_t time{};
    uint64_t offsetStart{};
    uint64_t offsetEnd{};
    std::vector<MaskSvg> svgData;

    bool isLoaded() const;
};

class WebMask {
public:
    std::string url;
    int32_t version, check, length;
    std::vector<MaskSlice> sliceData;

    const MaskSlice &getSlice(size_t index);

    /// 解析头部前16字节，获取 web mask 总段数 (每段储存 30fps 10s 数据)
    void parseHeader1(const std::string &text);

    /// 根据获取到的总段数，解析每段出现的时间与数据偏移位置
    void parseHeader2(const std::string &text);

    bool isLoaded() const;

    void clear();
};

enum class DanmakuFontStyle {
    DANMAKU_FONT_STROKE  = 0,  // 文字外有包边
    DANMAKU_FONT_INCLINE = 1,  // 文字右下方有包边
    DANMAKU_FONT_SHADOW  = 2,  // 文字下方有阴影
    DANMAKU_FONT_PURE    = 3,  // 纯色
};

enum class DanmakuImageType {
    DANMAKU_IMAGE_NONE = 0,  // 无图片
    DANMAKU_IMAGE_OHH,
    DANMAKU_IMAGE_HIGHLIGHT,
};

class AdvancedAnimation {
public:
    // 起点和结束点的坐标
    float startX{}, startY{}, endX{}, endY{};
    // 在起始点显示的时间，运动中的时间，在结束点显示的时间
    float time1{}, time2, time3{}, wholeTime{};
    // 半透明的起始和结束值
    float alpha1{}, alpha2{};
    // z / y 轴旋转
    float rotateZ{}, rotateY{};
    // 点的坐标是相对布局还是绝对布局
    bool relativeLayout{};
    // 动画是否为线形动画
    bool linear{};
    brls::Animatable transX, transY, alpha;

    // 路径跟随线路
    std::vector<brls::Point> path;
};

class DanmakuItem {
public:
    DanmakuItem(std::string content, const char *attributes);

    std::string msg;  // 弹幕内容
    float time;       // 弹幕出现的时间
    int type;         // 弹幕类型 1/2/3: 普通; 4: 底部; 5: 顶部;
    float fontSize;   // 弹幕字号 18/25/36, 以 25 为 1.0
    int fontColor;    // 弹幕颜色

    bool isShown         = false;
    bool showing         = false;
    bool canShow         = true;
    bool isDefaultColor  = true;  // 弹幕为默认颜色
    float length         = 0;
    int line             = 0;  // 弹幕在屏幕上的行数
    float speed          = 0;
    int64_t startTime    = 0;
    NVGcolor color       = nvgRGBA(255, 255, 255, 160);
    NVGcolor borderColor = nvgRGBA(0, 0, 0, 160);
    int level;  // 弹幕等级 1-10
    std::optional<AdvancedAnimation> advancedAnimation;
    DanmakuImageType image{};  // 弹幕图片类型
    // 暂时用不到的信息，先不使用
    //    int pubDate; // 弹幕发送时间
    //    int pool; // 弹幕池类型
    //    char hash[9] = {0};
    //    uint64_t dmid; // 弹幕ID

    bool operator<(const DanmakuItem &item) const { return this->time < item.time; }

    inline void draw(NVGcontext *vg, float x, float y, float alpha, bool multiLine = false) const;

    static inline NVGcolor a(NVGcolor color, float alpha);
};

class DanmakuCore : public brls::Singleton<DanmakuCore> {
public:
    DanmakuCore();
    ~DanmakuCore();
    /**
     * 重置弹幕数据
     */
    void reset();

    /**
     * 将当前弹幕列表重新编号
     * 下次播放弹幕时会重新处理弹幕的播放状态，比如要不要显示，在哪行显示，什么时间显示等等
     * 在更改了视频的播放速度、更改了窗口大小等内容后需要手动调用此方法以在正确的位置显示弹幕
     */
    void refresh();

    /**
     * 设置当前视频倍速，弹幕会根据倍速变动
     * @param speed
     */
    void setSpeed(double speed);

    /**
     * 将当前弹幕配置写入配置文件
     */
    static void save();

    /**
     * 绘制弹幕
     * @param vg nanovg 上下文
     * @param x 绘制区域的x坐标
     * @param y 绘制区域的y坐标
     * @param width 绘制区域的宽度
     * @param height 绘制区域的高度
     * @param alpha 组件的透明度，与弹幕本身的透明度叠加
     */
    void draw(NVGcontext *vg, float x, float y, float width, float height, float alpha);

    /**
     * 加载弹幕数据
     * @param data 弹幕列表
     */
    void loadDanmakuData(const std::vector<DanmakuItem> &data);

    /**
     * 实时添加一条弹幕
     * @param item 单条弹幕
     */
    void addSingleDanmaku(const DanmakuItem &item);

    /**
     * 获取弹幕数据
     * @return 弹幕列表
     */
    std::vector<DanmakuItem> getDanmakuData();

    /**
     * 加载遮罩数据
     * @param data 遮罩数据
     */
    void loadMaskData(const std::string &url);

    /// range: [1 - 10], 1: show all danmaku, 10: the most strong filter
    static inline int DANMAKU_FILTER_LEVEL = 1;

    static inline bool DANMAKU_FILTER_SHOW_TOP      = true;
    static inline bool DANMAKU_FILTER_SHOW_BOTTOM   = true;
    static inline bool DANMAKU_FILTER_SHOW_SCROLL   = true;
    static inline bool DANMAKU_FILTER_SHOW_COLOR    = true;
    static inline bool DANMAKU_FILTER_SHOW_ADVANCED = false;
    static inline bool DANMAKU_SMART_MASK           = true;

    /// [25, 50, 75, 100]
    static inline int DANMAKU_STYLE_AREA = 100;

    /// [10, 25, 50, 60, 70, 80, 90, 100]
    static inline int DANMAKU_STYLE_ALPHA = 80;

    /// [15, 22, 30, 37, 45, 50]
    static inline int DANMAKU_STYLE_FONTSIZE = 30;

    /// [100, 120, 140, 160, 180, 200]
    static inline int DANMAKU_STYLE_LINE_HEIGHT = 120;

    /// [stroke, incline, shadow, pure]
    static inline DanmakuFontStyle DANMAKU_STYLE_FONT = DanmakuFontStyle::DANMAKU_FONT_STROKE;

    /// [0 - 100]
    static inline int DANMAKU_RENDER_QUALITY = 100;

    /// [50 75 100 125 150]
    static inline int DANMAKU_STYLE_SPEED = 100;

    static inline bool DANMAKU_ON = true;

    // 弹幕字体 (在 config_helper 中对此初始化)
    static inline int DANMAKU_FONT = brls::FONT_INVALID;

    // 弹幕图片
    static inline int DANMAKU_IMAGE_OHH{};
    static inline int DANMAKU_IMAGE_HIGHLIGHT{};

private:
    std::mutex danmakuMutex;
    bool danmakuLoaded = false;

    // 当前显示的第一条弹幕序号
    size_t danmakuIndex = 0;

    // 弹幕列表
    std::vector<DanmakuItem> danmakuData;

    WebMask maskData{};
    size_t maskIndex      = 0;
    size_t maskLastIndex  = 0;
    size_t maskSliceIndex = 0;
    int maskTex           = 0;

    // 遮罩纹理的宽高
    uint32_t maskWidth = 0;
    uint32_t maskHeight = 0;

    // 滚动弹幕的信息 <起始时间，结束时间>
    std::vector<std::pair<float, float>> scrollLines;

    // 居中弹幕信息（顶部/底部） <起始时间，结束时间>
    std::vector<float> centerLines;

    // 弹幕显示的最大行数
    size_t lineNum;

    // 当前视频播放速度
    double videoSpeed;

    // 行高
    float lineHeight;

    MPVEvent::Subscription event_id;

    void drawMask(NVGcontext *vg, float x, float y, float width, float height);

    void clearMask(NVGcontext *vg, float x, float y, float width, float height);
};