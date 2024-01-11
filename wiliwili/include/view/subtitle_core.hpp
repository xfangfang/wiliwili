//
// Created by fang on 2023/3/5.
//

#pragma once

#include <nanovg.h>
#include <borealis/core/singleton.hpp>
#include <borealis/core/application.hpp>

#include "bilibili/result/video_detail_result.h"
#include "utils/event_helper.hpp"
#include "presenter/presenter.h"

class SubtitleCore : public brls::Singleton<SubtitleCore>, public Presenter {
public:
    SubtitleCore();
    ~SubtitleCore();
    /**
     * 重置字幕内容为空
     */
    void reset();

    /**
     * 绘制弹幕
     * @param vg nanovg 上下文
     * @param x 绘制区域的x坐标
     * @param y 绘制区域的y坐标
     * @param width 绘制区域的宽度
     * @param height 绘制区域的高度
     * @param alpha 组件的透明度，与字幕本身的透明度叠加
     */
    void drawSubtitle(NVGcontext* vg, float x, float y, float width, float height, float alpha);

    /**
     * 获取字幕数据
     * @return 字幕数据
     */
    bilibili::VideoPageResult getSubtitleList();

    /**
     * 设置字幕数据
     * @param data 字幕数据
     */
    void setSubtitleList(const bilibili::VideoPageResult& data);

    /**
     * 选择字幕
     * @param index 字幕的索引
     */
    void selectSubtitle(size_t index);

    /**
     * 清空当前选中的字幕
     */
    void clearSubtitle();

    /**
     * 当前是否可以显示字幕
     * @return
     */
    [[nodiscard]] bool isAvailable() const;

    /**
     * 获取当前显示的字幕 ID
     * @return
     */
    [[nodiscard]] std::string getCurrentSubtitleId() const;

private:
    void onLoadSubtitle(const bilibili::VideoPageSubtitle& page);

    bilibili::VideoPageResult videoPageData;
    int subtitleFont = brls::Application::getDefaultFont();
    bilibili::VideoPageSubtitle currentSubtitle;
    NVGcolor fontColor       = nvgRGB(255, 255, 255);
    NVGcolor backgroundColor = nvgRGBA(0, 0, 0, 127);
    size_t subtitleIndex     = 0;
    MPVEvent::Subscription event_id;
};