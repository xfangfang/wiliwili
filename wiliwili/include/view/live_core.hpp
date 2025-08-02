//
// Created by maye174 on 2023/11/13.
//

#pragma once

#include <chrono>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <memory>

typedef std::chrono::time_point<std::chrono::system_clock> time_p;

struct NVGcontext;

// 表情包映射类型
typedef std::unordered_map<std::string, std::string> lmp;

// 前向声明
namespace message {
    class Danmaku;
    class SuperChat;
}

#include "api/live/extract_messages.hpp"

#include "live/dl_emoticon.hpp"
#include <cstddef>
#include <map>
#include <memory>

#include <nanovg.h>
#include <borealis/core/singleton.hpp>

using time_p = std::chrono::time_point<std::chrono::system_clock>;
class LiveDanmakuItem {
public:
    enum class Type {
        DANMAKU,
        SUPER_CHAT
    };

    // 弹幕内容类型
    enum class ContentType {
        TEXT,           // 纯文本
        EMOTICON,       // 纯表情
        MIXED           // 包含文本和表情的混合内容
    };

    LiveDanmakuItem(std::shared_ptr<message::Danmaku> danmaku);
    LiveDanmakuItem(std::shared_ptr<message::SuperChat> sc);
    LiveDanmakuItem(const LiveDanmakuItem &item);
    LiveDanmakuItem(LiveDanmakuItem &&item) noexcept;
    ~LiveDanmakuItem() = default;
    
    Type type = Type::DANMAKU;
    ContentType contentType = ContentType::TEXT; // 默认为纯文本
    
    // 使用std::shared_ptr替换原始指针
    std::shared_ptr<message::Danmaku> danmaku;
    std::shared_ptr<message::SuperChat> super_chat;
    
    time_p time;
    size_t line  = 0;
    float length = 0;
    float speed  = 0;
    
    // 仅用于混合类型弹幕的表情位置缓存
    std::vector<std::pair<size_t, size_t>> emotePositions;
};

class LiveDanmakuCore : public brls::Singleton<LiveDanmakuCore> {
public:
    //0-60
    static inline int DANMAKU_FILTER_LEVEL_LIVE = 0;

    std::vector<std::pair<time_p, time_p>> scroll_lines;
    std::vector<int> center_lines;

    float line_height;

    std::deque<LiveDanmakuItem> next;
    std::mutex next_mutex;

    std::map<int, std::deque<LiveDanmakuItem>> now;
    
    // 表情包映射
    std::shared_ptr<lmp> emoticons;

    void reset();
    void refresh();
    void add(const std::vector<LiveDanmakuItem> &dan_l);
    void draw(NVGcontext *vg, float x, float y, float width, float height, float alpha);
    
    // 设置表情包映射
    void setEmoticons(std::shared_ptr<lmp> emotes) {
        this->emoticons = emotes;
        // 清空表情缓存，因为表情包可能已更新
        clearEmoticonCache();
    }

    bool init_danmaku(NVGcontext *vg, LiveDanmakuItem &i, float width, int LINES, float SECOND, time_p now, int time);
    
    // 判断字符串是否是表情
    bool isEmoticon(const LiveDanmakuItem& danmaku) const;
    
    // 查找文本中所有表情的位置和长度
    std::vector<std::pair<size_t, size_t>> findEmoticons(const std::string& text) const;
    
    // 渲染表情
    void drawEmoticon(NVGcontext *vg, const std::string& name, float x, float y, float size, float alpha = 1.0f);
    
    // 清理表情缓存
    void clearEmoticonCache();
    
    // 确定弹幕内容类型
    LiveDanmakuItem::ContentType determineContentType(const LiveDanmakuItem& danmaku);
};