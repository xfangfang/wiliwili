//
// Created by fang on 2023/4/17.
//

#pragma once
#include <string>

class Intent {
public:
    // 开启各类视频
    static void openBV(const std::string& bvid, int cid = 0, int progress = -1);
    static void openSeasonBySeasonId(int seasonId, int progress = -1);
    static void openSeasonByEpId(int epId, int progress = -1);
    static void openLive(int id, const std::string& name = "", const std::string& views = "");

    /// 开启收藏夹
    /// \param mid 收藏夹id
    /// \param type 1为收藏夹列表 2为订阅列表
    static void openCollection(const std::string& mid, const std::string& type = "1");

    // 开启搜索
    static void openSearch(const std::string& key);
    static void openTVSearch();

    // 番剧检索
    static void openPgcFilter(const std::string& filter);

    // 开启应用设置
    static void openSetting();

    // switch 应用开启教程
    static void openHint();

    // 开启主页面
    static void openMain();

    // 浏览图片
    static void openGallery(const std::vector<std::string>& data);

    // 开启 DLNA
    static void openDLNA();

    static void _registerFullscreen(brls::Activity* activity);
};

#if defined(__linux__) || defined(_WIN32) || defined(__APPLE__)
#define ALLOW_FULLSCREEN
#endif

#ifdef ALLOW_FULLSCREEN
#define registerFullscreen(activity) Intent::_registerFullscreen(activity)
#else
#define registerFullscreen(activity) (void)activity
#endif