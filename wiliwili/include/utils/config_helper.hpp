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

#ifdef __SWITCH__
#define THREAD_POOL_MAX_THREAD_NUM 2
#else
#define THREAD_POOL_MAX_THREAD_NUM CPR_DEFAULT_THREAD_POOL_MAX_THREAD_NUM
#endif

typedef std::map<std::string, std::string> Cookie;

class ProgramConfig: public Singleton<ProgramConfig>{
public:
    ProgramConfig();
    ProgramConfig(const ProgramConfig& config);
    void setProgramConfig(const ProgramConfig& conf);
    void setCookie(Cookie data);
    Cookie getCookie();
    Cookie cookie;
    std::string getCSRF();
    std::string getUserID();
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProgramConfig, cookie);


class ConfigHelper {
public:
    static ProgramConfig readProgramConf();

    static void saveProgramConf();

    static std::string getConfigDir();

    static void init();
};

class Register {
public:
    static void initCustomView();
    static void initCustomTheme();
    static void initCustomStyle();
};
