#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>
#include <borealis/views/label.hpp>
#include "view/live_core.hpp"
#include "view/text_box.hpp"

class LiveDanmakuItemView : public brls::Box {
public:
    LiveDanmakuItemView();
    ~LiveDanmakuItemView();
    
    // 设置弹幕内容
    void setDanmaku(const LiveDanmakuItem& danmaku);
    
    // 创建新的弹幕项视图
    static LiveDanmakuItemView* create();
    
    // 获取SC唯一标识
    const std::string& getSuperChatToken() const { return scToken; }
    
    // 设置SC唯一标识
    void setSuperChatToken(const std::string& token) { scToken = token; }
    
    // 设置/取消置顶状态
    void setPinned(bool pinned);
    
    // 检查是否置顶
    bool isPinned() const { return pinned; }
    
private:
    BRLS_BIND(brls::Label, usernameLabel, "danmaku_username");
    BRLS_BIND(brls::Label, contentLabel, "danmaku_content");
    BRLS_BIND(brls::Label, levelLabel, "danmaku_level_text");
    BRLS_BIND(brls::Box, levelBox, "danmaku_user_level");
    
    BRLS_BIND(brls::Box, fanMedalBox, "danmaku_fan_medal");
    BRLS_BIND(brls::Label, fanMedalLabel, "danmaku_fan_medal_text");
    BRLS_BIND(brls::Box, adminBox, "danmaku_admin");
    BRLS_BIND(brls::Label, adminLabel, "danmaku_admin_text");
    BRLS_BIND(brls::Box, vipBox, "danmaku_vip");
    BRLS_BIND(brls::Label, vipLabel, "danmaku_vip_text");
    
    // SC金额标签
    BRLS_BIND(brls::Box, scPriceBox, "danmaku_sc_price");
    BRLS_BIND(brls::Label, scPriceLabel, "danmaku_sc_price_text");
    
    // 用户头像
    BRLS_BIND(brls::Image, avatarImage, "danmaku_avatar");
    
    // 用于显示富文本内容的TextBox
    TextBox* contentBox = nullptr;
    
    // SC唯一标识
    std::string scToken;
    
    // 是否为置顶状态
    bool pinned = false;
    
    // 保存原始背景色，用于取消置顶时恢复
    NVGcolor originalBgColor = nvgRGBA(0, 0, 0, 0);
}; 