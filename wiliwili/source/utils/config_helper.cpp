//
// Created by fang on 2022/7/10.
//

#include <borealis.hpp>

#include "bilibili.h"
#include "borealis/core/cache_helper.hpp"
#include "utils/number_helper.hpp"
#include "utils/image_helper.hpp"
#include "utils/config_helper.hpp"
#include "utils/vibration_helper.hpp"
#include "presenter/video_detail.hpp"
#include "view/mpv_core.hpp"
#include "activity/player_activity.hpp"

using namespace brls::literals;

#ifdef __SWITCH__
std::unordered_map<SettingItem, ProgramOption> ProgramConfig::SETTING_MAP = {
    /// string
    {SettingItem::CUSTOM_UPDATE_API, {"custom_update_api", {}, {}, 0}},
    {SettingItem::APP_LANG,
     {"app_lang",
      {
          brls::LOCALE_AUTO,
          brls::LOCALE_EN_US,
          brls::LOCALE_JA,
          brls::LOCALE_RYU,
          brls::LOCALE_ZH_HANT,
          brls::LOCALE_ZH_HANS,
          brls::LOCALE_Ko,
      },
      {},
      0}},
    {SettingItem::APP_THEME, {"app_theme", {"auto", "light", "dark"}, {}, 0}},

    /// bool
    {SettingItem::GAMEPAD_VIBRATION, {"gamepad_vibration", {}, {}, 1}},
    {SettingItem::HIDE_BOTTOM_BAR, {"hide_bottom_bar", {}, {}, 0}},
    {SettingItem::FULLSCREEN, {"fullscreen", {}, {}, 1}},
    {SettingItem::HISTORY_REPORT, {"history_report", {}, {}, 1}},
    {SettingItem::PLAYER_BOTTOM_BAR, {"player_bottom_bar", {}, {}, 1}},
    {SettingItem::PLAYER_LOW_QUALITY, {"player_low_quality", {}, {}, 1}},
    {SettingItem::PLAYER_HWDEC, {"player_hwdec", {}, {}, 1}},
    {SettingItem::AUTO_NEXT_PART, {"auto_next_part", {}, {}, 1}},
    {SettingItem::AUTO_NEXT_RCMD, {"auto_next_recommend", {}, {}, 1}},
    {SettingItem::OPENCC_ON, {"opencc", {}, {}, 1}},

    /// number
    {SettingItem::PLAYER_INMEMORY_CACHE, {"player_inmemory_cache", {}, {}, 0}},
    {SettingItem::TEXTURE_CACHE_NUM, {"texture_cache_num", {}, {}, 0}},
    {SettingItem::IMAGE_REQUEST_THREADS,
     {"image_request_threads", {"1", "2", "3", "4"}, {1, 2, 3, 4}, 1}},
    {SettingItem::VIDEO_FORMAT,
     {"video_format", {"dash", "flv/mp4"}, {1744, 0}, 0}},
};
#else
std::unordered_map<SettingItem, ProgramOption> ProgramConfig::SETTING_MAP = {
    /// string
    {SettingItem::CUSTOM_UPDATE_API, {"custom_update_api", {}, {}, 0}},
    {SettingItem::APP_LANG,
     {"app_lang",
      {
          brls::LOCALE_EN_US,
          brls::LOCALE_JA,
          brls::LOCALE_RYU,
          brls::LOCALE_ZH_HANT,
          brls::LOCALE_ZH_HANS,
          brls::LOCALE_Ko,
      },
      {},
      4}},
    {SettingItem::APP_THEME, {"app_theme", {"auto", "light", "dark"}, {}, 0}},

    /// bool
    {SettingItem::GAMEPAD_VIBRATION, {"gamepad_vibration", {}, {}, 1}},
    {SettingItem::HIDE_BOTTOM_BAR, {"hide_bottom_bar", {}, {}, 0}},
    {SettingItem::FULLSCREEN, {"fullscreen", {}, {}, 1}},
    {SettingItem::HISTORY_REPORT, {"history_report", {}, {}, 1}},
    {SettingItem::PLAYER_BOTTOM_BAR, {"player_bottom_bar", {}, {}, 1}},
    {SettingItem::PLAYER_LOW_QUALITY, {"player_low_quality", {}, {}, 0}},
    {SettingItem::PLAYER_HWDEC, {"player_hwdec", {}, {}, 1}},
    {SettingItem::AUTO_NEXT_PART, {"auto_next_part", {}, {}, 1}},
    {SettingItem::AUTO_NEXT_RCMD, {"auto_next_recommend", {}, {}, 1}},
    {SettingItem::OPENCC_ON, {"opencc", {}, {}, 1}},

    /// number
    {SettingItem::PLAYER_INMEMORY_CACHE, {"player_inmemory_cache", {}, {}, 0}},
    {SettingItem::TEXTURE_CACHE_NUM, {"texture_cache_num", {}, {}, 0}},
    {SettingItem::IMAGE_REQUEST_THREADS,
     {"image_request_threads",
      {"1", "2", "3", "4", "8", "12", "16"},
      {1, 2, 3, 4, 8, 12, 16},
      3}},
    {SettingItem::VIDEO_FORMAT,
     {"video_format", {"dash", "flv/mp4"}, {1744, 0}, 0}},
};
#endif

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

    // 初始化是否支持手柄振动
    VibrationHelper::GAMEPAD_VIBRATION =
        getBoolOption(SettingItem::GAMEPAD_VIBRATION);

    // 初始化视频格式
    bilibili::BilibiliClient::FNVAL =
        std::to_string(getIntOption(SettingItem::VIDEO_FORMAT));

    // 初始化线程数
    ImageHelper::REQUEST_THREADS =
        getIntOption(SettingItem::IMAGE_REQUEST_THREADS);

    // 初始化底部栏
    brls::AppletFrame::HIDE_BOTTOM_BAR =
        getBoolOption(SettingItem::HIDE_BOTTOM_BAR);

    // 初始化是否全屏，必须在创建窗口前设置此值
    VideoContext::FULLSCREEN = getBoolOption(SettingItem::FULLSCREEN);

    // 初始化是否上传历史记录
    VideoDetail::REPORT_HISTORY = getBoolOption(SettingItem::HISTORY_REPORT);

    // 初始化是否自动播放下一分集
    BasePlayerActivity::AUTO_NEXT_PART =
        getBoolOption(SettingItem::AUTO_NEXT_PART);

    // 初始化是否自动播放推荐视频
    BasePlayerActivity::AUTO_NEXT_RCMD =
        getBoolOption(SettingItem::AUTO_NEXT_RCMD);

    // 初始化是否固定显示底部进度条
    MPVCore::BOTTOM_BAR = getBoolOption(SettingItem::PLAYER_BOTTOM_BAR);

    // 初始化是否使用硬件加速 （仅限非switch设备）
    MPVCore::HARDWARE_DEC = getBoolOption(SettingItem::PLAYER_HWDEC);

    // 初始化内存缓存大小
    MPVCore::INMEMORY_CACHE =
        getSettingItem(SettingItem::PLAYER_INMEMORY_CACHE, 10);

    // 初始化是否使用opencc自动转换简体
    brls::Label::OPENCC_ON = getBoolOption(SettingItem::OPENCC_ON);

    // 是否使用低质量解码
    MPVCore::LOW_QUALITY = getBoolOption(SettingItem::PLAYER_LOW_QUALITY);

    // 初始化系统字体
    std::set<std::string> i18nData{
        brls::LOCALE_AUTO, brls::LOCALE_EN_US,   brls::LOCALE_JA,
        brls::LOCALE_RYU,  brls::LOCALE_ZH_HANS, brls::LOCALE_ZH_HANT,
        brls::LOCALE_Ko,
    };
    std::string langData =
        getSettingItem(SettingItem::APP_LANG, brls::LOCALE_AUTO);

    if (langData != brls::LOCALE_AUTO && i18nData.count(langData)) {
        brls::Platform::APP_LOCALE_DEFAULT = langData;
    } else {
#ifndef __SWITCH__
        brls::Platform::APP_LOCALE_DEFAULT = brls::LOCALE_ZH_HANS;
#endif
    }

    // 初始化一些在创建窗口之后才能初始化的内容
    brls::Application::getWindowCreationDoneEvent()->subscribe([this]() {
        // 初始化主题
        std::string themeData =
            getSettingItem(SettingItem::APP_THEME, std::string{"auto"});
        if (themeData == "light") {
            brls::Application::getPlatform()->setThemeVariant(
                brls::ThemeVariant::LIGHT);
        } else if (themeData == "dark") {
            brls::Application::getPlatform()->setThemeVariant(
                brls::ThemeVariant::DARK);
        }

        // 初始化纹理缓存数量
        brls::TextureCache::instance().cache.setCapacity(
            getSettingItem(SettingItem::TEXTURE_CACHE_NUM, 200));
    });
}

ProgramOption ProgramConfig::getOptionData(SettingItem item) {
    return SETTING_MAP[item];
}

size_t ProgramConfig::getIntOptionIndex(SettingItem item) {
    auto optionData = getOptionData(item);
    if (setting.contains(optionData.key)) {
        try {
            int option = this->setting.at(optionData.key).get<int>();
            for (size_t i = 0; i < optionData.rawOptionList.size(); i++) {
                if (optionData.rawOptionList[i] == option) return i;
            }
        } catch (const std::exception& e) {
            brls::Logger::error("Damaged config found: {}/{}", optionData.key,
                                e.what());
            return optionData.defaultOption;
        }
    }
    return optionData.defaultOption;
}

int ProgramConfig::getIntOption(SettingItem item) {
    auto optionData = getOptionData(item);
    if (setting.contains(optionData.key)) {
        try {
            return this->setting.at(optionData.key).get<int>();
        } catch (const std::exception& e) {
            brls::Logger::error("Damaged config found: {}/{}", optionData.key,
                                e.what());
            return optionData.defaultOption;
        }
    }
    return optionData.rawOptionList[optionData.defaultOption];
}

bool ProgramConfig::getBoolOption(SettingItem item) {
    auto optionData = getOptionData(item);
    if (setting.contains(optionData.key)) {
        try {
            return this->setting.at(optionData.key).get<bool>();
        } catch (const std::exception& e) {
            brls::Logger::error("Damaged config found: {}/{}", optionData.key,
                                e.what());
            return optionData.defaultOption;
        }
    }
    return optionData.defaultOption;
}

int ProgramConfig::getStringOptionIndex(SettingItem item) {
    auto optionData = getOptionData(item);
    if (setting.contains(optionData.key)) {
        try {
            std::string option =
                this->setting.at(optionData.key).get<std::string>();
            for (int i = 0; i < optionData.optionList.size(); ++i)
                if (optionData.optionList[i] == option) return i;
        } catch (const std::exception& e) {
            brls::Logger::error("Damaged config found: {}/{}", optionData.key,
                                e.what());
            return optionData.defaultOption;
        }
    }
    return optionData.defaultOption;
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
    // load config from disk
    this->load();

    // init custom font path
    brls::FontLoader::USER_FONT_PATH = getConfigDir() + "/font.ttf";
    brls::FontLoader::USER_ICON_PATH = getConfigDir() + "/icon.ttf";

    // set bilibili cookie and cookie update callback
    Cookie diskCookie = this->getCookie();
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
    char* config_home  = getenv("XDG_CONFIG_HOME");
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
