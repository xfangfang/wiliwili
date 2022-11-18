//
// Created by fang on 2022/7/10.
//

#include <borealis.hpp>

#include "bilibili.h"
#include "utils/config_helper.hpp"
#include "utils/cache_helper.hpp"
#include "utils/number_helper.hpp"
#include "presenter/video_detail.hpp"
#include "view/mpv_core.hpp"

std::unordered_map<SettingItem, std::string> ProgramConfig::SETTING_MAP = {
    {SettingItem::HIDE_BOTTOM_BAR, "hide_bottom_bar"},
    {SettingItem::APP_THEME, "app_theme"},
    {SettingItem::HISTORY_REPORT, "history_report"},
    {SettingItem::PLAYER_BOTTOM_BAR, "player_bottom_bar"},
    {SettingItem::PLAYER_LOW_QUALITY, "player_low_quality"},
    {SettingItem::TEXTURE_CACHE_NUM, "texture_cache_num"},
    {SettingItem::OPENCC_ON, "opencc"},
};

ProgramConfig::ProgramConfig() {}

ProgramConfig::ProgramConfig(const ProgramConfig& conf) {
    this->cookie  = conf.cookie;
    this->setting = conf.setting;
}

void ProgramConfig::setProgramConfig(const ProgramConfig& conf) {
    this->cookie  = conf.cookie;
    this->setting = conf.setting;
    this->client  = conf.client;
    brls::Logger::info("ProgramConfig::setProgramConfig:");
    brls::Logger::info("client: {}", conf.client);
    for (auto c : conf.cookie) {
        brls::Logger::info("cookie: {}:{}", c.first, c.second);
    }
    brls::Logger::info("setting: {}", conf.setting.dump());
}

void ProgramConfig::setCookie(Cookie data) {
    this->cookie = data;
    this->save();
}

Cookie ProgramConfig::getCookie() { return this->cookie; }

std::string ProgramConfig::getCSRF() {
    if (this->cookie.count("bili_jct") == 0) {
        return "";
    }
    return this->cookie["bili_jct"];
}

std::string ProgramConfig::getUserID() {
    if (this->cookie.count("DedeUserID") == 0) {
        return "";
    }
    return this->cookie["DedeUserID"];
}

std::string ProgramConfig::getClientID() {
    if (this->client.empty()) {
        this->client = wiliwili::getRandomText();
        this->save();
    }
    return this->client;
}

void ProgramConfig::load() {
    const std::string path = this->getConfigDir() + "/wiliwili_config.json";

    std::ifstream readFile(path);
    if (!readFile) return;

    try {
        nlohmann::json content;
        readFile >> content;
        readFile.close();
        this->setProgramConfig(content.get<ProgramConfig>());
    } catch (const std::exception& e) {
        brls::Logger::error("ProgramConfig::load: {}", e.what());
    }
    brls::Logger::info("Load config from: {}", path);

    // 初始化底部栏
    brls::AppletFrame::HIDE_BOTTOM_BAR =
        getSettingItem(SettingItem::HIDE_BOTTOM_BAR, false);

    // 初始化主题
    int themeData = getSettingItem(SettingItem::APP_THEME, 0);
    if (themeData == 1) {
        brls::Application::getPlatform()->setThemeVariant(
            brls::ThemeVariant::LIGHT);
    } else if (themeData == 2) {
        brls::Application::getPlatform()->setThemeVariant(
            brls::ThemeVariant::DARK);
    }

    // 初始化是否上传历史记录
    VideoDetail::REPORT_HISTORY =
        getSettingItem(SettingItem::HISTORY_REPORT, true);

    // 初始化是否固定显示底部进度条
    MPVCore::BOTTOM_BAR = getSettingItem(SettingItem::PLAYER_BOTTOM_BAR, true);

    // 初始化纹理缓存数量
    TextureCache::instance().cache.setCapacity(
        getSettingItem(SettingItem::TEXTURE_CACHE_NUM, 200));

    // 初始化是否使用opencc自动转换简体
    brls::Label::OPENCC_ON = getSettingItem(SettingItem::OPENCC_ON, true);

    // 是否使用低质量解码
#ifdef __SWITCH__
    MPVCore::LOW_QUALITY =
        getSettingItem(SettingItem::PLAYER_LOW_QUALITY, true);
#else
    MPVCore::LOW_QUALITY =
        getSettingItem(SettingItem::PLAYER_LOW_QUALITY, false);
#endif
}

void ProgramConfig::save() {
    const std::string path = this->getConfigDir() + "/wiliwili_config.json";
    std::filesystem::create_directories(this->getConfigDir());
    nlohmann::json content(*this);
    std::ofstream writeFile(path);
    if (!writeFile) {
        brls::Logger::error("Cannot write config to: {}", path);
        return;
    }
    writeFile << content.dump(2);
    writeFile.close();
    brls::Logger::info("Write config to: {}", path);
}

void ProgramConfig::init() {
    this->load();
    Cookie diskCookie = this->getCookie();
    // set bilibili cookie and cookie update callback
    bilibili::BilibiliClient::init(
        diskCookie,
        [](Cookie newCookie) {
            brls::Logger::info("======== write cookies to disk");
            for (auto c : newCookie) {
                brls::Logger::info("cookie: {}:{}", c.first, c.second);
            }
            ProgramConfig::instance().setCookie(newCookie);
        },
        5000);
}

std::string ProgramConfig::getConfigDir() {
#ifdef __SWITCH__
    return "/config/wiliwili";
#else
#ifdef _DEBUG
    return "./config/wiliwili";
#else
#ifdef __APPLE__
    return std::string(getenv("HOME")) +
           "/Library/Application Support/wiliwili";
#endif
#ifdef __linux__
    std::string config = "";
    char* config_home = getenv("XDG_CONFIG_HOME");
    if (config_home) config = std::string(config_home);
    if (config.empty()) config = std::string(getenv("HOME")) + "/.config";
    return config + "/wiliwili";
#endif
#ifdef _WIN32
    return std::string(getenv("HOMEPATH")) +
           "/AppData/Local/xfangfang/wiliwili";
#endif
#endif /* _DEBUG */
#endif /* __SWITCH__ */
}
