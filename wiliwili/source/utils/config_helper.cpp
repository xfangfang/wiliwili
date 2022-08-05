//
// Created by fang on 2022/7/10.
//

#include "bilibili.h"
#include <borealis/core/logger.hpp>
#include "utils/config_helper.hpp"


ProgramConfig::ProgramConfig(){
    for(auto item: COOKIE_LIST){
        this->cookie[item] = "";
    }
}

ProgramConfig::ProgramConfig(const ProgramConfig& conf){
    this->cookie = conf.cookie;
}

void ProgramConfig::setProgramConfig(const ProgramConfig& conf){
    this->cookie = conf.cookie;
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

void ConfigHelper::saveProgramConf(){
    const std::string path = ConfigHelper::getConfigDir()+"/wiliwili_config.json";
    printf("config path: %s\n", path.c_str());
    std::filesystem::create_directories(ConfigHelper::getConfigDir());
    nlohmann::json content(ProgramConfig::instance());
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
    ProgramConfig::instance().setProgramConfig(ConfigHelper::readProgramConf());
    Cookie cookie = ProgramConfig::instance().getCookie();
    for(auto c : cookie){
        brls::Logger::error("cookie: {}:{}", c.first, c.second);
    }
    // set bilibili cookie and cookie update callback
    bilibili::BilibiliClient::init(cookie,[](Cookie newCookie){
        brls::Logger::error("======== write cookies to disk");
        ProgramConfig::instance().setCookie(newCookie);
        ConfigHelper::saveProgramConf();
    });
}
