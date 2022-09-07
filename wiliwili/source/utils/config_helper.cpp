//
// Created by fang on 2022/7/10.
//

#include "bilibili.h"
#include <borealis/core/logger.hpp>
#include "utils/config_helper.hpp"
#include "borealis/views/applet_frame.hpp"

std::unordered_map<SettingItem, std::string> ProgramConfig::SETTING_MAP = {
    {SettingItem::HIDE_BOTTOM_BAR, "hide_bottom_bar"},
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

    // 初始化设置
    brls::AppletFrame::HIDE_BOTTOM_BAR =
        getSettingItem(SettingItem::HIDE_BOTTOM_BAR, false);
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
