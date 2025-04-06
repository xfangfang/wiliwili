//
// Created by maye174 on 2023/11/13.
//

#include "view/live_core.hpp"
#include "view/danmaku_core.hpp"
#include "view/text_box.hpp"
#include "utils/image_helper.hpp"

#include <chrono>
#include <cstddef>

#include "nanovg.h"
#include <borealis/core/application.hpp>
#include <borealis/core/logger.hpp>

LiveDanmakuItem::LiveDanmakuItem(std::shared_ptr<message::Danmaku> dan) { 
    this->type = Type::DANMAKU;
    this->danmaku = dan; 
}

LiveDanmakuItem::LiveDanmakuItem(std::shared_ptr<message::SuperChat> sc) {
    this->type = Type::SUPER_CHAT;
    this->super_chat = sc;
}

LiveDanmakuItem::LiveDanmakuItem(const LiveDanmakuItem &item) {
    this->type    = item.type;
    this->time    = item.time;
    this->length  = item.length;
    this->speed   = item.speed;
    this->line    = item.line;
    
    if (item.type == Type::DANMAKU) {
        this->danmaku = item.danmaku;
    } else if (item.type == Type::SUPER_CHAT) {
        this->super_chat = item.super_chat;
    }
}

LiveDanmakuItem::LiveDanmakuItem(LiveDanmakuItem &&item) noexcept {
    this->type    = item.type;
    this->time    = item.time;
    this->length  = item.length;
    this->speed   = item.speed;
    this->line    = item.line;
    
    if (item.type == Type::DANMAKU) {
        this->danmaku = std::move(item.danmaku);
    } else if (item.type == Type::SUPER_CHAT) {
        this->super_chat = std::move(item.super_chat);
    }
}

void LiveDanmakuCore::reset() {
    this->scroll_lines.clear();
    this->center_lines.clear();
    this->now.clear();
    this->next_mutex.lock();
    while (!this->next.empty()) {
        this->next.pop_front();
    }
    this->next_mutex.unlock();
    
    // 清空表情包
    if (this->emoticons) {
        this->emoticons->clear();
    }
    
    // 清空表情缓存
    clearEmoticonCache();
}

void LiveDanmakuCore::clearEmoticonCache() {
    static std::mutex emoticon_cache_mutex;
    static std::unordered_map<std::string, std::unique_ptr<RichTextImage>>& emotionImageCache = 
        []() -> std::unordered_map<std::string, std::unique_ptr<RichTextImage>>& {
            static std::unordered_map<std::string, std::unique_ptr<RichTextImage>> cache;
            return cache;
        }();
    
    // 使用锁保护静态缓存
    std::lock_guard<std::mutex> lock(emoticon_cache_mutex);
    
    // 在清除缓存前，先释放所有图片资源
    for (auto& pair : emotionImageCache) {
        if (pair.second && pair.second->image) {
            ImageHelper::clear(pair.second->image);
        }
    }
    
    // 然后清空缓存
    emotionImageCache.clear();
}

void LiveDanmakuCore::refresh() {
    // 保留现有的弹幕数据，但重新初始化显示参数
    
    // 计算弹幕显示的行数
    int lineNum = brls::Application::windowHeight / DanmakuCore::DANMAKU_STYLE_FONTSIZE;
    if (lineNum < 1) lineNum = 1;
    
    // 重置行时间信息
    this->scroll_lines.clear();
    this->center_lines.clear();
    
    // 调整行数
    this->scroll_lines.resize(lineNum);
    this->center_lines.resize(lineNum, 0);
    
    // 设置行高
    this->line_height = DanmakuCore::DANMAKU_STYLE_FONTSIZE * DanmakuCore::DANMAKU_STYLE_LINE_HEIGHT * 0.01f;
}

void LiveDanmakuCore::add(const std::vector<LiveDanmakuItem> &dan_l) {
    // 加锁保护 next 队列
    std::lock_guard<std::mutex> lock(this->next_mutex);
    
    for (const auto &i : dan_l) {
        if (i.danmaku->dan_type == 4 && !DanmakuCore::DANMAKU_FILTER_SHOW_BOTTOM)
            continue;
        else if (i.danmaku->dan_type == 5 && !DanmakuCore::DANMAKU_FILTER_SHOW_TOP)
            continue;
        else if (i.danmaku->dan_type != 4 && i.danmaku->dan_type != 5 && !DanmakuCore::DANMAKU_FILTER_SHOW_SCROLL)
            continue;
        if (i.danmaku->user_level < DANMAKU_FILTER_LEVEL_LIVE) continue;
        if (i.danmaku->dan_color != 0xffffff && !DanmakuCore::DANMAKU_FILTER_SHOW_COLOR) continue;
        
        // 不需要再加锁，因为外层已经加锁了
        this->next.emplace_front(std::move(i));
    }
}

bool LiveDanmakuCore::isEmoticon(const std::string& text) const {
    if (!this->emoticons || this->emoticons->empty()) {
        return false;
    }
    
    return this->emoticons->find(text) != this->emoticons->end();
}

// 判断文本中是否包含表情，返回所有表情的起始位置和长度
std::vector<std::pair<size_t, size_t>> LiveDanmakuCore::findEmoticons(const std::string& text) const {
    std::vector<std::pair<size_t, size_t>> result;
    if (!this->emoticons || this->emoticons->empty()) {
        return result;
    }
    
    // 遍历所有表情，查找它们在文本中的位置
    for (const auto& [emoteName, _] : *this->emoticons) {
        // 只有带[]的才是混合弹幕中的表情
        if (emoteName.size() < 2 || emoteName[0] != '[' || emoteName.back() != ']') {
            continue;
        }
        
        size_t pos = 0;
        while ((pos = text.find(emoteName, pos)) != std::string::npos) {
            // 找到表情，记录位置和长度
            result.push_back({pos, emoteName.length()});
            pos += emoteName.length();
        }
    }
    
    // 按照位置排序，确保按顺序处理
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });
    
    // 处理重叠的表情（保留较长的一个）
    if (!result.empty()) {
        std::vector<std::pair<size_t, size_t>> filteredResult;
        filteredResult.push_back(result[0]);
        
        for (size_t i = 1; i < result.size(); i++) {
            const auto& prev = filteredResult.back();
            const auto& curr = result[i];
            
            // 如果当前表情与上一个表情重叠
            if (curr.first < prev.first + prev.second) {
                // 保留较长的表情
                if (curr.second > prev.second) {
                    filteredResult.pop_back();
                    filteredResult.push_back(curr);
                }
            } else {
                filteredResult.push_back(curr);
            }
        }
        
        result = std::move(filteredResult);
    }
    
    return result;
}

void LiveDanmakuCore::drawEmoticon(NVGcontext *vg, const std::string& name, float x, float y, float size, float alpha) {
    if (!this->emoticons || this->emoticons->empty()) {
        return;
    }
    
    auto it = this->emoticons->find(name);
    if (it == this->emoticons->end()) {
        return; // 表情不存在
    }
    
    // 保存当前NVG状态
    nvgSave(vg);
    
    const std::string& baseUrl = it->second;
    // 添加表情尺寸后缀，参考视频评论区的实现
    std::string url = baseUrl;
    if (size > 40) {
        // 大表情
        url = baseUrl + ImageHelper::emoji_size2_ext;
    } else {
        // 小表情
        url = baseUrl + ImageHelper::emoji_size1_ext;
    }
    
    // 创建RichTextImage对象
    static std::mutex emoticon_cache_mutex;
    static constexpr size_t MAX_CACHE_SIZE = 50; // 最大缓存数量
    static std::unordered_map<std::string, std::unique_ptr<RichTextImage>>& emotionImageCache = 
        []() -> std::unordered_map<std::string, std::unique_ptr<RichTextImage>>& {
            static std::unordered_map<std::string, std::unique_ptr<RichTextImage>> cache;
            return cache;
        }();
    
    RichTextImage* emotionImage = nullptr;
    
    {
        // 使用锁保护缓存访问
        std::lock_guard<std::mutex> lock(emoticon_cache_mutex);
        
        auto cacheIt = emotionImageCache.find(name);
        if (cacheIt == emotionImageCache.end()) {
            // 缓存过大时，随机清除一个
            if (emotionImageCache.size() >= MAX_CACHE_SIZE) {
                auto it = emotionImageCache.begin();
                std::advance(it, rand() % emotionImageCache.size());
                // 在删除前先释放图片资源
                if (it->second && it->second->image) {
                    ImageHelper::clear(it->second->image);
                }
                emotionImageCache.erase(it);
            }
            
            // 如果缓存中没有，创建新的RichTextImage
            emotionImageCache[name] = std::make_unique<RichTextImage>(url, size, size);
            emotionImage = emotionImageCache[name].get();
            
            // 加载图片
            ImageHelper::with(emotionImage->image)->load(url);
        } else {
            emotionImage = cacheIt->second.get();
            // 更新尺寸（如果需要）
            if (emotionImage->width != size || emotionImage->height != size) {
                emotionImage->width = size;
                emotionImage->height = size;
                emotionImage->image->setWidth(size);
                emotionImage->image->setHeight(size);
            }
        }
    } // 锁在这里释放
    
    // 设置位置
    emotionImage->setPosition(x, y);
    
    // 设置透明度
    emotionImage->image->setAlpha(alpha);
    
    // 绘制表情
    emotionImage->image->draw(vg, x - size/2, y, size, size, brls::Application::getStyle(), nullptr);
    
    // 恢复NVG状态
    nvgRestore(vg);
}

void LiveDanmakuCore::draw(NVGcontext *vg, float x, float y, float width, float height, float alpha) {
    if (!DanmakuCore::DANMAKU_ON) return;

    int r, g, b;
    float SECOND        = 0.12f * DanmakuCore::DANMAKU_STYLE_SPEED;
    float CENTER_SECOND = 0.04f * DanmakuCore::DANMAKU_STYLE_SPEED;
    line_height         = DanmakuCore::DANMAKU_STYLE_FONTSIZE * DanmakuCore::DANMAKU_STYLE_LINE_HEIGHT * 0.01f;
    size_t LINES        = height / this->line_height * DanmakuCore::DANMAKU_STYLE_AREA * 0.01;
    if (LINES < 1) LINES = 1;

    if (this->scroll_lines.size() < LINES) {
        this->scroll_lines.resize(LINES);
        this->center_lines.resize(LINES, 0);
    }

    // Enable scissoring
    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);

    nvgFontSize(vg, DanmakuCore::DANMAKU_STYLE_FONTSIZE);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, DanmakuCore::DANMAKU_FONT);
    nvgTextLineHeight(vg, 1);

    // 弹幕渲染质量
    if (DanmakuCore::DANMAKU_RENDER_QUALITY < 100) {
        nvgFontQuality(vg, 0.01f * DanmakuCore::DANMAKU_RENDER_QUALITY);
    }

    auto _now = std::chrono::system_clock::now();

    size_t _time = 0;
    this->next_mutex.lock();
    while (!this->next.empty() && init_danmaku(vg, this->next.front(), width, LINES, SECOND, _now, _time)) {
        const auto &i = next.front();
        if (this->now.find(i.danmaku->dan_color) == this->now.end())
            this->now.emplace(i.danmaku->dan_color, std::deque<LiveDanmakuItem>{});
        this->now[i.danmaku->dan_color].emplace_back(std::move(i));
        this->next.pop_front();
        _time += 80;
        if (_time > 80 * LINES) _time = 0;
    }
    while (this->next.size() > 100) {
        this->next.pop_back();
    }
    this->next_mutex.unlock();

    for (const auto &[i, v] : this->now) {
        r                     = (i >> 16) & 0xff;
        g                     = (i >> 8) & 0xff;
        b                     = i & 0xff;
        NVGcolor color        = nvgRGB(r, g, b);
        color.a               = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01 * alpha;
        NVGcolor border_color = nvgRGBA(0, 0, 0, DanmakuCore::DANMAKU_STYLE_ALPHA * 1.28 * alpha);
        if ((r * 299 + g * 587 + b * 114) < 60000) {
            border_color = nvgRGBA(255, 255, 255, DanmakuCore::DANMAKU_STYLE_ALPHA * 1.28 * alpha);
        }

        if (DanmakuCore::DANMAKU_STYLE_FONT != DanmakuFontStyle::DANMAKU_FONT_PURE) {
            float dx, dy;
            dx = dy = DanmakuCore::DANMAKU_STYLE_FONT == DanmakuFontStyle::DANMAKU_FONT_INCLINE;
            nvgFontBlur(vg, DanmakuCore::DANMAKU_STYLE_FONT == DanmakuFontStyle::DANMAKU_FONT_SHADOW);
            nvgFontDilate(vg, DanmakuCore::DANMAKU_STYLE_FONT == DanmakuFontStyle::DANMAKU_FONT_STROKE);

            nvgFillColor(vg, border_color);
            for (const auto &j : v) {
                float position = j.speed * std::chrono::duration<float>(_now - j.time).count();
                
                // 检查是否是表情
                if (j.danmaku->is_emoticon || isEmoticon(j.danmaku->dan)) {
                    // 表情不需要绘制描边
                    continue;
                }
                
                // 检查是否包含混合表情 (带有[])
                auto emotePositions = findEmoticons(j.danmaku->dan);
                if (!emotePositions.empty()) {
                    // 包含表情的文本不在这里绘制描边，而是在下面单独处理
                    continue;
                }
                
                if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                    nvgText(vg, x + width / 2 - j.length / 2 + dx, y + j.line * line_height + 5 + dy, j.danmaku->dan.c_str(),
                            nullptr);
                } else if (position > 0) {
                    nvgText(vg, x + width - position + dx, y + j.line * line_height + 5 + dy, j.danmaku->dan.c_str(), nullptr);
                }
            }
            nvgFontBlur(vg, 0.0f);
            nvgFontDilate(vg, 0.0f);
        }

        nvgFillColor(vg, color);
        for (const auto &j : v) {
            float position = j.speed * std::chrono::duration<float>(_now - j.time).count();
            
            // 检查是否是表情
            if (j.danmaku->is_emoticon || isEmoticon(j.danmaku->dan)) {
                float emoticon_size = DanmakuCore::DANMAKU_STYLE_FONTSIZE * 1.1f; // 表情略大于字体
                // 计算垂直偏移量，使表情中心与文字中心对齐
                // 文字高度约为DANMAKU_STYLE_FONTSIZE，表情高度为emoticon_size
                // 需要将表情向上偏移，使其中心与文字中心对齐
                float v_offset = (DanmakuCore::DANMAKU_STYLE_FONTSIZE - emoticon_size) / 2;
                float emoticonAlpha = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01f * alpha; // 与弹幕一致的透明度
                
                if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                    // 顶部或底部弹幕
                    drawEmoticon(vg, j.danmaku->dan, 
                                 x + width / 2, 
                                 y + j.line * line_height + 5 + v_offset, 
                                 emoticon_size, emoticonAlpha);
                } else if (position > 0) {
                    // 滚动弹幕
                    drawEmoticon(vg, j.danmaku->dan, 
                                 x + width - position + emoticon_size/2, 
                                 y + j.line * line_height + 5 + v_offset, 
                                 emoticon_size, emoticonAlpha);
                }
                continue;
            }
            
            // 检查是否包含混合表情 (带有[])
            auto emotePositions = findEmoticons(j.danmaku->dan);
            if (!emotePositions.empty()) {
                // 包含表情的混合文本，需要分段绘制
                std::string fullText = j.danmaku->dan;
                float emoticon_size = DanmakuCore::DANMAKU_STYLE_FONTSIZE * 1.05f; // 表情略大于字体
                // 计算垂直偏移量，使表情中心与文字中心对齐
                float v_offset = 0;
                float emoticonAlpha = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01f * alpha; // 透明度
                float cursorX = 0;
                float baseX = 0;
                
                // 计算基准X位置
                if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                    // 顶部或底部弹幕，居中显示
                    baseX = x + width / 2 - j.length / 2;
                } else if (position > 0) {
                    // 滚动弹幕
                    baseX = x + width - position;
                } else {
                    continue; // 还未开始滚动
                }
                
                // 在每次绘制文本前重新设置颜色
                nvgFillColor(vg, color);
                
                // 按顺序绘制文本和表情
                size_t lastEnd = 0;
                for (const auto& [start, len] : emotePositions) {
                    // 绘制表情前的文本
                    if (start > lastEnd) {
                        std::string textPart = fullText.substr(lastEnd, start - lastEnd);
                        float bounds[4];
                        nvgTextBounds(vg, 0, 0, textPart.c_str(), nullptr, bounds);
                        float textWidth = bounds[2] - bounds[0];
                        
                        // 重新设置文本颜色，确保每段文本都有正确的颜色
                        nvgFillColor(vg, color);
                        nvgText(vg, baseX + cursorX, y + j.line * line_height + 5, textPart.c_str(), nullptr);
                        cursorX += textWidth;
                    }
                    
                    // 绘制表情
                    std::string emoteName = fullText.substr(start, len);
                    drawEmoticon(vg, emoteName, 
                                 baseX + cursorX + emoticon_size/2, 
                                 y + j.line * line_height + 5 + v_offset,
                                 emoticon_size, emoticonAlpha);
                    
                    // 更新光标位置 (表情的宽度)
                    cursorX += emoticon_size;
                    lastEnd = start + len;
                }
                
                // 绘制剩余的文本，再次确保颜色正确
                if (lastEnd < fullText.length()) {
                    std::string textPart = fullText.substr(lastEnd);
                    nvgFillColor(vg, color);
                    nvgText(vg, baseX + cursorX, y + j.line * line_height + 5, textPart.c_str(), nullptr);
                }
                
                continue;
            }
            
            if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                nvgText(vg, x + width / 2 - j.length / 2, y + j.line * line_height + 5, j.danmaku->dan.c_str(), nullptr);
            } else if (position > 0) {
                nvgText(vg, x + width - position, y + j.line * line_height + 5, j.danmaku->dan.c_str(), nullptr);
            }
        }
    }
    nvgRestore(vg);

    for (auto &[i, v] : this->now) {
        while (!v.empty()) {
            const auto &j  = v.front();
            float position = j.speed * std::chrono::duration<float>(_now - j.time).count();
            if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                if (j.time + std::chrono::milliseconds(size_t(CENTER_SECOND * 1000.0f)) < _now) {
                    center_lines[j.line] = 0;
                    v.pop_front();
                } else {
                    break;
                }
            } else if (position > width + j.length) {
                v.pop_front();
            } else {
                break;
            }
        }
    }
}

bool LiveDanmakuCore::init_danmaku(NVGcontext *vg, LiveDanmakuItem &i, float width, int LINES, float SECOND, time_p now,
                                   int time) {
    float bounds[4];
    if (!i.length) {
        nvgTextBounds(vg, 0, 0, i.danmaku->dan.c_str(), nullptr, bounds);
        i.length = bounds[2] - bounds[0];
        if (!i.length) i.length = 1;
    }
    i.speed = (width + i.length) / SECOND;
    i.time  = now + std::chrono::milliseconds(time);

    for (int k = 0; k < LINES; ++k) {
        if (i.danmaku->dan_type == 4 && !center_lines[LINES - k - 1]) {
            //底部
            center_lines[LINES - k - 1] = 1;
            i.line                      = LINES - k - 1;
            return true;
        } else if (i.danmaku->dan_type == 5 && !center_lines[k]) {
            //顶部
            center_lines[k] = 1;
            i.line          = k;
            return true;
        } else if (i.time > scroll_lines[k].first &&
                   i.time + std::chrono::milliseconds(size_t(width / i.speed * 1000.0f)) > scroll_lines[k].second) {
            //滚动
            // 一条弹幕末尾出现的时间点
            scroll_lines[k].first = i.time + std::chrono::milliseconds(size_t(i.length / i.speed * 1000.0f));
            // 一条弹幕完全消失的时间点
            scroll_lines[k].second = i.time + std::chrono::milliseconds(size_t(SECOND * 1000.0f));
            i.line                 = k;
            return true;
        }
    }
    return false;
}