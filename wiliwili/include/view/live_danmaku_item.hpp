#pragma once

#include <borealis/core/box.hpp>
#include <borealis/core/bind.hpp>
#include <borealis/views/label.hpp>
#include "view/live_core.hpp"
#include "view/text_box.hpp"

class LiveDanmakuItemView : public brls::Box {
public:
    LiveDanmakuItemView();
    
    // 设置弹幕内容
    void setDanmaku(const LiveDanmakuItem& danmaku);
    
    // 创建新的弹幕项视图
    static LiveDanmakuItemView* create();
    
private:
    BRLS_BIND(brls::Label, usernameLabel, "danmaku_username");
    BRLS_BIND(brls::Label, contentLabel, "danmaku_content");
    BRLS_BIND(brls::Label, levelLabel, "danmaku_level_text");
    BRLS_BIND(brls::Box, levelBox, "danmaku_user_level");
    
    // 用于显示富文本内容的TextBox
    TextBox* contentBox = nullptr;
}; 