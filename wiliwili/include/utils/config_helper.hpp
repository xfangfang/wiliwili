//
// Created by fang on 2022/7/7.
//

#pragma once

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <cpr/cpr.h>
#include "borealis/core/singleton.hpp"
#include "borealis/core/logger.hpp"

typedef std::map<std::string, std::string> Cookie;

enum class SettingItem {
    HIDE_BOTTOM_BAR,
    FULLSCREEN,
    APP_THEME,
    APP_LANG,
    HISTORY_REPORT,
    AUTO_NEXT_PART,
    AUTO_NEXT_RCMD,
    PLAYER_BOTTOM_BAR,
    PLAYER_LOW_QUALITY,
    PLAYER_INMEMORY_CACHE,
    PLAYER_HWDEC,
    TEXTURE_CACHE_NUM,
    OPENCC_ON,
    CUSTOM_UPDATE_API,
    IMAGE_REQUEST_THREADS,
    VIDEO_FORMAT,
    GAMEPAD_VIBRATION,
    DANMAKU_ON,
    DANMAKU_FILTER_LEVEL,
    DANMAKU_FILTER_SCROLL,
    DANMAKU_FILTER_TOP,
    DANMAKU_FILTER_BOTTOM,
    DANMAKU_FILTER_COLOR,
    DANMAKU_STYLE_AREA,
    DANMAKU_STYLE_ALPHA,
    DANMAKU_STYLE_FONTSIZE,
    DANMAKU_STYLE_SPEED,
};

class APPVersion : public brls::Singleton<APPVersion> {
    inline static std::string RELEASE_API =
        "https://api.github.com/repos/xfangfang/wiliwili/releases/latest";

public:
    int major, minor, revision;
    std::string git_commit = "", git_tag = "";

    APPVersion();

    std::string getVersionStr();

    bool needUpdate(std::string latestVersion);

    void checkUpdate(int delay = 2000, bool showUpToDateDialog = false);
};

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
    void setCookie(Cookie data);
    Cookie getCookie();
    std::string getCSRF();
    std::string getUserID();
    std::string getClientID();

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

    void checkRestart(char* argv[]);

    Cookie cookie = {{"DedeUserID", "0"}};
    nlohmann::json setting;
    std::string client = "";

    static std::unordered_map<SettingItem, ProgramOption> SETTING_MAP;
};

inline void to_json(nlohmann::json& nlohmann_json_j,
                    const ProgramConfig& nlohmann_json_t) {
    NLOHMANN_JSON_EXPAND(
        NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, cookie, setting, client));
}

inline void from_json(const nlohmann::json& nlohmann_json_j,
                      ProgramConfig& nlohmann_json_t) {
    if (nlohmann_json_j.contains("cookie") &&
        !nlohmann_json_j.at("cookie").empty())
        nlohmann_json_j.at("cookie").get_to(nlohmann_json_t.cookie);
    if (nlohmann_json_j.contains("setting"))
        nlohmann_json_j.at("setting").get_to(nlohmann_json_t.setting);
    if (nlohmann_json_j.contains("client"))
        nlohmann_json_j.at("client").get_to(nlohmann_json_t.client);
}

class Register {
public:
    static void initCustomView();
    static void initCustomTheme();
    static void initCustomStyle();
};
