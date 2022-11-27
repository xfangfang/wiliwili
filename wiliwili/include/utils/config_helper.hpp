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
#include "utils/singleton.hpp"

typedef std::map<std::string, std::string> Cookie;

enum class SettingItem {
    HIDE_BOTTOM_BAR,
    FULLSCREEN,
    APP_THEME,
    HISTORY_REPORT,
    PLAYER_BOTTOM_BAR,
    PLAYER_LOW_QUALITY,
    PLAYER_INMEMORY_CACHE,
    TEXTURE_CACHE_NUM,
    OPENCC_ON,
    CUSTOM_UPDATE_API,
};

class APPVersion : public Singleton<APPVersion> {
    inline static std::string RELEASE_API =
        "https://api.github.com/repos/xfangfang/wiliwili/releases/latest";

public:
    int major, minor, revision;
    std::string git_commit = "", git_tag = "";

    APPVersion();

    std::string getVersionStr();

    bool needUpdate(std::string latestVersion);

    void checkUpdate(int delay = 2000);
};

class ProgramConfig : public Singleton<ProgramConfig> {
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
        auto& key = SETTING_MAP[item];
        if (!setting.contains(key)) return defaultValue;
        return this->setting.at(key).get<T>();
    }

    template <typename T>
    void setSettingItem(SettingItem item, T data) {
        setting[SETTING_MAP[item]] = data;
        this->save();
    }

    void load();

    void save();

    void init();

    std::string getConfigDir();

    Cookie cookie = {{"DedeUserID", "0"}};
    nlohmann::json setting;
    std::string client = "";

    static std::unordered_map<SettingItem, std::string> SETTING_MAP;
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
