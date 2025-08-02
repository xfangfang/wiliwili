//
// Created by fang on 2025/7/27.
//

#pragma once
#include <string>
#include <borealis/core/actions.hpp>
#include <borealis/core/application.hpp>

#define WILI_DECL_SHORTCUT(func) \
public: \
    static void set##func(const std::string& key) { \
        shortcut##func = parseKey(key); \
        brls::Application::addToWatchedKeys(shortcut##func); \
    } \
    static brls::BrlsKeyCombination get##func() { return shortcut##func; } \
private: \
    inline static brls::BrlsKeyCombination shortcut##func{brls::BRLS_KBD_KEY_UNKNOWN};

class ShortcutHelper {
public:
    /**
     * @brief 解析快捷键字符串
     * @param config 快捷键字符串
     *        支持的修饰键 (可多个同时出现): ctrl,alt,shift,meta
     *        支持的按键 (只能出现一个，必须放置在最后): 0-9,a-z,f1-f12,pgup,pgdn,home,end,up,down,left,right,space,[,]
     *        例: ctrl-shift-v, ctrl-alt-f1, f2, a, meta-1 ...
     * @return 返回 BrlsKeyCombination对象；若解析失败返回的对象键值为 BRLS_KBD_KEY_UNKNOWN
     */
    static brls::BrlsKeyCombination parseKey(const std::string& config);

    // 刷新与切换快捷键
    WILI_DECL_SHORTCUT(Refresh);

    // 首页搜索快捷键
    WILI_DECL_SHORTCUT(Search);

    // 向左切换Tab
    WILI_DECL_SHORTCUT(Last);

    // 向右切换Tab
    WILI_DECL_SHORTCUT(Next);

    // 向左切换子Tab (热门、追番、影视 三个页面的二级菜单)
    WILI_DECL_SHORTCUT(LastSub);

    // 向右切换子Tab (热门、追番、影视 三个页面的二级菜单)
    WILI_DECL_SHORTCUT(NextSub);

    // 音量增大快捷键
    WILI_DECL_SHORTCUT(VolumeUp);

    // 音量减小快捷键
    WILI_DECL_SHORTCUT(VolumeDown);

    // 视频详情快捷键
    WILI_DECL_SHORTCUT(VideoProfile);

    // 弹幕快捷键
    WILI_DECL_SHORTCUT(Danmaku);

    // 播放列表快捷键
    WILI_DECL_SHORTCUT(Playlist);

    // 快进快捷键
    WILI_DECL_SHORTCUT(Forward);

    // 快退快捷键
    WILI_DECL_SHORTCUT(Rewind);

    // 设置快捷键
    WILI_DECL_SHORTCUT(Setting);

    // 视频清晰度菜单快捷键
    WILI_DECL_SHORTCUT(VideoQuality);

    // 视频倍速菜单快捷键
    WILI_DECL_SHORTCUT(VideoSpeed);

    // 视频倍速快捷键
    WILI_DECL_SHORTCUT(VideoSpeedUp);

    // 视频OSD快捷键
    WILI_DECL_SHORTCUT(VideoOsd);

    // 视频播放暂停
    WILI_DECL_SHORTCUT(VideoPause);
};