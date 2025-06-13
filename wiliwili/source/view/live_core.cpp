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

// 创建表情缓存单例类
class EmoticonCache {
public:
    static EmoticonCache& instance() {
        static EmoticonCache instance;
        return instance;
    }
    
    // 获取表情图片，如果不在缓存中则创建并加载
    RichTextImage* getEmoticon(const std::string& name, const std::string& url, float size) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        
        // 检查是否在缓存中
        auto it = emoticon_map.find(name);
        if (it != emoticon_map.end()) {
            // 更新使用顺序
            auto list_it = std::find(lru_list.begin(), lru_list.end(), name);
            if (list_it != lru_list.end()) {
                lru_list.erase(list_it);
            }
            lru_list.push_front(name);
            
            // 更新尺寸(如果需要)
            if (it->second->width != size || it->second->height != size) {
                it->second->width = size;
                it->second->height = size;
                it->second->image->setWidth(size);
                it->second->image->setHeight(size);
            }
            
            return it->second.get();
        }
        
        // 缓存满时，删除最久未使用的项
        if (emoticon_map.size() >= MAX_CACHE_SIZE) {
            // 从列表末尾移除最久未使用的项
            while (!lru_list.empty() && emoticon_map.size() >= MAX_CACHE_SIZE) {
                std::string oldest = lru_list.back();
                lru_list.pop_back();
                
                // 在删除前释放图片资源
                auto oldest_it = emoticon_map.find(oldest);
                if (oldest_it != emoticon_map.end() && oldest_it->second && oldest_it->second->image) {
                    ImageHelper::clear(oldest_it->second->image);
                }
                emoticon_map.erase(oldest);
            }
        }
        
        // 创建新的表情图片
        auto new_emoticon = std::make_unique<RichTextImage>(url, size, size);
        
        // 加载图片
        ImageHelper::with(new_emoticon->image)->load(url);
        
        // 添加到缓存
        RichTextImage* result = new_emoticon.get();
        emoticon_map[name] = std::move(new_emoticon);
        lru_list.push_front(name);
        
        return result;
    }
    
    // 清空缓存
    void clear() {
        std::lock_guard<std::mutex> lock(cache_mutex);
        
        // 释放所有图片资源
        for (auto& pair : emoticon_map) {
            if (pair.second && pair.second->image) {
                ImageHelper::clear(pair.second->image);
            }
        }
        
        emoticon_map.clear();
        lru_list.clear();
    }
    
private:
    EmoticonCache() = default;
    ~EmoticonCache() = default;
    
    static constexpr size_t MAX_CACHE_SIZE = 50;
    
    std::mutex cache_mutex;
    std::unordered_map<std::string, std::unique_ptr<RichTextImage>> emoticon_map;
    std::list<std::string> lru_list; // 用于LRU策略
};

LiveDanmakuItem::LiveDanmakuItem(std::shared_ptr<message::Danmaku> dan) { 
    this->type = Type::DANMAKU;
    this->danmaku = dan; 
}

LiveDanmakuItem::LiveDanmakuItem(std::shared_ptr<message::SuperChat> sc) {
    this->type = Type::SUPER_CHAT;
    this->super_chat = sc;
    this->contentType = ContentType::TEXT;
}

LiveDanmakuItem::LiveDanmakuItem(const LiveDanmakuItem &item) {
    this->type = item.type;
    this->time = item.time;
    this->length = item.length;
    this->speed = item.speed;
    this->line = item.line;
    this->contentType = item.contentType;
    this->emotePositions = item.emotePositions;
    
    if (item.type == Type::DANMAKU) {
        this->danmaku = item.danmaku;
    } else if (item.type == Type::SUPER_CHAT) {
        this->super_chat = item.super_chat;
    }
}

LiveDanmakuItem::LiveDanmakuItem(LiveDanmakuItem &&item) noexcept {
    this->type = item.type;
    this->time = item.time;
    this->length = item.length;
    this->speed = item.speed;
    this->line = item.line;
    this->contentType = item.contentType;
    this->emotePositions = std::move(item.emotePositions);
    
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
    {
        std::lock_guard<std::mutex> lock(this->next_mutex);
        this->next.clear();
    }
    // 清空表情包
    if (this->emoticons) {
        this->emoticons->clear();
    }
    
    // 清空表情缓存
    clearEmoticonCache();
}

void LiveDanmakuCore::clearEmoticonCache() {
    // 使用新的单例类清空缓存
    EmoticonCache::instance().clear();
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
    std::vector<LiveDanmakuItem> filtered_danmakus;
    filtered_danmakus.reserve(dan_l.size());
    
    // 先在当前线程进行过滤，避免在锁内部执行过滤操作
    for (const auto &i : dan_l) {
        // 应用所有过滤条件
        if (i.danmaku->dan_type == 4 && !DanmakuCore::DANMAKU_FILTER_SHOW_BOTTOM)
            continue;
        else if (i.danmaku->dan_type == 5 && !DanmakuCore::DANMAKU_FILTER_SHOW_TOP)
            continue;
        else if (i.danmaku->dan_type != 4 && i.danmaku->dan_type != 5 && !DanmakuCore::DANMAKU_FILTER_SHOW_SCROLL)
            continue;
        if (i.danmaku->user_level < DANMAKU_FILTER_LEVEL_LIVE) 
            continue;
        if (i.danmaku->dan_color != 0xffffff && !DanmakuCore::DANMAKU_FILTER_SHOW_COLOR) 
            continue;
        
        // 通过过滤的弹幕添加到临时列表
        filtered_danmakus.emplace_back(i);
    }
    
    // 如果没有通过过滤的弹幕，直接返回
    if (filtered_danmakus.empty()) {
        return;
    }
    
    // 加锁保护 next 队列，使用RAII方式管理锁
    {
        std::lock_guard<std::mutex> lock(this->next_mutex);
        
        // 检查队列容量，如果队列过长则先清理一部分
        if (this->next.size() + filtered_danmakus.size() > 200) {
            // 保留最新的100条
            while (this->next.size() > 100) {
                this->next.pop_back();
            }
        }
        
        // 一次性将所有过滤后的弹幕添加到队首
        for (auto& item : filtered_danmakus) {
            this->next.emplace_front(std::move(item));
        }
    }
}

bool LiveDanmakuCore::isEmoticon(const LiveDanmakuItem& danmaku) const {
    if (!this->emoticons || this->emoticons->empty()) {
        return false;
    }

    if (danmaku.danmaku->is_emoticon) return true;

    return this->emoticons->find(danmaku.danmaku->dan) != this->emoticons->end();
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
    // 添加表情尺寸后缀
    std::string url = baseUrl;
    if (size > 40) {
        // 大表情
        url = baseUrl + ImageHelper::emoji_size2_ext;
    } else {
        // 小表情
        url = baseUrl + ImageHelper::emoji_size1_ext;
    }
    
    // 使用EmoticonCache获取表情图片
    RichTextImage* emotionImage = EmoticonCache::instance().getEmoticon(name, url, size);
    
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

    // 预先计算常量
    const float emoticon_size = DanmakuCore::DANMAKU_STYLE_FONTSIZE * 1.8f;
    const float mixed_emoticon_size = DanmakuCore::DANMAKU_STYLE_FONTSIZE * 1.05f;
    const float emoticonAlpha = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01f * alpha;
    
    size_t _time = 0;
    {
        std::lock_guard<std::mutex> lock(this->next_mutex);
        while (!this->next.empty() && init_danmaku(vg, this->next.front(), width, LINES, SECOND, _now, _time)) {
            const auto &i = next.front();
            if (this->now.find(i.danmaku->dan_color) == this->now.end())
                this->now.emplace(i.danmaku->dan_color, std::deque<LiveDanmakuItem>{});
            this->now[i.danmaku->dan_color].emplace_back(std::move(i));
            this->next.pop_front();
            _time += 80;
            if (_time > 80 * LINES) _time = 0;
        }
        
        // 限制队列大小
        while (this->next.size() > 100) {
            this->next.pop_back();
        }
    }

    for (const auto &[i, v] : this->now) {
        // 计算颜色
        r = (i >> 16) & 0xff;
        g = (i >> 8) & 0xff;
        b = i & 0xff;
        NVGcolor color = nvgRGB(r, g, b);
        color.a = DanmakuCore::DANMAKU_STYLE_ALPHA * 0.01 * alpha;
        
        NVGcolor border_color = nvgRGBA(0, 0, 0, DanmakuCore::DANMAKU_STYLE_ALPHA * 1.28 * alpha);
        if ((r * 299 + g * 587 + b * 114) < 60000) {
            border_color = nvgRGBA(255, 255, 255, DanmakuCore::DANMAKU_STYLE_ALPHA * 1.28 * alpha);
        }

        // 绘制描边
        if (DanmakuCore::DANMAKU_STYLE_FONT != DanmakuFontStyle::DANMAKU_FONT_PURE) {
            float dx, dy;
            dx = dy = DanmakuCore::DANMAKU_STYLE_FONT == DanmakuFontStyle::DANMAKU_FONT_INCLINE;
            nvgFontBlur(vg, DanmakuCore::DANMAKU_STYLE_FONT == DanmakuFontStyle::DANMAKU_FONT_SHADOW);
            nvgFontDilate(vg, DanmakuCore::DANMAKU_STYLE_FONT == DanmakuFontStyle::DANMAKU_FONT_STROKE);

            nvgFillColor(vg, border_color);
            for (const auto &j : v) {
                // 计算位置
                float position = j.speed * std::chrono::duration<float>(_now - j.time).count();
                
                // 跳过表情和混合表情弹幕，它们不需要描边
                if (j.contentType != LiveDanmakuItem::ContentType::TEXT) {
                    continue;
                }
                
                // 根据弹幕类型绘制描边
                if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                    // 顶部或底部弹幕
                    nvgText(vg, x + width / 2 - j.length / 2 + dx, y + j.line * line_height + 5 + dy, j.danmaku->dan.c_str(),
                            nullptr);
                } else if (position > 0) {
                    // 滚动弹幕
                    nvgText(vg, x + width - position + dx, y + j.line * line_height + 5 + dy, j.danmaku->dan.c_str(), nullptr);
                }
            }
            nvgFontBlur(vg, 0.0f);
            nvgFontDilate(vg, 0.0f);
        }

        // 绘制主体
        nvgFillColor(vg, color);
        for (const auto &j : v) {
            float position = j.speed * std::chrono::duration<float>(_now - j.time).count();
            
            // 计算基准X位置用于所有类型的弹幕
            float baseX = 0;
            if (j.danmaku->dan_type == 4 || j.danmaku->dan_type == 5) {
                // 顶部或底部弹幕，居中显示
                baseX = x + width / 2 - j.length / 2;
            } else if (position > 0) {
                // 滚动弹幕
                baseX = x + width - position;
            } else {
                continue; // 还未开始滚动
            }
            
            // 根据内容类型绘制弹幕
            switch (j.contentType) {
                case LiveDanmakuItem::ContentType::EMOTICON: {
                    // 纯表情弹幕
                    drawEmoticon(vg, j.danmaku->dan, 
                                baseX + emoticon_size/2, 
                                y + j.line * line_height + 8, 
                                emoticon_size, emoticonAlpha);
                    break;
                }
                case LiveDanmakuItem::ContentType::MIXED: {
                    // 混合表情弹幕，使用预先计算的表情位置
                    std::string fullText = j.danmaku->dan;
                    float v_offset = 0;
                    float cursorX = 0;
                    
                    // 按顺序绘制文本和表情
                    size_t lastEnd = 0;
                    for (const auto& [start, len] : j.emotePositions) {
                        // 绘制表情前的文本
                        if (start > lastEnd) {
                            std::string textPart = fullText.substr(lastEnd, start - lastEnd);
                            float bounds[4];
                            nvgTextBounds(vg, 0, 0, textPart.c_str(), nullptr, bounds);
                            float textWidth = bounds[2] - bounds[0];
                            
                            // 设置文本颜色
                            nvgFillColor(vg, color);
                            nvgText(vg, baseX + cursorX, y + j.line * line_height + 5, textPart.c_str(), nullptr);
                            cursorX += textWidth;
                        }
                        
                        // 绘制表情
                        std::string emoteName = fullText.substr(start, len);
                        drawEmoticon(vg, emoteName, 
                                    baseX + cursorX + mixed_emoticon_size/2, 
                                    y + j.line * line_height + 5 + v_offset,
                                    mixed_emoticon_size, emoticonAlpha);
                        
                        // 更新光标位置
                        cursorX += mixed_emoticon_size;
                        lastEnd = start + len;
                    }
                    
                    // 绘制剩余的文本
                    if (lastEnd < fullText.length()) {
                        std::string textPart = fullText.substr(lastEnd);
                        nvgFillColor(vg, color);
                        nvgText(vg, baseX + cursorX, y + j.line * line_height + 5, textPart.c_str(), nullptr);
                    }
                    break;
                }
                case LiveDanmakuItem::ContentType::TEXT:
                default: {
                    // 纯文本弹幕
                    nvgText(vg, baseX, y + j.line * line_height + 5, j.danmaku->dan.c_str(), nullptr);
                    break;
                }
            }
        }
    }
    nvgRestore(vg);

    // 清理过期弹幕
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
    // 首先预计算弹幕内容类型
    i.contentType = determineContentType(i);
    
    // 如果是混合表情类型，预先保存表情位置
    if (i.contentType == LiveDanmakuItem::ContentType::MIXED) {
        i.emotePositions = findEmoticons(i.danmaku->dan);
    }
    
    // 计算弹幕长度（如果尚未计算）
    if (!i.length) {
        if (i.contentType == LiveDanmakuItem::ContentType::EMOTICON) {
            // 纯表情弹幕使用固定长度
            i.length = DanmakuCore::DANMAKU_STYLE_FONTSIZE * 2.0f;
        } else if (i.contentType == LiveDanmakuItem::ContentType::MIXED) {
            // 混合表情弹幕精确计算长度
            float totalLength = 0;
            size_t lastEnd = 0;
            float emoticon_size = DanmakuCore::DANMAKU_STYLE_FONTSIZE * 1.05f;
            
            for (const auto& [start, len] : i.emotePositions) {
                // 计算表情前文本的长度
                if (start > lastEnd) {
                    std::string textPart = i.danmaku->dan.substr(lastEnd, start - lastEnd);
                    float bounds[4];
                    nvgTextBounds(vg, 0, 0, textPart.c_str(), nullptr, bounds);
                    totalLength += bounds[2] - bounds[0];
                }
                
                // 加上表情的长度
                totalLength += emoticon_size;
                lastEnd = start + len;
            }
            
            // 加上剩余文本的长度
            if (lastEnd < i.danmaku->dan.length()) {
                std::string textPart = i.danmaku->dan.substr(lastEnd);
                float bounds[4];
                nvgTextBounds(vg, 0, 0, textPart.c_str(), nullptr, bounds);
                totalLength += bounds[2] - bounds[0];
            }
            
            i.length = totalLength;
        } else {
            // 普通文本弹幕
            float bounds[4];
            nvgTextBounds(vg, 0, 0, i.danmaku->dan.c_str(), nullptr, bounds);
            i.length = bounds[2] - bounds[0];
        }
        
        // 确保长度至少为1
        if (i.length < 1) i.length = 1;
    }
    
    // 计算速度和时间
    i.speed = (width + i.length) / SECOND;
    i.time  = now + std::chrono::milliseconds(time);

    // 如果是纯表情弹幕，并且不是滚动弹幕，则直接抛弃
    if (i.contentType == LiveDanmakuItem::ContentType::EMOTICON && (i.danmaku->dan_type == 4 || i.danmaku->dan_type == 5)) {
        return false;
    }

    // 根据弹幕类型分配行数
    for (int k = 0; k < LINES; ++k) {
        if (i.danmaku->dan_type == 4 && !center_lines[LINES - k - 1]) {
            // 底部弹幕
            center_lines[LINES - k - 1] = 1;
            i.line = LINES - k - 1;
            return true;
        } else if (i.danmaku->dan_type == 5 && !center_lines[k]) {
            // 顶部弹幕
            center_lines[k] = 1;
            i.line = k;
            return true;
        } else if (i.time > scroll_lines[k].first &&
                  i.time + std::chrono::milliseconds(size_t(width / i.speed * 1000.0f)) > scroll_lines[k].second) {
            // 滚动弹幕
            
            // 密度调整
            float bufferFactor = 1.05f;
            if (i.contentType == LiveDanmakuItem::ContentType::EMOTICON) {
                bufferFactor = 1.1f; // 纯表情弹幕需要更多缓冲
            }
            
            // 一条弹幕末尾出现的时间点
            scroll_lines[k].first = i.time + std::chrono::milliseconds(size_t(i.length * bufferFactor / i.speed * 1000.0f));
            // 一条弹幕完全消失的时间点
            scroll_lines[k].second = i.time + std::chrono::milliseconds(size_t(SECOND * 1000.0f));
            i.line = k;
            
            // 如果是纯表情弹幕，需要检查是否需要额外空间
            if (i.contentType == LiveDanmakuItem::ContentType::EMOTICON && k + 1 < LINES) {
                // 检查下一行是否也可用
                if (!(i.time > scroll_lines[k + 1].first &&
                     i.time + std::chrono::milliseconds(size_t(width / i.speed * 1000.0f)) > scroll_lines[k + 1].second)) {
                    // 下一行不可用，当前行也不能用，尝试其他行
                    continue;
                }
                
                // 占用下一行作为表情的额外空间
                scroll_lines[k + 1].first = i.time + std::chrono::milliseconds(size_t(i.length * bufferFactor / i.speed * 1000.0f));
                scroll_lines[k + 1].second = i.time + std::chrono::milliseconds(size_t(SECOND * 1000.0f));
            }
            
            return true;
        }
    }
    return false;
}

LiveDanmakuItem::ContentType LiveDanmakuCore::determineContentType(const LiveDanmakuItem& danmaku) {
    // 如果是SC，直接返回TEXT类型
    if (danmaku.type == LiveDanmakuItem::Type::SUPER_CHAT) {
        return LiveDanmakuItem::ContentType::TEXT;
    }
    
    // 检查是否是纯表情
    if (isEmoticon(danmaku)) {
        return LiveDanmakuItem::ContentType::EMOTICON;
    }
    
    // 检查是否含有混合表情
    auto emotePositions = findEmoticons(danmaku.danmaku->dan);
    if (!emotePositions.empty()) {
        return LiveDanmakuItem::ContentType::MIXED;
    }
    
    // 默认为纯文本
    return LiveDanmakuItem::ContentType::TEXT;
}