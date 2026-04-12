//
// Created by fang on 2025/1/1.
//

#include <borealis/core/application.hpp>
#include <borealis/core/thread.hpp>
#include <borealis/core/logger.hpp>
#include <stb_image.h>
#include <cpr/cpr.h>

#include "view/video_snapshot_core.hpp"
#include "api/bilibili/util/http.hpp"

void VideoSnapshotCore::reset() {
    // 在主线程销毁所有已创建的 NVG 纹理
    NVGcontext* vg = brls::Application::getNVGContext();
    if (vg) {
        for (int tex : snapshotTextures) {
            if (tex > 0) nvgDeleteImage(vg, tex);
        }
    }
    // 自增世代计数器，使所有正在飞行中的异步回调失效
    snapshotGeneration++;
    snapshotData    = bilibili::VideoSnapshotData{};
    snapshotTextures.clear();
    snapshotLoading.clear();
}

void VideoSnapshotCore::setSnapshotData(const bilibili::VideoSnapshotData& data) {
    reset();  // 先销毁旧纹理，再接受新数据
    snapshotData = data;
    snapshotTextures.assign(data.image.size(), 0);
    snapshotLoading.assign(data.image.size(), false);
    // 预加载第一张精灵图
    loadTexture(0);
}

bool VideoSnapshotCore::isValid() const { return snapshotData.isValid(); }

void VideoSnapshotCore::preloadForTime(float seekTimeSec) {
    if (!snapshotData.isValid()) return;
    int imageIdx = findIndex(seekTimeSec);
    loadTexture((size_t)(imageIdx / tilesPerSheet()));
}

int VideoSnapshotCore::findIndex(float seekTime) const {
    // index[0] is always 0 (sentinel).
    // index[i] (i >= 1) is the capture time for image (i-1) in seconds.
    // Find the largest n >= 1 such that index[n] <= seekTime; the 0-based image index is n-1.
    int imageIdx    = 0;
    const auto& idx = snapshotData.index;
    for (int i = 1; i < (int)idx.size(); i++) {
        if (idx[i] <= (int)seekTime) {
            imageIdx = i - 1;
        } else {
            break;
        }
    }
    return imageIdx;
}

void VideoSnapshotCore::loadTexture(size_t index) {
    if (!snapshotData.isValid() || index >= snapshotData.image.size()) {
        brls::Logger::error("[Snapshot] loadTexture early exit: invalid data or index {} out of range", index);
        return;
    }
    if (index < snapshotTextures.size() && snapshotTextures[index] > 0) {
        return;
    }
    if (index < snapshotLoading.size() && snapshotLoading[index]) {
        return;
    }

    while (snapshotTextures.size() <= index) snapshotTextures.push_back(0);
    while (snapshotLoading.size() <= index) snapshotLoading.push_back(false);
    snapshotLoading[index] = true;

    std::string url = bilibili::HTTP::PROTOCOL + snapshotData.image[index];
    int gen         = snapshotGeneration;  // 捕获当前世代，用于检测过期回调
    brls::Logger::info("[Snapshot] loadTexture start: index={} url={} gen={}", index, url, gen);

    // Use cpr async callback to avoid std::thread + detach
    auto session = bilibili::HTTP::createSession();
    session->SetUrl(cpr::Url{url});
    session->GetCallback([this, gen, index, url](const cpr::Response& r) {
        if (r.status_code != 200 || r.text.empty()) {
            brls::Logger::error("[Snapshot] HTTP failed: index={} url={} status={} size={}", index, url, r.status_code, r.text.size());
            brls::sync([this, gen, index]() {
                if (gen != snapshotGeneration) return;  // 已 reset，忽略
                if (index < snapshotLoading.size()) snapshotLoading[index] = false;
            });
            return;
        }
        int imageW = 0, imageH = 0, n;
        uint8_t* imageData = stbi_load_from_memory(
            (unsigned char*)r.text.data(), (int)r.text.size(), &imageW, &imageH, &n, 4);
        if (!imageData) {
            brls::Logger::error("[Snapshot] stb_image decode failed: index={} url={} gen={} stb_error={}", index, url, gen, stbi_failure_reason());
            brls::sync([this, gen, index]() {
                if (gen != snapshotGeneration) return;  // 已 reset，忽略
                if (index < snapshotLoading.size()) snapshotLoading[index] = false;
            });
            return;
        }
        brls::sync([this, gen, imageData, imageW, imageH, index, url]() {
            NVGcontext* vg = brls::Application::getNVGContext();
            int tex        = nvgCreateImageRGBA(vg, imageW, imageH, 0, imageData);
            stbi_image_free(imageData);
            if (gen != snapshotGeneration) {
                brls::Logger::error("[Snapshot] gen mismatch: index={} url={} gen={} curGen={}, deleting tex={}", index, url, gen, snapshotGeneration, tex);
                if (tex > 0) nvgDeleteImage(vg, tex);
                return;
            }
            if (tex <= 0) {
                brls::Logger::error("[Snapshot] nvgCreateImageRGBA failed: index={} url={} gen={} imageW={} imageH={}", index, url, gen, imageW, imageH);
            }
            if (index < snapshotTextures.size()) {
                if (snapshotTextures[index] > 0) {
                    brls::Logger::error("[Snapshot] duplicate tex: index={} url={} oldTex={} newTex={}, deleting new", index, url, snapshotTextures[index], tex);
                    if (tex > 0) nvgDeleteImage(vg, tex);
                } else {
                    snapshotTextures[index] = tex;
                }
                snapshotLoading[index] = false;
            } else {
                brls::Logger::error("[Snapshot] out-of-bounds tex: index={} url={} tex={}, deleting", index, url, tex);
                if (tex > 0) nvgDeleteImage(vg, tex);
            }
        });
    });
}

void VideoSnapshotCore::draw(NVGcontext* vg, float x, float y, float width, float height, float progress,
                             float snapShotWidth, float positionX, float positionY) {
    if (!snapshotData.isValid()) return;
    if (snapshotTextures.empty()) return;

    int imageIdx       = findIndex(progress);
    size_t sheetIdx    = (size_t)(imageIdx / tilesPerSheet());
    int posInSheet     = imageIdx % tilesPerSheet();

    // Start loading the needed sprite sheet if not yet loaded
    loadTexture(sheetIdx);
    if (sheetIdx >= snapshotTextures.size() || snapshotTextures[sheetIdx] <= 0) return;

    int col    = posInSheet % snapshotData.img_x_len;
    int row    = posInSheet / snapshotData.img_x_len;
    float srcX = (float)(col * snapshotData.img_x_size);
    float srcY = (float)(row * snapshotData.img_y_size);

    // 计算缩略图宽高
    float displayW = snapShotWidth;
    float displayH = displayW * (float)snapshotData.img_y_size / (float)snapshotData.img_x_size;

    // 计算缩略图中心点横坐标，钳制到 VideoView 区域
    float minX = x + displayW / 2 + 8.0f;
    float maxX = x + width - displayW / 2 - 8.0f;
    float realCenterX = positionX + 22; // 保证相对拖动条按钮居中
    if (realCenterX < minX) realCenterX = minX;
    if (realCenterX > maxX) realCenterX = maxX;

    // 纵坐标：缩略图底部在进度条上方 12px
    float centerY = positionY - 12.0f;

    float dstX = realCenterX - displayW / 2.0f;
    float dstY = centerY - displayH;

    float totalW = (float)(snapshotData.img_x_len * snapshotData.img_x_size);
    float totalH = (float)(snapshotData.img_y_len * snapshotData.img_y_size);
    float scaleX = displayW / (float)snapshotData.img_x_size;
    float scaleY = displayH / (float)snapshotData.img_y_size;

    // 缩略图
    nvgSave(vg);
    nvgScissor(vg, dstX, dstY, displayW, displayH);
    NVGpaint paint = nvgImagePattern(vg,
                                     dstX - srcX * scaleX,
                                     dstY - srcY * scaleY,
                                     totalW * scaleX,
                                     totalH * scaleY,
                                     0, snapshotTextures[sheetIdx], 1.0f);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, dstX, dstY, displayW, displayH, 4.0f);
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    nvgRestore(vg);
}
