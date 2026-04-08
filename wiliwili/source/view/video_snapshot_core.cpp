//
// Created by fang on 2025/1/1.
//

#include <borealis/core/application.hpp>
#include <borealis/core/cache_helper.hpp>
#include <borealis/core/thread.hpp>
#include <stb_image.h>
#include <cpr/cpr.h>

#include "view/video_snapshot_core.hpp"
#include "api/bilibili/util/http.hpp"

void VideoSnapshotCore::reset() {
    snapshotData    = bilibili::VideoSnapshotData{};
    snapshotTextures.clear();
    snapshotLoading.clear();
}

void VideoSnapshotCore::setSnapshotData(const bilibili::VideoSnapshotData& data) {
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
    if (!snapshotData.isValid() || index >= snapshotData.image.size()) return;
    if (index < snapshotTextures.size() && snapshotTextures[index] > 0) return;
    if (index < snapshotLoading.size() && snapshotLoading[index]) return;

    while (snapshotTextures.size() <= index) snapshotTextures.push_back(0);
    while (snapshotLoading.size() <= index) snapshotLoading.push_back(false);
    snapshotLoading[index] = true;

    std::string url = bilibili::HTTP::PROTOCOL + snapshotData.image[index];

    // Check cache first
    int tex = brls::TextureCache::instance().getCache(url);
    if (tex > 0) {
        snapshotTextures[index] = tex;
        snapshotLoading[index]  = false;
        return;
    }

    // Use cpr async callback to avoid std::thread + detach
    auto session = bilibili::HTTP::createSession();
    session->SetUrl(cpr::Url{url});
    session->GetCallback([this, url, index](const cpr::Response& r) {
        if (r.status_code != 200 || r.text.empty()) {
            brls::sync([this, index]() {
                if (index < snapshotLoading.size()) snapshotLoading[index] = false;
            });
            return;
        }

        int imageW = 0, imageH = 0, n;
        uint8_t* imageData = stbi_load_from_memory(
            (unsigned char*)r.text.data(), (int)r.text.size(), &imageW, &imageH, &n, 4);

        if (!imageData) {
            brls::sync([this, index]() {
                if (index < snapshotLoading.size()) snapshotLoading[index] = false;
            });
            return;
        }

        brls::sync([this, url, imageData, imageW, imageH, index]() {
            NVGcontext* vg = brls::Application::getNVGContext();
            int tex        = nvgCreateImageRGBA(vg, imageW, imageH, 0, imageData);
            stbi_image_free(imageData);
            if (tex > 0) brls::TextureCache::instance().addCache(url, tex);
            if (index < snapshotTextures.size()) {
                snapshotTextures[index] = tex;
                snapshotLoading[index]  = false;
            }
        });
    });
}

void VideoSnapshotCore::draw(NVGcontext* vg, float x, float y, float width, float height, float progress,
                             float duration, bool centerHintVisible) {
    if (!snapshotData.isValid()) return;
    if (snapshotTextures.empty()) return;

    int imageIdx       = findIndex(duration * progress);
    size_t sheetIdx    = (size_t)(imageIdx / tilesPerSheet());
    int posInSheet     = imageIdx % tilesPerSheet();

    // Start loading the needed sprite sheet if not yet loaded
    loadTexture(sheetIdx);

    if (sheetIdx >= snapshotTextures.size() || snapshotTextures[sheetIdx] <= 0) return;

    int col    = posInSheet % snapshotData.img_x_len;
    int row    = posInSheet / snapshotData.img_x_len;
    float srcX = (float)(col * snapshotData.img_x_size);
    float srcY = (float)(row * snapshotData.img_y_size);

    // Display at a fixed width of 240, preserving aspect ratio
    float displayW = 240.0f;
    float displayH = displayW * (float)snapshotData.img_y_size / (float)snapshotData.img_x_size;

    // Center horizontally.
    float dstX = x + (width - displayW) / 2.0f;

    // Vertical position:
    // - When the center hint (osdCenterBox2, 100×100 centered) is visible, place the thumbnail
    //   above it with a 20 px gap so they don't overlap.
    // - Otherwise, center the thumbnail inside the video area.
    float dstY;
    if (centerHintVisible) {
        // osdCenterBox2 top-edge = y + height/2 - 50
        // thumbnail bottom-edge  = osdCenterBox2 top - gap
        constexpr float HINT_HALF_H = 50.0f;   // half of the 100 px hint box
        constexpr float GAP         = 20.0f;
        dstY = y + height / 2.0f - HINT_HALF_H - GAP - displayH;
    } else {
        dstY = y + (height - displayH) / 2.0f;
    }

    float totalW = (float)(snapshotData.img_x_len * snapshotData.img_x_size);
    float totalH = (float)(snapshotData.img_y_len * snapshotData.img_y_size);
    float scaleX = displayW / (float)snapshotData.img_x_size;
    float scaleY = displayH / (float)snapshotData.img_y_size;

    // Background
    nvgBeginPath(vg);
    nvgRoundedRect(vg, dstX - 3, dstY - 3, displayW + 6, displayH + 6, 3);
    nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.8f));
    nvgFill(vg);

    // Thumbnail: use scissor to clip to the display rect, then paint the full sprite sheet
    nvgSave(vg);
    nvgScissor(vg, dstX, dstY, displayW, displayH);
    NVGpaint paint = nvgImagePattern(vg,
                                     dstX - srcX * scaleX,
                                     dstY - srcY * scaleY,
                                     totalW * scaleX,
                                     totalH * scaleY,
                                     0, snapshotTextures[sheetIdx], 1.0f);
    nvgBeginPath(vg);
    nvgRect(vg, dstX, dstY, displayW, displayH);
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    nvgRestore(vg);
}
