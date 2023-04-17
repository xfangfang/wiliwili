//
// Created by fang on 2023/1/11.
//

#pragma once

#include <mutex>
#include <borealis.hpp>
#include <borealis/core/singleton.hpp>
#include "nanovg.h"

class DanmakuItem {
public:
    DanmakuItem(std::string content, const char *attributes);

    std::string msg;  // 弹幕内容
    float time;       // 弹幕出现的时间
    int type;         // 弹幕类型 1/2/3: 普通; 4: 底部; 5: 顶部;
    int fontSize;     // 弹幕字号 18/25/36
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
    // 暂时用不到的信息，先不使用
    //    int pubDate; // 弹幕发送时间
    //    int pool; // 弹幕池类型
    //    char hash[9] = {0};
    //    uint64_t dmid; // 弹幕ID

    bool operator<(const DanmakuItem &item) const {
        return this->time < item.time;
    }
};

class DanmakuCore : public brls::Singleton<DanmakuCore> {
public:
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
    void drawDanmaku(NVGcontext *vg, float x, float y, float width,
                     float height, float alpha);

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

    /// range: [1 - 10], 1: show all danmaku, 10: the most strong filter
    static inline int DANMAKU_FILTER_LEVEL = 1;

    static inline bool DANMAKU_FILTER_SHOW_TOP    = true;
    static inline bool DANMAKU_FILTER_SHOW_BOTTOM = true;
    static inline bool DANMAKU_FILTER_SHOW_SCROLL = true;
    static inline bool DANMAKU_FILTER_SHOW_COLOR  = true;

    /// [25, 50, 75, 100]
    static inline int DANMAKU_STYLE_AREA = 100;

    /// [10, 25, 50, 60, 70, 80, 90, 100]
    static inline int DANMAKU_STYLE_ALPHA = 80;

    /// [15, 22, 30, 37, 45, 50]
    static inline int DANMAKU_STYLE_FONTSIZE = 30;

    /// [100, 120, 140, 160, 180, 200]
    static inline int DANMAKU_STYLE_LINE_HEIGHT = 120;

    /// [50 75 100 125 150]
    static inline int DANMAKU_STYLE_SPEED = 100;

    static inline bool DANMAKU_ON = true;

private:
    std::mutex danmakuMutex;
    bool danmakuLoaded = false;

    // 当前显示的第一条弹幕序号
    size_t danmakuIndex = 0;

    // 弹幕列表
    std::vector<DanmakuItem> danmakuData;

    // 滚动弹幕的信息 <起始时间，结束时间>
    std::vector<std::pair<float, float>> scrollLines;

    // 居中弹幕信息（顶部/底部） <起始时间，结束时间>
    std::vector<float> centerLines;

    // 弹幕显示的最大行数
    size_t lineNum;

    // 当前弹幕显示的行数
    size_t lineNumCurrent;

    // 当前视频播放速度
    double videoSpeed;

    // 弹幕字体
    int danmakuFont = brls::Application::getDefaultFont();

    // 行高
    float lineHeight;

    static inline NVGcolor a(NVGcolor color, float alpha);
};