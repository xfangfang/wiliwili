//
// Created by fang on 2022/7/10.
//

#include <borealis.hpp>

#include "bilibili.h"
#include "utils/config_helper.hpp"
#include "utils/cache_helper.hpp"
#include "presenter/video_detail.hpp"
#include "view/mpv_core.hpp"

std::unordered_map<SettingItem, std::string> ProgramConfig::SETTING_MAP = {
    {SettingItem::HIDE_BOTTOM_BAR, "hide_bottom_bar"},
    {SettingItem::APP_THEME, "app_theme"},
    {SettingItem::HISTORY_REPORT, "history_report"},
    {SettingItem::PLAYER_BOTTOM_BAR, "player_bottom_bar"},
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
    TextureCache::instance().cache.setCapacity(getSettingItem(SettingItem::TEXTURE_CACHE_NUM
                                                              , 200));

    // 初始化是否使用opencc自动转换简体
    brls::Label::OPENCC_ON = getSettingItem(SettingItem::OPENCC_ON, true);
}

void ProgramConfig::save() {
    const std::string path = this->getConfigDir() + "/wiliwili_config.json";
    std::filesystem::create_directories(this->getConfigDir());
    nlohmann::json content(*this);
    std::ofstream writeFile(path);
    if (!writeFile) return;
    writeFile << content.dump(2);
    writeFile.close();
}

void ProgramConfig::init() {
    this->load();
    Cookie diskCookie = this->getCookie();
    for (auto c : diskCookie) {
        brls::Logger::info("cookie: {}:{}", c.first, c.second);
    }
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
    return "./config/wiliwili";
#endif
}
