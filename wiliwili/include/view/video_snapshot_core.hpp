//
// Created by fang on 2025/1/1.
//

#pragma once

#include <vector>

#include <nanovg.h>
#include <borealis/core/singleton.hpp>

#include "bilibili/result/video_detail_result.h"

class VideoSnapshotCore : public brls::Singleton<VideoSnapshotCore> {
public:
    VideoSnapshotCore() = default;

    /**
     * 重置快照数据（清空纹理与加载状态）
     */
    void reset();

    /**
     * 设置快照数据，并预加载第一张精灵图
     * @param data 快照数据
     */
    void setSnapshotData(const bilibili::VideoSnapshotData& data);

    /**
     * 判断快照数据是否有效
     * @return 是否有效
     */
    bool isValid() const;

    /**
     * 根据时间（秒）找到对应的 0-based 缩略图索引
     * @param seekTime 视频当前时间（秒）
     * @return 0-based 缩略图索引
     */
    int findIndex(float seekTime) const;

    /**
     * 根据视频时间预加载对应的精灵图纹理
     * @param seekTimeSec 视频当前时间（秒）
     */
    void preloadForTime(float seekTimeSec);

    /**
     * 异步加载指定精灵图纹理（不使用 std::thread）
     * @param index 精灵图索引
     */
    void loadTexture(size_t index);

    /**
     * 绘制缩略图预览
     * @param vg nanovg 上下文
     * @param x 绘制区域的 x 坐标
     * @param y 绘制区域的 y 坐标
     * @param width 绘制区域的宽度
     * @param height 绘制区域的高度
     * @param progress 当前预览时间
     * @param snapShotWidth 缩略图宽度
     * @param positionX 预览框左上角 x 坐标
     * @param positionY 预览框左上角 y 坐标
     */
    void draw(NVGcontext* vg, float x, float y, float width, float height, float progress, float snapShotWidth,
              float positionX, float positionY);

private:
    bilibili::VideoSnapshotData snapshotData;
    std::vector<int> snapshotTextures;   // NVG 纹理 ID，每个精灵图对应一个
    std::vector<bool> snapshotLoading;   // 是否正在加载对应精灵图

    /// 每次 reset() 自增，用于识别并丢弃过期的异步回调
    int snapshotGeneration = 0;

    int tilesPerSheet() const { return snapshotData.img_x_len * snapshotData.img_y_len; }
};
