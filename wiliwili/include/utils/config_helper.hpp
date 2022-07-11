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


typedef std::map<std::string, std::string> Cookie;

class ProgramConfig {
public:
    ProgramConfig();
    ProgramConfig(ProgramConfig& config);
    void setCookie(Cookie data);
    Cookie getCookie();
    Cookie cookie;
    const std::vector<std::string> COOKIE_LIST  = {"DedeUserID", "DedeUserID_ckMd5", "SESSDATA", "bili_jct"};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProgramConfig, cookie);


class ConfigHelper {
public:
    static ProgramConfig programConfig;
    static ProgramConfig readProgramConf();

    static void saveProgramConf(ProgramConfig conf);

    static std::string getConfigDir();
};
