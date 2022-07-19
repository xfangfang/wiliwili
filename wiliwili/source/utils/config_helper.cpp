//
// Created by fang on 2022/7/10.
//

#include "bilibili.h"
#include <borealis/core/logger.hpp>
#include "utils/config_helper.hpp"


ProgramConfig ConfigHelper::programConfig = ConfigHelper::readProgramConf();

ProgramConfig::ProgramConfig(){
    for(auto item: COOKIE_LIST){
        this->cookie[item] = "";
    }
}
ProgramConfig::ProgramConfig(ProgramConfig& config){
    this->cookie = config.getCookie();
}
void ProgramConfig::setCookie(Cookie data){
    this->cookie = data;
}
Cookie ProgramConfig::getCookie(){
    return this->cookie;
}

ProgramConfig ConfigHelper::readProgramConf(){
    const std::string path = ConfigHelper::getConfigDir()+"/wiliwili_config.json";
    ProgramConfig config;
    nlohmann::json content;
    std::ifstream readFile(path);
    if(!readFile) return config;
    try{
        readFile >> content;
        readFile.close();
        return content.get<ProgramConfig>();
    }
    catch(const std::exception& e){
        return config;
    }
    return config;
}

void ConfigHelper::saveProgramConf(ProgramConfig conf){
    const std::string path = ConfigHelper::getConfigDir()+"/wiliwili_config.json";
    printf("config path: %s", path.c_str());
    std::filesystem::create_directories(ConfigHelper::getConfigDir());
    nlohmann::json content(conf);
    std::ofstream writeFile(path);
    if(!writeFile) return;
    writeFile << content.dump(2);
    writeFile.close();
}

std::string ConfigHelper::getConfigDir(){
#ifdef __SWITCH__
    return "/config/wiliwili";
#else
    return "./config/wiliwili";
#endif
}

void ConfigHelper::init(){
    Cookie cookie = ConfigHelper::programConfig.getCookie();
    for(auto c : cookie){
        brls::Logger::error("cookie: {}:{}", c.first, c.second);
    }
    // set bilibili cookie and cookie update callback
    bilibili::BilibiliClient::init(cookie,[](Cookie newCookie){
        ProgramConfig config;
        config.setCookie(newCookie);
        ConfigHelper::saveProgramConf(config);
    });
}
