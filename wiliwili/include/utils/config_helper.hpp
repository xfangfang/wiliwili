//
// Created by fang on 2022/7/7.
//

#pragma once

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include "analytics.h"
#include "borealis/core/singleton.hpp"
#include "borealis/core/logger.hpp"

#ifdef USE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#elif __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include("experimental/filesystem")
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif !defined(USE_LIBROMFS)
#error "Failed to include <filesystem> header!"
#endif

#ifdef PS4
const std::string primaryDNSStr   = "223.5.5.5";
const std::string secondaryDNSStr = "1.1.1.1";
#endif

typedef std::map<std::string, std::string> Cookie;
constexpr uint32_t MINIMUM_WINDOW_WIDTH  = 480;
constexpr uint32_t MINIMUM_WINDOW_HEIGHT = 270;

enum class SettingItem {
    HIDE_BOTTOM_BAR,
    HIDE_FPS,
    FULLSCREEN,
    ALWAYS_ON_TOP,
    APP_THEME,      // 深浅主题色
    APP_LANG,       // 应用语言
    APP_RESOURCES,  // 自定义界面布局
    APP_UI_SCALE,   // 界面缩放
    HISTORY_REPORT,
    PLAYER_STRATEGY,
    PLAYER_BOTTOM_BAR,
    PLAYER_HIGHLIGHT_BAR,
    PLAYER_SKIP_OPENING_CREDITS,
    PLAYER_LOW_QUALITY,
    PLAYER_INMEMORY_CACHE,
    PLAYER_HWDEC,
    PLAYER_HWDEC_CUSTOM,
    PLAYER_EXIT_FULLSCREEN_ON_END,
    PLAYER_DEFAULT_SPEED,
    PLAYER_VOLUME,
    PLAYER_ASPECT,
    PLAYER_BRIGHTNESS,
    PLAYER_CONTRAST,
    PLAYER_SATURATION,
    PLAYER_HUE,
    PLAYER_GAMMA,
    PLAYER_OSD_TV_MODE,
    VIDEO_QUALITY,
    TEXTURE_CACHE_NUM,
    OPENCC_ON,
    CUSTOM_UPDATE_API,
    IMAGE_REQUEST_THREADS,
    VIDEO_FORMAT,
    VIDEO_CODEC,
    AUDIO_QUALITY,
    GAMEPAD_VIBRATION,
    DANMAKU_ON,
    DANMAKU_FILTER_LEVEL,
    DANMAKU_FILTER_SCROLL,
    DANMAKU_FILTER_TOP,
    DANMAKU_FILTER_BOTTOM,
    DANMAKU_FILTER_COLOR,
    DANMAKU_FILTER_ADVANCED,
    DANMAKU_SMART_MASK,
    DANMAKU_STYLE_AREA,
    DANMAKU_STYLE_ALPHA,
    DANMAKU_STYLE_FONTSIZE,
    DANMAKU_STYLE_LINE_HEIGHT,
    DANMAKU_STYLE_SPEED,
    DANMAKU_STYLE_FONT,
    DANMAKU_RENDER_QUALITY,
    KEYMAP,
    HOME_WINDOW_STATE,
    SEARCH_TV_MODE,
    LIMITED_FPS,
    DEACTIVATED_TIME,
    DEACTIVATED_FPS,
    DLNA_IP,
    DLNA_PORT,
    DLNA_NAME,
    HTTP_PROXY,
    HTTP_PROXY_STATUS,
    TLS_VERIFY,
};

class APPVersion : public brls::Singleton<APPVersion> {
    inline static std::string RELEASE_API = "https://api.github.com/repos/xfangfang/wiliwili/releases/latest";

public:
    int major, minor, revision;
    std::string git_commit, git_tag;

    APPVersion();

    std::string getVersionStr();

    std::string getPlatform();

    static std::string getPackageName();

    bool needUpdate(std::string latestVersion);

    void checkUpdate(int delay = 2000, bool showUpToDateDialog = false);
};

class CustomTheme {
public:
    std::string id;
    std::string name;
    std::string desc;
    std::string version;
    std::string author;
    std::string path;
};
inline void from_json(const nlohmann::json& nlohmann_json_j, CustomTheme& nlohmann_json_t) {
    if (nlohmann_json_j.contains("name") && nlohmann_json_j.at("name").is_string())
        nlohmann_json_j.at("name").get_to(nlohmann_json_t.name);
    if (nlohmann_json_j.contains("desc") && nlohmann_json_j.at("desc").is_string())
        nlohmann_json_j.at("desc").get_to(nlohmann_json_t.desc);
    if (nlohmann_json_j.contains("version") && nlohmann_json_j.at("version").is_string())
        nlohmann_json_j.at("version").get_to(nlohmann_json_t.version);
    if (nlohmann_json_j.contains("author") && nlohmann_json_j.at("author").is_string())
        nlohmann_json_j.at("author").get_to(nlohmann_json_t.author);
}

class SeasonCustomItem {
public:
    std::string player_aspect;
    bool custom_clip{};
    int clip_start{};
    int clip_end{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SeasonCustomItem, player_aspect, custom_clip, clip_start, clip_end);

typedef std::unordered_map<std::string, SeasonCustomItem> SeasonCustomSetting;

typedef struct ProgramOption {
    /// 保存在配置文件中的选项明
    std::string key;
    /// 字符串类型的选项
    std::vector<std::string> optionList;
    /// 数字类型的选项
    std::vector<int> rawOptionList;
    /// 默认的选项，范围为 [0 - optionList.size()-1]
    size_t defaultOption;
} ProgramOption;

class ProgramConfig : public brls::Singleton<ProgramConfig> {
public:
    ProgramConfig();
    ProgramConfig(const ProgramConfig& config);
    void setProgramConfig(const ProgramConfig& conf);
    void setCookie(const Cookie& data);
    Cookie getCookie() const;
    void addHistory(const std::string& key);
    std::vector<std::string> getHistoryList();
    void setHistory(const std::vector<std::string>& list);
    void setRefreshToken(const std::string& token);
    std::string getRefreshToken() const;
    std::string getCSRF();
    std::string getUserID();
    std::string getBuvid3();
    bool hasLoginInfo();

    // Google Analytics ID
    std::string getClientID();

    // Device ID
    std::string getDeviceID();

    void loadHomeWindowState();
    void saveHomeWindowState();

    template <typename T>
    T getSettingItem(SettingItem item, T defaultValue) {
        auto& key = SETTING_MAP[item].key;
        if (!setting.contains(key)) return defaultValue;
        try {
            return this->setting.at(key).get<T>();
        } catch (const std::exception& e) {
            brls::Logger::error("Damaged config found: {}/{}", key, e.what());
            return defaultValue;
        }
    }

    template <typename T>
    void setSettingItem(SettingItem item, T data, bool save = true) {
        setting[SETTING_MAP[item].key] = data;
        if (save) this->save();
    }

    ProgramOption getOptionData(SettingItem item);

    /**
     * 获取 int 类型选项的当前设定值的索引
     */
    size_t getIntOptionIndex(SettingItem item);

    /**
     * 获取 int 类型选项的当前设定值
     */
    int getIntOption(SettingItem item);

    bool getBoolOption(SettingItem item);

    int getStringOptionIndex(SettingItem item);

    void load();

    void save();

    void init();

    std::string getConfigDir();

    std::string getHomePath();

    void exit(char* argv[]);

    void loadCustomThemes();

    std::vector<CustomTheme> getCustomThemes();

    std::string getProxy();

    void setProxy(const std::string& proxy);

    void setTlsVerify(bool value);

    void addSeasonCustomSetting(const std::string& key, const SeasonCustomItem& item);

    SeasonCustomSetting getSeasonCustomSetting() const;

    SeasonCustomItem getSeasonCustom(const std::string& key) const;

    SeasonCustomItem getSeasonCustom(unsigned int key) const;

    void addSeasonCustomSetting(unsigned int key, const SeasonCustomItem& item);

    void setSeasonCustomSetting(const SeasonCustomSetting& setting);

    void toggleFullscreen();

    std::vector<CustomTheme> customThemes;
    Cookie cookie = {{"DedeUserID", "0"}};
    std::string refreshToken;
    nlohmann::json setting;
    std::string client;
    std::string device;
    std::vector<std::string> searchHistory;
    SeasonCustomSetting seasonCustom;
    std::string httpProxy;
    std::string httpsProxy;

    static std::unordered_map<SettingItem, ProgramOption> SETTING_MAP;
};

inline void to_json(nlohmann::json& nlohmann_json_j, const ProgramConfig& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, cookie, refreshToken, setting, client, device,
                                             searchHistory, seasonCustom));
}

inline void from_json(const nlohmann::json& nlohmann_json_j, ProgramConfig& nlohmann_json_t) {
    if (nlohmann_json_j.contains("cookie") && !nlohmann_json_j.at("cookie").empty())
        nlohmann_json_j.at("cookie").get_to(nlohmann_json_t.cookie);
    if (nlohmann_json_j.contains("searchHistory") && nlohmann_json_j.at("searchHistory").is_array())
        nlohmann_json_j.at("searchHistory").get_to(nlohmann_json_t.searchHistory);
    if (nlohmann_json_j.contains("seasonCustom") && nlohmann_json_j.at("seasonCustom").is_object())
        nlohmann_json_j.at("seasonCustom").get_to(nlohmann_json_t.seasonCustom);
    if (nlohmann_json_j.contains("setting")) nlohmann_json_j.at("setting").get_to(nlohmann_json_t.setting);
    if (nlohmann_json_j.contains("client") && nlohmann_json_j.at("client").is_string())
        nlohmann_json_j.at("client").get_to(nlohmann_json_t.client);
    if (nlohmann_json_j.contains("device") && nlohmann_json_j.at("device").is_string())
        nlohmann_json_j.at("device").get_to(nlohmann_json_t.device);
    if (nlohmann_json_j.contains("refreshToken") && nlohmann_json_j.at("refreshToken").is_string())
        nlohmann_json_j.at("refreshToken").get_to(nlohmann_json_t.refreshToken);
}

class Register {
public:
    static void initCustomView();
    static void initCustomTheme();
    static void initCustomStyle();
};
