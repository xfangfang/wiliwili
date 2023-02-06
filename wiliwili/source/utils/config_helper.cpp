//
// Created by fang on 2022/7/10.
//

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
#include <unistd.h>
#include <borealis/platforms/desktop/desktop_platform.hpp>
#endif

#include <borealis.hpp>

#include "bilibili.h"
#include "borealis/core/cache_helper.hpp"
#include "utils/number_helper.hpp"
#include "utils/image_helper.hpp"
#include "utils/config_helper.hpp"
#include "utils/vibration_helper.hpp"
#include "presenter/video_detail.hpp"
#include "view/mpv_core.hpp"
#include "view/danmaku_core.hpp"
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
    {SettingItem::KEYMAP, {"keymap", {"xbox", "ps", "keyboard"}, {}, 0}},

    /// bool
    {SettingItem::GAMEPAD_VIBRATION, {"gamepad_vibration", {}, {}, 1}},
    {SettingItem::HIDE_BOTTOM_BAR, {"hide_bottom_bar", {}, {}, 0}},
    {SettingItem::HIDE_FPS, {"hide_fps", {}, {}, 1}},
    {SettingItem::FULLSCREEN, {"fullscreen", {}, {}, 1}},
    {SettingItem::HISTORY_REPORT, {"history_report", {}, {}, 1}},
    {SettingItem::PLAYER_BOTTOM_BAR, {"player_bottom_bar", {}, {}, 1}},
    {SettingItem::PLAYER_LOW_QUALITY, {"player_low_quality", {}, {}, 1}},
    {SettingItem::PLAYER_HWDEC, {"player_hwdec", {}, {}, 0}},
    {SettingItem::PLAYER_HWDEC_CUSTOM, {"player_hwdec_custom", {}, {}, 0}},
    {SettingItem::AUTO_NEXT_PART, {"auto_next_part", {}, {}, 1}},
    {SettingItem::AUTO_NEXT_RCMD, {"auto_next_recommend", {}, {}, 1}},
    {SettingItem::OPENCC_ON, {"opencc", {}, {}, 1}},
    {SettingItem::DANMAKU_ON, {"danmaku", {}, {}, 1}},
    {SettingItem::DANMAKU_FILTER_BOTTOM, {"danmaku_filter_bottom", {}, {}, 1}},
    {SettingItem::DANMAKU_FILTER_TOP, {"danmaku_filter_top", {}, {}, 1}},
    {SettingItem::DANMAKU_FILTER_SCROLL, {"danmaku_filter_scroll", {}, {}, 1}},
    {SettingItem::DANMAKU_FILTER_COLOR, {"danmaku_filter_color", {}, {}, 1}},

    /// number
    {SettingItem::PLAYER_INMEMORY_CACHE,
     {"player_inmemory_cache",
      {"0MB", "10MB", "20MB", "50MB", "100MB", "200MB", "500MB"},
      {0, 10, 20, 50, 100, 200, 500},
      0}},
    {SettingItem::TEXTURE_CACHE_NUM, {"texture_cache_num", {}, {}, 0}},
    {SettingItem::VIDEO_QUALITY, {"video_quality", {}, {}, 116}},
    {SettingItem::IMAGE_REQUEST_THREADS,
     {"image_request_threads", {"1", "2", "3", "4"}, {1, 2, 3, 4}, 1}},
    {SettingItem::VIDEO_FORMAT,
     {"video_format", {"dash", "flv/mp4"}, {1744, 0}, 0}},
    {SettingItem::DANMAKU_FILTER_LEVEL,
     {"danmaku_filter_level",
      {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"},
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
      0}},
    {SettingItem::DANMAKU_STYLE_AREA,
     {"danmaku_style_area", {"1/4", "1/2", "3/4", "1"}, {25, 50, 75, 100}, 3}},
    {SettingItem::DANMAKU_STYLE_ALPHA,
     {"danmaku_style_alpha",
      {"10%", "25%", "50%", "60%", "70%", "80%", "90%", "100%"},
      {10, 25, 50, 60, 70, 80, 90, 100},
      5}},
    {SettingItem::DANMAKU_STYLE_FONTSIZE,
     {"danmaku_style_fontsize",
      {"50%", "75%", "100%", "125%", "150%", "175%"},
      {15, 22, 30, 37, 45, 50},
      2}},
    {SettingItem::DANMAKU_STYLE_LINE_HEIGHT,
     {"danmaku_style_line_height",
      {"100%", "120%", "140%", "160%", "180%", "200%"},
      {100, 120, 140, 160, 180, 200},
      1}},
    {SettingItem::DANMAKU_STYLE_SPEED,
     {"danmaku_style_speed",
      {"0.5", "0.75", "1.0", "1.25", "1.5"},
      {150, 125, 100, 75, 50},
      2}},
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
    {SettingItem::KEYMAP, {"keymap", {"xbox", "ps", "keyboard"}, {}, 0}},
    {SettingItem::HOME_WINDOW_STATE, {"home_window_state", {}, {}, 0}},

    /// bool
    {SettingItem::GAMEPAD_VIBRATION, {"gamepad_vibration", {}, {}, 1}},
    {SettingItem::HIDE_BOTTOM_BAR, {"hide_bottom_bar", {}, {}, 0}},
    {SettingItem::HIDE_FPS, {"hide_fps", {}, {}, 1}},
#ifdef __APPLE__
    // mac使用原生全屏按钮效果更好，不通过软件来控制
    {SettingItem::FULLSCREEN, {"fullscreen", {}, {}, 0}},
#else
    {SettingItem::FULLSCREEN, {"fullscreen", {}, {}, 1}},
#endif
    {SettingItem::HISTORY_REPORT, {"history_report", {}, {}, 1}},
    {SettingItem::PLAYER_BOTTOM_BAR, {"player_bottom_bar", {}, {}, 1}},
    {SettingItem::PLAYER_LOW_QUALITY, {"player_low_quality", {}, {}, 0}},
    {SettingItem::PLAYER_HWDEC, {"player_hwdec", {}, {}, 0}},
    {SettingItem::PLAYER_HWDEC_CUSTOM, {"player_hwdec_custom", {}, {}, 0}},
    {SettingItem::AUTO_NEXT_PART, {"auto_next_part", {}, {}, 1}},
    {SettingItem::AUTO_NEXT_RCMD, {"auto_next_recommend", {}, {}, 1}},
    {SettingItem::OPENCC_ON, {"opencc", {}, {}, 1}},
    {SettingItem::DANMAKU_ON, {"danmaku", {}, {}, 1}},
    {SettingItem::DANMAKU_FILTER_BOTTOM, {"danmaku_filter_bottom", {}, {}, 1}},
    {SettingItem::DANMAKU_FILTER_TOP, {"danmaku_filter_top", {}, {}, 1}},
    {SettingItem::DANMAKU_FILTER_SCROLL, {"danmaku_filter_scroll", {}, {}, 1}},
    {SettingItem::DANMAKU_FILTER_COLOR, {"danmaku_filter_color", {}, {}, 1}},

    /// number
    {SettingItem::PLAYER_INMEMORY_CACHE,
     {"player_inmemory_cache",
      {"0MB", "10MB", "20MB", "50MB", "100MB", "200MB", "500MB"},
      {0, 10, 20, 50, 100, 200, 500},
      2}},
    {SettingItem::TEXTURE_CACHE_NUM, {"texture_cache_num", {}, {}, 0}},
    {SettingItem::VIDEO_QUALITY, {"video_quality", {}, {}, 116}},
    {SettingItem::IMAGE_REQUEST_THREADS,
     {"image_request_threads",
      {"1", "2", "3", "4", "8", "12", "16"},
      {1, 2, 3, 4, 8, 12, 16},
      3}},
    {SettingItem::VIDEO_FORMAT,
     {"video_format", {"dash", "flv/mp4"}, {1744, 0}, 0}},
    {SettingItem::DANMAKU_FILTER_LEVEL,
     {"danmaku_filter_level",
      {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"},
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
      0}},
    {SettingItem::DANMAKU_STYLE_AREA,
     {"danmaku_style_area", {"1/4", "1/2", "3/4", "1"}, {25, 50, 75, 100}, 3}},
    {SettingItem::DANMAKU_STYLE_ALPHA,
     {"danmaku_style_alpha",
      {"10%", "25%", "50%", "60%", "70%", "80%", "90%", "100%"},
      {10, 25, 50, 60, 70, 80, 90, 100},
      5}},
    {SettingItem::DANMAKU_STYLE_FONTSIZE,
     {"danmaku_style_fontsize",
      {"50%", "75%", "100%", "125%", "150%", "175%"},
      {15, 22, 30, 37, 45, 50},
      2}},
    {SettingItem::DANMAKU_STYLE_LINE_HEIGHT,
     {"danmaku_style_line_height",
      {"100%", "120%", "140%", "160%", "180%", "200%"},
      {100, 120, 140, 160, 180, 200},
      1}},
    {SettingItem::DANMAKU_STYLE_SPEED,
     {"danmaku_style_speed",
      {"0.5", "0.75", "1.0", "1.25", "1.5"},
      {150, 125, 100, 75, 50},
      2}},
};
#endif

ProgramConfig::ProgramConfig() = default;

ProgramConfig::ProgramConfig(const ProgramConfig& conf) {
    this->cookie  = conf.cookie;
    this->setting = conf.setting;
}

void ProgramConfig::setProgramConfig(const ProgramConfig& conf) {
    this->cookie  = conf.cookie;
    this->setting = conf.setting;
    this->client  = conf.client;
    brls::Logger::info("client: {}", conf.client);
    for (const auto& c : conf.cookie) {
        brls::Logger::info("cookie: {}:{}", c.first, c.second);
    }
    brls::Logger::info("setting: {}", conf.setting.dump());
}

void ProgramConfig::setCookie(Cookie data) {
    this->cookie = std::move(data);
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
        this->client = fmt::format("{}.{}", wiliwili::getRandomNumber(),
                                   wiliwili::getUnixTime());
        this->save();
    }
    return this->client;
}

void ProgramConfig::loadHomeWindowState() {
    std::string homeWindowStateData =
        getSettingItem(SettingItem::HOME_WINDOW_STATE, std::string{""});

    if (homeWindowStateData.empty()) return;

    uint32_t hWidth, hHeight;
    int hXPos, hYPos;
    int monitor;

    sscanf(homeWindowStateData.c_str(), "%d,%ux%u,%dx%d", &monitor, &hWidth,
           &hHeight, &hXPos, &hYPos);

    if (hWidth == 0 || hHeight == 0) return;

    VideoContext::sizeH        = hHeight;
    VideoContext::sizeW        = hWidth;
    VideoContext::posX         = (float)hXPos;
    VideoContext::posY         = (float)hYPos;
    VideoContext::monitorIndex = monitor;

    brls::Logger::info("Load window state: {}x{},{}x{}", hWidth, hHeight, hXPos,
                       hYPos);
}

void ProgramConfig::saveHomeWindowState() {
    if (isnan(VideoContext::posX) || isnan(VideoContext::posY)) return;
    auto videoContext = brls::Application::getPlatform()->getVideoContext();

    uint32_t width  = VideoContext::sizeW;
    uint32_t height = VideoContext::sizeH;
    int xPos        = VideoContext::posX;
    int yPos        = VideoContext::posY;
    int monitor     = videoContext->getCurrentMonitorIndex();
    if (width == 0) width = brls::ORIGINAL_WINDOW_WIDTH;
    if (height == 0) height = brls::ORIGINAL_WINDOW_HEIGHT;
    brls::Logger::info("Save window state: {},{}x{},{}x{}", monitor, width,
                       height, xPos, yPos);
    setSettingItem(
        SettingItem::HOME_WINDOW_STATE,
        fmt::format("{},{}x{},{}x{}", monitor, width, height, xPos, yPos));
}

void ProgramConfig::load() {
    const std::string path = this->getConfigDir() + "/wiliwili_config.json";

    std::ifstream readFile(path);
    if (readFile) {
        try {
            nlohmann::json content;
            readFile >> content;
            readFile.close();
            this->setProgramConfig(content.get<ProgramConfig>());
        } catch (const std::exception& e) {
            brls::Logger::error("ProgramConfig::load: {}", e.what());
        }
        brls::Logger::info("Load config from: {}", path);
    }

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
    brls::DesktopPlatform::GAMEPAD_DB =
        getConfigDir() + "/gamecontrollerdb.txt";
#endif

    // 初始化视频清晰度
    VideoDetail::defaultQuality =
        getSettingItem(SettingItem::VIDEO_QUALITY, 116);

    // 初始化弹幕相关内容
    DanmakuCore::DANMAKU_ON = getBoolOption(SettingItem::DANMAKU_ON);
    DanmakuCore::DANMAKU_FILTER_SHOW_TOP =
        getBoolOption(SettingItem::DANMAKU_FILTER_TOP);
    DanmakuCore::DANMAKU_FILTER_SHOW_BOTTOM =
        getBoolOption(SettingItem::DANMAKU_FILTER_BOTTOM);
    DanmakuCore::DANMAKU_FILTER_SHOW_SCROLL =
        getBoolOption(SettingItem::DANMAKU_FILTER_SCROLL);
    DanmakuCore::DANMAKU_FILTER_SHOW_COLOR =
        getBoolOption(SettingItem::DANMAKU_FILTER_COLOR);
    DanmakuCore::DANMAKU_FILTER_LEVEL =
        getIntOption(SettingItem::DANMAKU_FILTER_LEVEL);
    DanmakuCore::DANMAKU_STYLE_AREA =
        getIntOption(SettingItem::DANMAKU_STYLE_AREA);
    DanmakuCore::DANMAKU_STYLE_ALPHA =
        getIntOption(SettingItem::DANMAKU_STYLE_ALPHA);
    DanmakuCore::DANMAKU_STYLE_FONTSIZE =
        getIntOption(SettingItem::DANMAKU_STYLE_FONTSIZE);
    DanmakuCore::DANMAKU_STYLE_LINE_HEIGHT =
        getIntOption(SettingItem::DANMAKU_STYLE_LINE_HEIGHT);
    DanmakuCore::DANMAKU_STYLE_SPEED =
        getIntOption(SettingItem::DANMAKU_STYLE_SPEED);

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

    // 初始化FPS
    brls::Application::setFPSStatus(!getBoolOption(SettingItem::HIDE_FPS));

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

    // 初始化自定义的硬件加速方案
    MPVCore::PLAYER_HWDEC_METHOD = getSettingItem(
        SettingItem::PLAYER_HWDEC_CUSTOM, MPVCore::PLAYER_HWDEC_METHOD);

    // 初始化内存缓存大小
    MPVCore::INMEMORY_CACHE = getIntOption(SettingItem::PLAYER_INMEMORY_CACHE);

    // 初始化是否使用opencc自动转换简体
    brls::Label::OPENCC_ON = getBoolOption(SettingItem::OPENCC_ON);

    // 是否使用低质量解码
    MPVCore::LOW_QUALITY = getBoolOption(SettingItem::PLAYER_LOW_QUALITY);

    // 初始化i18n
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

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
    // 初始化上一次窗口位置
    loadHomeWindowState();
#endif

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

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
        // 设置窗口最小尺寸
        brls::Application::getPlatform()->setWindowSizeLimits(
            MINIMUM_WINDOW_WIDTH, MINIMUM_WINDOW_HEIGHT, 0, 0);
#endif
    });

#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
    // 窗口将要关闭时, 保存窗口状态配置
    brls::Application::getExitEvent()->subscribe(
        [this]() { saveHomeWindowState(); });
#endif
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
            return optionData.rawOptionList[optionData.defaultOption];
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
            for (size_t i = 0; i < optionData.optionList.size(); ++i)
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
    brls::Logger::info("wiliwili {}", APPVersion::instance().git_tag);
    // load config from disk
    this->load();

    // init custom font path
    brls::FontLoader::USER_FONT_PATH = getConfigDir() + "/font.ttf";
    brls::FontLoader::USER_ICON_PATH = getConfigDir() + "/icon.ttf";

    if (access(brls::FontLoader::USER_ICON_PATH.c_str(), F_OK) == -1) {
        // 自定义字体不存在，使用内置字体
        std::string icon =
            getSettingItem(SettingItem::KEYMAP, std::string{"xbox"});
        if (icon == "xbox") {
            brls::FontLoader::USER_ICON_PATH =
                BRLS_ASSET("font/keymap_xbox.ttf");
        } else if (icon == "ps") {
            brls::FontLoader::USER_ICON_PATH = BRLS_ASSET("font/keymap_ps.ttf");
        } else {
            brls::FontLoader::USER_ICON_PATH =
                BRLS_ASSET("font/keymap_keyboard.ttf");
        }
    }

    // set bilibili cookie and cookie update callback
    Cookie diskCookie = this->getCookie();
    bilibili::BilibiliClient::init(
        diskCookie,
        [](const Cookie& newCookie) {
            brls::Logger::info("======== write cookies to disk");
            for (const auto& c : newCookie) {
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
    char currentPathBuffer[PATH_MAX];
    std::string currentPath =
        getcwd(currentPathBuffer, sizeof(currentPathBuffer));
#ifdef _WIN32
    return currentPath + "\\config\\wiliwili";
#else
    return currentPath + "/config/wiliwili";
#endif /* _WIN32 */
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
    return std::string(getenv("LOCALAPPDATA")) + "\\xfangfang\\wiliwili";
#endif
#endif /* _DEBUG */
#endif /* __SWITCH__ */
}

void ProgramConfig::checkRestart(char* argv[]) {
#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
    if (!brls::DesktopPlatform::RESTART_APP) return;

#ifdef __linux__
    char filePath[PATH_MAX + 1];
    ssize_t count = readlink("/proc/self/exe", filePath, PATH_MAX);
    if (count <= 0)
        strcpy(filePath, argv[0]);
    else
        filePath[count] = 0;
#else
    char* filePath = argv[0];
#endif

    brls::Logger::info("Restart app {}", filePath);

    execv(filePath, argv);
#endif
}
